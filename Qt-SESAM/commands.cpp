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

#include "commands.h"

// ----------------------------------------------------------------------
// ChangeSpinBoxCommand
// ----------------------------------------------------------------------

ChangeSpinBoxCommand::ChangeSpinBoxCommand(QSpinBox *spinBox, int oldValue, QUndoCommand *parent)
    : QUndoCommand(parent)
    , spinBox(spinBox)
    , oldValue(oldValue)
    , newValue(spinBox->value())
{ /* ... */ }


void ChangeSpinBoxCommand::undo(void)
{
    spinBox->setValue(oldValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


void ChangeSpinBoxCommand::redo(void)
{
    spinBox->setValue(newValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


// ----------------------------------------------------------------------
// ChangeTextEditCommand
// ----------------------------------------------------------------------

ChangeLineEditCommand::ChangeLineEditCommand(QLineEdit *textEdit, const QString &oldValue, QUndoCommand *parent)
    : QUndoCommand(parent)
    , textEdit(textEdit)
    , oldValue(oldValue)
    , newValue(textEdit->text())
{ /* ... */ }


void ChangeLineEditCommand::undo(void)
{
    textEdit->setText(oldValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


void ChangeLineEditCommand::redo(void)
{
    textEdit->setText(newValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


// ----------------------------------------------------------------------
// ChangePlainTextEditCommand
// ----------------------------------------------------------------------

ChangePlainTextEditCommand::ChangePlainTextEditCommand(QPlainTextEdit *textEdit, const QString &oldValue, QUndoCommand *parent)
    : QUndoCommand(parent)
    , textEdit(textEdit)
    , oldValue(oldValue)
    , newValue(textEdit->toPlainText())
{ /* ... */ }


void ChangePlainTextEditCommand::undo(void)
{
    textEdit->setPlainText(oldValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


void ChangePlainTextEditCommand::redo(void)
{
    textEdit->setPlainText(newValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


// ----------------------------------------------------------------------
// ChangeCheckboxCommand
// ----------------------------------------------------------------------

ChangeCheckboxCommand::ChangeCheckboxCommand(QCheckBox *checkBox, bool oldValue, QUndoCommand *parent)
    : QUndoCommand(parent)
    , checkBox(checkBox)
    , oldValue(oldValue)
    , newValue(checkBox->isChecked())
{ /* ... */ }


void ChangeCheckboxCommand::undo(void)
{
    checkBox->setChecked(oldValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


void ChangeCheckboxCommand::redo(void)
{
    checkBox->setChecked(newValue);
    setText(QObject::tr("Change to %1").arg(newValue));
}


