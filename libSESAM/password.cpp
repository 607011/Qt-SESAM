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


static QBitArray toQBitArray(const QString &s)
{
  QBitArray ba(s.count(), false);
  int i = 0;
  foreach (QChar d, s) {
    if (d == '1')
      ba.setBit(i);
    ++i;
  }
  return ba;
}


const QVector<QBitArray> Password::ComplexityMapping =
{
  toQBitArray("1000"),
  toQBitArray("0100"),
  toQBitArray("0010"),
  toQBitArray("1100"),
  toQBitArray("0110"),
  toQBitArray("1110"),
  toQBitArray("1111")
};


const QString Password::LowerChars = QString("abcdefghijklmnopqrstuvwxyz");
const QString Password::UpperChars = Password::LowerChars.toUpper();
const QString Password::Digits = QString("0123456789");
const QString Password::ExtraChars = QString("!\\|\"$%/&?!<>()[]{}~`´#'=-_+*~.,;:^°").toUtf8(); // default: !"$%&?!<>()[]{}\|/~`´#'=-_+*~.,;:^°
const QString Password::AllChars = Password::LowerChars + Password::UpperChars + Password::Digits + Password::ExtraChars;
const int Password::DefaultLength = 18;
const int Password::DefaultMaxLength = 2 * Password::DefaultLength;
const int Password::MaxComplexity = 6;
const int Password::DefaultComplexity = Password::MaxComplexity;
const int Password::NoComplexity = -1;


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
  d_ptr->domainSettings = ds;
}


QBitArray Password::deconstructedComplexity(int complexity)
{
  QBitArray ba(4, false);
  switch (complexity) {
  case 6:
    ba[TemplateDigits] = true;
    ba[TemplateLowercase] = true;
    ba[TemplateUppercase] = true;
    ba[TemplateExtra] = true;
    break;
  case 5:
    ba[TemplateDigits] = true;
    ba[TemplateLowercase] = true;
    ba[TemplateUppercase] = true;
    ba[TemplateExtra] = false;
    break;
  case 4:
    ba[TemplateDigits] = false;
    ba[TemplateLowercase] = true;
    ba[TemplateUppercase] = true;
    ba[TemplateExtra] = false;
    break;
  case 3:
    ba[TemplateDigits] = true;
    ba[TemplateLowercase] = true;
    ba[TemplateUppercase] = false;
    ba[TemplateExtra] = false;
    break;
  case 2:
    ba[TemplateDigits] = false;
    ba[TemplateLowercase] = false;
    ba[TemplateUppercase] = true;
    ba[TemplateExtra] = false;
    break;
  case 1:
    ba[TemplateDigits] = false;
    ba[TemplateLowercase] = true;
    ba[TemplateUppercase] = false;
    ba[TemplateExtra] = false;
    break;
  case 0:
    ba[TemplateDigits] = true;
    ba[TemplateLowercase] = false;
    ba[TemplateUppercase] = false;
    ba[TemplateExtra] = false;
    break;
  default:
    break;
  }
  return ba;
}


int Password::constructedComplexity(const QBitArray &ba)
{
  int complexity = -1;
  int i = 0;
  foreach (QBitArray testBa, Password::ComplexityMapping) {
    if (ba == testBa) {
      complexity = i;
      break;
    }
    ++i;
  }
  return complexity;
}


void Password::generate(const SecureByteArray &masterPassword)
{
  Q_D(Password);

  const SecureByteArray &pwd =
      d->domainSettings.domainName.toUtf8() +
      d->domainSettings.userName.toUtf8() +
      masterPassword;

  d->pbkdf2.generate(pwd, QByteArray::fromBase64(d->domainSettings.salt_base64.toUtf8()), d->domainSettings.iterations, QCryptographicHash::Sha512);

  remixed();
  emit generated();
}


const SecureString &Password::remixed(void)
{
  Q_D(Password);
  d->password.clear();
  static const BigInt::Rossi Zero(0);
  BigInt::Rossi v(d->pbkdf2.hexKey().toStdString(), BigInt::HEX_DIGIT);
  if (d->domainSettings.passwordTemplate.isEmpty() && !d->domainSettings.usedCharacters.isEmpty()) { // v2 method
    const int nChars = d->domainSettings.usedCharacters.count();
    const QString strModulus = QString("%1").arg(nChars);
    const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
    int n = d->domainSettings.passwordLength;
    while (v > Zero && n-- > 0) {
      const BigInt::Rossi &mod = v % Modulus;
      d->password += d->domainSettings.usedCharacters.at(int(mod.toUlong()));
      v = v / Modulus;
    }
  }
  else { // v3 method
    QByteArray templ;
    const QList<QByteArray> &templateParts = d->domainSettings.passwordTemplate.split(';');
    if (templateParts.count() != 2)
      return SecureString();
    templ = templateParts.at(1);
    int n = 0;
    while (v > Zero && n < templ.length()) {
      const char m = templ.at(n);
      QString charSet;
      switch (m) {
      case 'x':
        charSet = d->domainSettings.usedCharacters;
        break;
      case 'o':
        charSet = d->domainSettings.extraCharacters;
        break;
      case 'a':
        // fall-through
      case 'A':
        // fall-through
      case 'n':
        charSet = TemplateCharacters[m];
        break;
      default:
        qWarning() << "Invalid template character:" << m;
        break;
      }
      if (!charSet.isEmpty()) {
        const QString strModulus = QString("%1").arg(charSet.count());
        const BigInt::Rossi Modulus(strModulus.toStdString(), BigInt::DEC_DIGIT);
        const BigInt::Rossi &mod = v % Modulus;
        d->password += charSet.at(int(mod.toUlong()));
        v = v / Modulus;
      }
      ++n;
    }
  }
  return d->password;
}


void Password::generateAsync(const SecureByteArray &masterPassword, const DomainSettings &domainSettings)
{
  Q_D(Password);
  if (!domainSettings.isEmpty())
    d->domainSettings = domainSettings;
  d->future = QtConcurrent::run(this, &Password::generate, masterPassword);
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
