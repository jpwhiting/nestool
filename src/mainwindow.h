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
    void bgClicked();

    void paletteClicked();

    void on_action_Open_Palettes_triggered();
    void on_action_Save_Palettes_triggered();

    void on_action_Open_CHR_triggered();
    void on_action_Save_CHR_triggered();

    void on_action_Open_NameTable_triggered();
    void on_action_Save_NameTable_triggered();

    void on_bankAButton_toggled(bool set);
    void on_bankBButton_toggled(bool set);

    void updatePalettes();
    void openRecentPalettes();
    void updateTileset();
    void openRecentCHR();
    void openRecentNameTable();

    void nameTableClicked(int x, int y);
private:
    void loadPalettes(QString filename);
    void loadCHR(QString filename);
    void loadNameTable(QString filename);

    // Delete and create recent file actions based on QStringLists
    void updateRecentActions();

    Ui::MainWindow *ui;
    QColor mBasePalette[64];
    unsigned char mBgPal[4][4]; // Palettes
    char mChr[8192]; // Tileset
    int mCurrentPalette; // Which palette to use
    unsigned char *mCurrentPal; // Which color is selected
    Swatch *mCurrentPalSwatch;
    QMap<QString, int> mColorIndexes; // Which color is which index in nes palette
    QSettings *mSettings;
    QStringList mLastPaletteFiles;
    QStringList mLastCHRFiles;
    QStringList mLastNameTableFiles;
    QList<NameTable*> mNameTables;
};

#endif // MAINWINDOW_H
