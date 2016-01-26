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

class PasswordPrivate {
public:
  PasswordPrivate(void)
  { /* ... */ }
  ~PasswordPrivate()
  { /* ... */ }
  PBKDF2 pbkdf2;
  SecureString password;
  DomainSettings domainSettings;
  QFuture<void> future;
};


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


const QString Password::LowerChars = QString("abcdefghijklmnopqrstuvwxyz");
const QString Password::UpperChars = Password::LowerChars.toUpper();
const QString Password::Digits = QString("0123456789");
const QString Password::ExtraChars = QString("!\\|\"$%/&?!<>()[]{}~`´#'=-_+*~.,;:^°").toUtf8(); // default: !"$%&?!<>()[]{}\|/~`´#'=-_+*~.,;:^°
const int Password::DefaultLength = 13;
const int Password::DefaultMaxLength = 2 * Password::DefaultLength;
const int Password::MaxComplexityValue = 7;
const int Password::DefaultComplexityValue = 5;
const int Password::NoComplexityValue = -1;


const Password::TemplateCharacterMap Password::TemplateCharacters = {
  std::pair<char, QString>('V', "AEIOUY"),
  std::pair<char, QString>('v', "aeiuoy"),
  std::pair<char, QString>('C', "BCDFGHJKLMNPQRSTVWXZ"),
  std::pair<char, QString>('c', "bcdfghjklmnpqrstvwxz"),
  std::pair<char, QString>('A', "ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
  std::pair<char, QString>('a', "abcdefghijklmnopqrstuvwxyz"),
  std::pair<char, QString>('n', "0123456789"),
  std::pair<char, QString>('o', "@&%?,=[]_:-+*$#!'^~;()/."),
  std::pair<char, QString>('x', "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@&%?,=[]_:-+*$#!'^~;()/.")
};


Password::Password(const DomainSettings &ds, QObject *parent)
  : QObject(parent)
  , d_ptr(new PasswordPrivate)
{
  d_ptr->domainSettings = ds;
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationStarted()), SIGNAL(generationStarted()));
  QObject::connect(&d_ptr->pbkdf2, SIGNAL(generationAborted()), SIGNAL(generationAborted()));
}

Password::~Password()
{
  // ...
}


void Password::setDomainSettings(const DomainSettings &ds)
{
  Q_D(Password);
  d->domainSettings = ds;
}


void Password::generate(const SecureByteArray &key)
{
  Q_D(Password);
  const SecureByteArray &pwd =
      d->domainSettings.domainName.toUtf8() +
      d->domainSettings.userName.toUtf8() +
      key;
  d->pbkdf2.generate(pwd,
                     QByteArray::fromBase64(d->domainSettings.salt_base64.toUtf8()),
                     d->domainSettings.iterations,
                     QCryptographicHash::Sha512);
  remix();
  emit generated();
}


const SecureString &Password::remix(void)
{
  Q_D(Password);
  d->password.clear();
  static const BigInt::Rossi Zero(0);
  BigInt::Rossi v(d->pbkdf2.hexKey().toStdString(), BigInt::HEX_DIGIT);
#ifndef OMIT_V2_CODE
  const QStringList &templateParts = d->domainSettings.passwordTemplate.split(';', QString::KeepEmptyParts);
  QString templ;
  if (templateParts.count() == 1) {
    templ = templateParts.at(0);
  }
  else if (templateParts.count() == 2) {
    templ = templateParts.at(1);
  }
#else
  const QString &templ = d->domainSettings.passwordTemplate;
#endif
  const QString &used = usedCharacters(templ);
  if (used.isEmpty()) {
    return "*** CANNOT COMPUTE PASSWORD ***";
  }
  if (!templ.isEmpty()) {
    int n = 0;
    while (v > Zero && n < templ.length()) {
      const char m = templ.at(n).toLatin1();
      QString charSet;
      switch (m) {
      case 'x':
        charSet = used;
        if (charSet.isEmpty()) {
          qWarning() << "character set must not be empty for template character 'x'";
        }
        break;
      case 'o':
        charSet = d->domainSettings.extraCharacters;
        break;
      case 'a': // fall-through
      case 'A': // fall-through
      case 'n':
        charSet = TemplateCharacters[m];
        break;
      default:
        qWarning() << "Invalid template character:" << m;
        break;
      }
      const int nChars = charSet.count();
      if (nChars > 0) {
        const BigInt::Rossi Modulus(QString("%1").arg(nChars).toStdString(), BigInt::DEC_DIGIT);
        const BigInt::Rossi &mod = v % Modulus;
        d->password.append(charSet.at(int(mod.toUlong())));
        v = v / Modulus;
      }
      else {
        qWarning() << "character set is empty";
      }
      ++n;
    }
  }
  else {
    qWarning() << "password template is empty";
  }
  return d->password;
}


void Password::generateAsync(const SecureByteArray &key, const DomainSettings &domainSettings)
{
  Q_D(Password);
  d->domainSettings = domainSettings;
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


QString Password::usedCharacters(const QString &templ) const
{
  QString used;
  if (templ.contains('n')) {
    used += Password::Digits;
  }
  if (templ.contains('a')) {
    used += Password::LowerChars;
  }
  if (templ.contains('A')) {
    used += Password::UpperChars;
  }
  if (templ.contains('o')) {
    used += d_ptr->domainSettings.extraCharacters;
  }
  return used;
}


SecureString Password::operator()(void) const
{
  return password();
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


const QString &Password::charSetFor(char ch)
{
  return TemplateCharacters[ch];
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
