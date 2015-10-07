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

#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <QUndoCommand>
#include <QSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>

#include "easyselectorwidget.h"

class ChangeSpinBoxCommand : public QUndoCommand
{
public:
    enum { Id = 0xa4757d6a };
    ChangeSpinBoxCommand(QSpinBox *spinBox, int oldValue, QUndoCommand *parent = Q_NULLPTR);
    void undo(void) Q_DECL_OVERRIDE;
    void redo(void) Q_DECL_OVERRIDE;
    // bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    int id(void) const Q_DECL_OVERRIDE { return Id; }

private:
    QSpinBox *spinBox;
    int oldValue;
    int newValue;
};


class ChangeLineEditCommand : public QUndoCommand
{
public:
    enum { Id = 0x540b66f0 };
    ChangeLineEditCommand(QLineEdit *textEdit, const QString &oldValue, QUndoCommand *parent = Q_NULLPTR);
    void undo(void) Q_DECL_OVERRIDE;
    void redo(void) Q_DECL_OVERRIDE;
    int id(void) const Q_DECL_OVERRIDE { return Id; }

private:
    QLineEdit *textEdit;
    QString oldValue;
    QString newValue;
};


class ChangePlainTextEditCommand : public QUndoCommand
{
public:
    enum { Id = 0x3d17cf13 };
    ChangePlainTextEditCommand(QPlainTextEdit *textEdit, const QString &oldValue, QUndoCommand *parent = Q_NULLPTR);
    void undo(void) Q_DECL_OVERRIDE;
    void redo(void) Q_DECL_OVERRIDE;
    int id(void) const Q_DECL_OVERRIDE { return Id; }

private:
    QPlainTextEdit *textEdit;
    QString oldValue;
    QString newValue;
};


class ChangeCheckboxCommand : public QUndoCommand
{
public:
    enum { Id = 0x3d17cf13 };
    ChangeCheckboxCommand(QCheckBox *checkBox, bool oldValue, QUndoCommand *parent = Q_NULLPTR);
    void undo(void) Q_DECL_OVERRIDE;
    void redo(void) Q_DECL_OVERRIDE;
    int id(void) const Q_DECL_OVERRIDE { return Id; }

private:
    QCheckBox *checkBox;
    bool oldValue;
    bool newValue;
};


class ChangeEasySelectorCommand : public QUndoCommand
{
public:
  enum { Id = 0x249c15ef };
  ChangeEasySelectorCommand(EasySelectorWidget *easySelector, int oldLength, int oldComplexity, QUndoCommand *parent = Q_NULLPTR);
  void undo(void) Q_DECL_OVERRIDE;
  void redo(void) Q_DECL_OVERRIDE;
  int id(void) const Q_DECL_OVERRIDE { return Id; }

private:
    EasySelectorWidget *easySelector;
    int oldLength;
    int oldComplexity;
    int newLength;
    int newComplexity;
};



#endif // __COMMANDS_H_

