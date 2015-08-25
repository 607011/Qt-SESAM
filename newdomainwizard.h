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

#ifndef __NEWDOMAINWIZARD_H_
#define __NEWDOMAINWIZARD_H_

#include <QDialog>
#include <QString>


namespace Ui {
class NewDomainWizard;
}

class NewDomainWizard : public QDialog
{
  Q_OBJECT

public:
  explicit NewDomainWizard(QWidget *parent = nullptr);
  ~NewDomainWizard();

  void clear(void);
  QString domain(void) const;
  QString username(void) const;
  QString legacyPassword(void) const;
  int iterations(void) const;
  int passwordLength(void) const;
  QString salt_base64(void) const;
  QString notes(void) const;
  QString usedCharacters(void) const;
  bool forceLowercase(void) const;
  bool forceUppercase(void) const;
  bool forceDigits(void) const;
  bool forceExtra(void) const;
  void setForceLowercase(bool);
  void setForceUppercase(bool);
  void setForceDigits(bool);
  void setForceExtra(bool);

private slots:
  void addLowercaseToUsedCharacters(void);
  void addUppercaseToUsedCharacters(void);
  void addDigitsToUsedCharacters(void);
  void addExtraCharactersToUsedCharacters(void);
  void onUsedCharactersChanged(void);
  void renewSalt(void);

private:
  Ui::NewDomainWizard *ui;
};

#endif // __NEWDOMAINWIZARD_H_
