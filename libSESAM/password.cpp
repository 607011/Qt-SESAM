/*

    Copyright (c) 2015 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <string>

#include "domainsettings.h"
#include "securebytearray.h"
#include "securestring.h"
#include "password.h"
#include "pbkdf2.h"
#include "util.h"

#include "3rdparty/bigint/bigInt.h"

Password::Complexity::Complexity(void)
  : digits(false)
  , lowercase(true)
  , uppercase(true)
  , extra(false)
{ /* ... */ }


Password::Complexity::Complexity(const Password::Complexity &o)
  : digits(o.digits)
  , lowercase(o.lowercase)
  , uppercase(o.uppercase)
  , extra(o.extra)
{ /* ... */ }


Password::Complexity::Complexity(bool digits, bool lowercase, bool uppercase, bool extra)
  : digits(digits)
  , lowercase(lowercase)
  , uppercase(uppercase)
  , extra(extra)
{ /* ... */ }


bool Password::Complexity::operator==(const Password::Complexity &o)
{
  return o.digits == digits
      && o.lowercase == lowercase
      && o.uppercase == uppercase
      && o.extra == extra;
}


bool Password::Complexity::operator!=(const Password::Complexity &o)
{
  return o.digits != digits
      || o.lowercase != lowercase
      || o.uppercase != uppercase
      || o.extra != extra;
}


Password::Complexity Password::Complexity::fromValue(int c)
{
  return Mapping.at(c);
}


Password::Complexity Password::Complexity::fromTemplate(const QString &templ)
{
  return Password::Complexity(templ.contains('n'),
                              templ.contains('a'),
                              templ.contains('A'),
                              templ.contains('o'));
}


int Password::Complexity::value(void) const
{
  int i = 0;
  foreach (Password::Complexity complexity, Password::Complexity::Mapping) {
    if (complexity == *this) {
      return i;
    }
    ++i;
  }
  return -1;
}


const QVector<Password::Complexity> Password::Complexity::Mapping =
{
  Password::Complexity(true, false, false, false),
  Password::Complexity(false, true, false, false),
  Password::Complexity(false, false, true, false),
  Password::Complexity(true, true, false, false),
  Password::Complexity(false, true, true, false),
  Password::Complexity(true, true, true, false),
  Password::Complexity(true, true, true, true),
  Password::Complexity(false, false, false, true)
};


class PasswordPrivate {
public:
  PasswordPrivate(void)
    : error(Password::NoError)
  { /* ... */ }
  ~PasswordPrivate()
  { /* ... */ }
  DomainSettings ds;
  PBKDF2 pbkdf2;
  SecureString password;
  int error;
  QString errorString;
  QFuture<void> future;
};


const QString Password::Digits = "0123456789";
const QString Password::LowerChars = "abcdefghijklmnopqrstuvwxyz";
const QString Password::UpperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const QString Password::ExtraChars = "!\\|\"$%/&?!<>()[]{}~`´#'=-_+*~.,;:^°";
const int Password::DefaultLength = 13;
const int Password::DefaultMaxLength = 2 * Password::DefaultLength;
const int Password::MaxComplexityValue = 7;
const int Password::DefaultComplexityValue = 5;
const int Password::NoComplexityValue = -1;


const Password::TemplateCharacterMap Password::TemplateCharacters = {
  std::pair<char, QString>('n', Password::Digits),
  std::pair<char, QString>('a', Password::LowerChars),
  std::pair<char, QString>('A', Password::UpperChars)
};


Password::Password(const DomainSettings &ds, QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationStarted()), SIGNAL(generationStarted()));
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationAborted()), SIGNAL(generationAborted()));
  setDomainSettings(ds);
}


Password::~Password()
{
  // ...
}


void Password::setDomainSettings(const DomainSettings &ds)
{
  Q_D(Password);
  d->ds = ds;
  d->ds.usedCharacters.clear();
  if (d->ds.passwordTemplate.contains('n')) {
    d->ds.usedCharacters.append(Password::Digits);
  }
  if (d->ds.passwordTemplate.contains('a')) {
    d->ds.usedCharacters.append(Password::LowerChars);
  }
  if (d->ds.passwordTemplate.contains('A')) {
    d->ds.usedCharacters.append(Password::UpperChars);
  }
  if (d->ds.passwordTemplate.contains('o')) {
    d->ds.usedCharacters.append(d_ptr->ds.extraCharacters);
  }
}


const SecureString &Password::remix(void)
{
  Q_D(Password);
  d->password.clear();
  if (d->ds.usedCharacters.isEmpty()) {
    d->error = EmptyCharacterSetError;
    d->errorString = "used character set must not be empty";
    return SecureString();
  }
  if (d->ds.passwordTemplate.isEmpty()) {
    d->error = EmptyTemplateError;
    d->errorString = "password template is empty";
    return SecureString();
  }
  d->error = NoError;
  d->errorString.clear();
  BigInt::Rossi v(d->pbkdf2.hexKey().toStdString(), BigInt::HEX_DIGIT);
  const QString &templ = d->ds.passwordTemplate;
  for (int i = 0; i < templ.length(); ++i) {
    QString charSet;
    const char m = templ.at(i).toLatin1();
    switch (m) {
    case 'x':
      charSet = d->ds.usedCharacters;
      break;
    case 'o':
      charSet = d->ds.extraCharacters;
      break;
    case 'a': // fall-through
    case 'A': // fall-through
    case 'n':
      charSet = TemplateCharacters[m];
      break;
    default:
      d->error = EmptyTemplateError;
      d->errorString = QString("invalid template character: %1").arg(m);
      return SecureString();
    }
    const int nChars = charSet.count();
    if (nChars == 0) {
      d->error = EmptyCharacterSetError;
      d->errorString = QString("character set for template character %1 must not be empty").arg(m);
      return SecureString();
    }
    const BigInt::Rossi Modulus(QString("%1").arg(nChars).toStdString(), BigInt::DEC_DIGIT);
    const BigInt::Rossi &mod = v % Modulus;
    d->password.append(charSet.at(int(mod.toUlong())));
    v = v / Modulus;
  }
  return d->password;
}


void Password::generate(const SecureByteArray &key)
{
  Q_D(Password);
  const SecureByteArray &pwd =
      d->ds.domainName.toUtf8() +
      d->ds.userName.toUtf8() +
      key;
  d->pbkdf2.generate(pwd,
                     QByteArray::fromBase64(d->ds.salt_base64.toUtf8()),
                     d->ds.iterations,
                     QCryptographicHash::Sha512);
  remix();
  emit generated();
}


void Password::generateAsync(const SecureByteArray &key, const DomainSettings &domainSettings)
{
  Q_D(Password);
  setDomainSettings(domainSettings);
  d->future = QtConcurrent::run(this, &Password::generate, key);
}


bool Password::isRunning(void) const
{
  return d_ptr->future.isRunning();
}


bool Password::isAborted(void) const
{
  return d_ptr->pbkdf2.isAborted();
}


qreal Password::elapsedSeconds(void) const
{
  return d_ptr->pbkdf2.elapsedSeconds();
}


void Password::abortGeneration(void)
{
  d_ptr->pbkdf2.abortGeneration();
}


const SecureString &Password::password(void) const
{
  return d_ptr->password;
}


const SecureString &Password::hexKey(void) const
{
  return d_ptr->pbkdf2.hexKey();
}


void Password::waitForFinished(void)
{
  d_ptr->future.waitForFinished();
}


int Password::error(void) const
{
  return d_ptr->error;
}


QString Password::errorString(void) const
{
  return d_ptr->errorString;
}


QDebug operator<<(QDebug debug, const Password::Complexity &complexity)
{
  QDebugStateSaver saver(debug);
  (void)saver;
  debug.nospace()
      << "Password::Complexity {\n"
      << "  digits: " << complexity.digits << "\n"
      << "  lowercase:" << complexity.lowercase << "\n"
      << "  uppercase:" << complexity.uppercase << "\n"
      << "  extra:" << complexity.extra << "\n"
      << "}";
  return debug;
}
