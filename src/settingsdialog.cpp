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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>

#define kNameTableCompressedKey "NameTableCompressed"
#define kRecentFilesKey "RecentFilesCount"
#define kTileSetScaleKey "TileSetScale"
#define kNameTableScaleKey "NameTableScale"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    mSettings(new QSettings())
{
    ui->setupUi(this);

    // Initialize ui controls based on settings values
    if (compressNameTables())
        ui->rleRadioButton->setChecked(true);
    else
        ui->uncompressedRadioButton->setChecked(true);
    ui->tileSetScaleSpinBox->setValue(tileSetScale());
    ui->nameTableScaleSpinBox->setValue(nameTableScale());
    ui->recentFilesSpinBox->setValue(maxRecentFiles());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::compressNameTables() const
{
    return mSettings->value(kNameTableCompressedKey, true).toBool();
}

int SettingsDialog::maxRecentFiles() const
{
    return mSettings->value(kRecentFilesKey, 5).toInt();
}

int SettingsDialog::tileSetScale() const
{
    return mSettings->value(kTileSetScaleKey, 2).toInt();
}

int SettingsDialog::nameTableScale() const
{
    return mSettings->value(kNameTableScaleKey, 3).toInt();
}

void SettingsDialog::on_rleRadioButton_toggled()
{
    mSettings->setValue(kNameTableCompressedKey, ui->rleRadioButton->isChecked());
    emit settingsChanged();
}

void SettingsDialog::on_recentFilesSpinBox_valueChanged()
{
    mSettings->setValue(kRecentFilesKey, ui->recentFilesSpinBox->value());
    emit settingsChanged();
}

void SettingsDialog::on_tileSetScaleSpinBox_valueChanged()
{
    mSettings->setValue(kTileSetScaleKey, ui->tileSetScaleSpinBox->value());
    emit settingsChanged();
}

void SettingsDialog::on_nameTableScaleSpinBox_valueChanged()
{
    mSettings->setValue(kNameTableScaleKey, ui->nameTableScaleSpinBox->value());
    emit settingsChanged();
}
