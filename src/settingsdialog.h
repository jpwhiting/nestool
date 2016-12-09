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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class QSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    bool compressNameTables() const;
    int maxRecentFiles() const;
    int tileSetScale() const;
    int nameTableScale() const;

signals:
    void settingsChanged(); // Signal that some settings have changed

private slots:
    void on_rleRadioButton_toggled();
    void on_recentFilesSpinBox_valueChanged();
    void on_tileSetScaleSpinBox_valueChanged();
    void on_nameTableScaleSpinBox_valueChanged();


private:
    Ui::SettingsDialog *ui;
    QSettings *mSettings;
};

#endif // SETTINGSDIALOG_H
