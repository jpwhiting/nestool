/***************************************************************************
 *   Copyright (C) 2016 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#include "importdialog.h"

#include "ui_importdialog.h"

#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

#include "defines.h"

ImportDialog::ImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog),
    mSettings(new QSettings())
{
    ui->setupUi(this);

    updateButtons();

    connect(ui->filenameComboBox, &QComboBox::currentTextChanged,
            this, &ImportDialog::updateButtons);
    connect(ui->nametableNameLineEdit, &QLineEdit::textChanged,
            this, &ImportDialog::updateButtons);

    int size = mSettings->beginReadArray(kPreviousImportKey);
    for (int i = 0; i < size; ++i) {
        mSettings->setArrayIndex(i);
        ui->filenameComboBox->addItem(mSettings->value(kPreviousPathKey).toString());
    }
    mSettings->endArray();
}

ImportDialog::~ImportDialog()
{
    delete ui;
    delete mSettings;
}

QString ImportDialog::filename() const
{
    return ui->filenameComboBox->currentText();
}

QString ImportDialog::nametableName() const
{
    return ui->nametableNameLineEdit->text();
}

bool ImportDialog::importColors() const
{
    return ui->detectPalettesRadioButton->isChecked();
}

bool ImportDialog::importTiles() const
{
    return ui->importTilesetRadioButton->isChecked();
}

bool ImportDialog::clearTileset() const
{
    return ui->clearTilesetCheckBox->isChecked();
}

void ImportDialog::accept()
{
    // Save chosen settings for quick load later
    int size = ui->filenameComboBox->count();
    if (ui->filenameComboBox->currentIndex() == -1)
        size ++; // Add one for the current text

    mSettings->beginWriteArray(kPreviousImportKey, size);
    for (int i = 0; i < ui->filenameComboBox->count(); ++i) {
        mSettings->setArrayIndex(i);
        mSettings->setValue(kPreviousPathKey, ui->filenameComboBox->itemText(i));
    }
    if (ui->filenameComboBox->currentIndex() == -1) {
        mSettings->setArrayIndex(size - 1);
        mSettings->setValue(kPreviousPathKey, ui->filenameComboBox->currentText());
    }
    mSettings->endArray();

    QDialog::accept();
}

void ImportDialog::on_browseToolButton_clicked()
{
    // Get image filename
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open image file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "Images (*.png)");
    ui->filenameComboBox->setCurrentText(filename);
    QFileInfo info(filename);
    mSettings->setValue(kLastOpenPathKey, info.absolutePath());
}

void ImportDialog::updateButtons()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptableInput());
}

bool ImportDialog::acceptableInput()
{
    return !ui->filenameComboBox->currentText().isEmpty() && !ui->nametableNameLineEdit->text().isEmpty();
}
