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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class Swatch;
class NameTable;
class SettingsDialog;
class QSettings;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void paletteHovered(QString name);

    void on_action_Remove_Duplicates_triggered();
    void on_action_Remove_Unused_triggered();

    void on_action_Import_From_Image_triggered();

    void on_action_Preferences_triggered();

    void on_action_Open_Background_Palettes_triggered();
    void on_action_Save_Background_Palettes_As_triggered();
    void on_action_Save_Background_Palettes_triggered();

    void on_action_Open_Sprites_Palettes_triggered();
    void on_action_Save_Sprites_Palettes_As_triggered();
    void on_action_Save_Sprites_Palettes_triggered();

    void on_action_Open_CHR_triggered();
    void on_action_Save_CHR_As_triggered();
    void on_action_Save_CHR_triggered();

    void on_action_Open_NameTable_triggered();
    void on_action_Save_NameTable_triggered();
    void on_action_Save_All_NameTables_triggered();

    void on_addNameTableButton_clicked();

    void openRecentBackgroundPalettes();
    void openRecentSpritesPalettes();
    void openRecentCHR();
    void openRecentNameTable();

    void nameTableClicked(int x, int y);

    void setStatus(QString text);

    void onSettingsChanged(); // Called whenever settings are changed

    void setTitle(QString name);

     void closeEvent(QCloseEvent *event);
private:
    void loadPalettes(QString filename, bool background = true);
    void loadCHR(QString filename);
    void loadNameTable(QString filename);

    // Delete and create recent file actions based on QStringLists
    void updateRecentActions();

    Ui::MainWindow *ui;
    SettingsDialog *mSettingsDialog;
    NameTable *mCurrentNameTable; // Current name table
    QSettings *mSettings;
    QStringList mLastPaletteFiles;
    QStringList mLastCHRFiles;
    QStringList mLastNameTableFiles;
    QList<NameTable*> mNameTables;
};

#endif // MAINWINDOW_H
