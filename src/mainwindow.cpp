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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "nametable.h"
#include "palette.h"
#include "settingsdialog.h"
#include "swatch.h"

#include <QDebug>

#define kLastOpenPathKey "lastOpenPath"
#define kPreviousPalKey "previousPalFiles"
#define kPreviousCHRKey "previousCHRFiles"
#define kPreviousNameTablesKey "previousNameTableFiles"
#define kPreviousPathKey "path"
#define kWindowGeometryKey "windowGeometry"
#define kWindowStateKey "windowState"

#define VERSION "0.1"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  mSettingsDialog(new SettingsDialog(this)),
  mBgPal{{15, 0, 16, 48}, {15, 1, 33, 49}, {15, 6, 22, 38}, {15, 9, 25, 41}},
  mCurrentPalette(-1),
  mCurrentPal(0),
  mCurrentPalSwatch(0),
  mCurrentNameTable(0),
  mSettings(new QSettings())
{
  ui->setupUi(this);

  restoreGeometry(mSettings->value(kWindowGeometryKey).toByteArray());
  // create docks, toolbars, etc…
  restoreState(mSettings->value(kWindowStateKey).toByteArray());

  // Read in list of last files
  int size = mSettings->beginReadArray(kPreviousPalKey);
  for (int i = 0; i < size; ++i) {
      mSettings->setArrayIndex(i);
      mLastPaletteFiles.append(mSettings->value(kPreviousPathKey).toString());
  }
  mSettings->endArray();

  size = mSettings->beginReadArray(kPreviousCHRKey);
  for (int i = 0; i < size; ++i) {
      mSettings->setArrayIndex(i);
      mLastCHRFiles.append(mSettings->value(kPreviousPathKey).toString());
  }
  mSettings->endArray();

  size = mSettings->beginReadArray(kPreviousNameTablesKey);
  for (int i = 0; i < size; ++i) {
      mSettings->setArrayIndex(i);
      mLastNameTableFiles.append(mSettings->value(kPreviousPathKey).toString());
  }
  mSettings->endArray();

  updateRecentActions();

  // Connect to tileset signals
  connect(ui->tileSet, SIGNAL(setStatus(QString)), this, SLOT(setStatus(QString)));

  // Create one nametable
  mNameTables.append(ui->nameTable);
  ui->nameTable->setTileSet(ui->tileSet);
  mCurrentNameTable = mNameTables.at(0);
  setTitle(mCurrentNameTable->getName());
  connect(ui->nameTable, SIGNAL(tileClicked(int,int)), this, SLOT(nameTableClicked(int,int)));

  // Populate mBasePalette
  int pp = 0;
  for (int i = 0; i < 64; ++i) {
      mBasePalette[i] = QColor(ntscPalette[pp], ntscPalette[pp+1],ntscPalette[pp+2]);
      Swatch *swatch = ui->paletteGroupBox->findChild<Swatch *>(QString("swatch%1").arg(i));
      if (swatch) {
          QColor color(ntscPalette[pp], ntscPalette[pp+1], ntscPalette[pp+2]);
          swatch->setColor(color);
          swatch->setHoverText(QString("Color:$%1").arg(i, 2, 16, QChar('0')));
          connect(swatch, SIGNAL(hovered()), this, SLOT(paletteHovered()));
          mColorIndexes.insert(color.name(), i);
          connect(swatch, SIGNAL(clicked()), this, SLOT(paletteClicked()));
      }
      pp+=3;
  }

  updatePalettes();

  // Connect to palette swatch click signals
  for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
          Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
          connect(swatch, SIGNAL(clicked()), this, SLOT(bgClicked()));
      }
  }

  // Select swatch 0_0
  Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch0_0"));
  swatch->setSelected(true);
  mCurrentPal = &mBgPal[0][0];
  mCurrentPalette = 0;
  mCurrentPalSwatch = swatch;
  QList<QColor> pal;
  pal.append(mBasePalette[mBgPal[0][0]]);
  pal.append(mBasePalette[mBgPal[0][1]]);
  pal.append(mBasePalette[mBgPal[0][2]]);
  pal.append(mBasePalette[mBgPal[0][3]]);
  ui->tileSet->setPalette(pal);

  connect(mSettingsDialog, SIGNAL(settingsChanged()), this, SLOT(onSettingsChanged()));
  onSettingsChanged(); // Update scales
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mSettings;
}

void MainWindow::bgClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
            if (swatch == from) {
                swatch->setSelected(true);
                mCurrentPal = &mBgPal[i][j];
                mCurrentPalette = i;
                mCurrentPalSwatch = swatch;
                QList<QColor> pal;
                pal.append(mBasePalette[mBgPal[i][0]]);
                pal.append(mBasePalette[mBgPal[i][1]]);
                pal.append(mBasePalette[mBgPal[i][2]]);
                pal.append(mBasePalette[mBgPal[i][3]]);
                ui->tileSet->setPalette(pal);
            } else {
                swatch->setSelected(false);
            }
        }
    }
}

void MainWindow::paletteClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    QColor color = from->getColor();
    int index = mColorIndexes.value(color.name());
    if (mCurrentPal != 0) {
        *mCurrentPal = index;
        mCurrentPalSwatch->setColor(color);

        // Send list of QColor to nametable
        QList<QList<QColor> > palettes;
        for (int i = 0; i < 4; ++i) {
            QList<QColor> pal;
            for (int j = 0; j < 4; ++j) {
                pal.append(mBasePalette[mBgPal[i][j]]);
            }
            palettes.append(pal);
        }
        ui->nameTable->setPalettes(palettes);
    }
}

void MainWindow::paletteHovered()
{
    Swatch *swatch = qobject_cast<Swatch*>(sender());
    setStatus(swatch->getHoverText());
}

void MainWindow::on_action_Remove_Duplicates_triggered()
{
    // Find duplicates in tileset
    QList<QPair<int, int> >duplicates = ui->tileSet->duplicateTiles();

    // Create mapping of duplicates
    QMap<int, int> mapping;
    QSet<int> unused;
    for (int i = 0; i < duplicates.size(); ++i) {
        QPair<int, int> pair = duplicates.at(i);
        mapping.insert(pair.second, pair.first);
        unused.insert(pair.second);
        ui->tileSet->clearTile(pair.second);
    }
    // Remove duplicates and update all nametables
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->remapTiles(mapping);
    }

    mapping.clear();
    // Now shift all used tiles to make space at the end
    for (int i = 0; i < 256; ++i) {
        if (unused.contains(i)) {
            for (int j = i+1; j < 256; ++j) {
                if (!unused.contains(j)) {
                    ui->tileSet->copyTile(j, i);
                    unused.remove(i);
                    unused.insert(j);
                    mapping.insert(j, i);
                    j = 256; // Skip to the next unused
                }
            }
        }
    }

    // Now clear out all unused tiles
    for (int i = 0; i < 256; ++i) {
        if (unused.contains(i)) {
            // Clear out the tile
            ui->tileSet->clearTile(i);
        }
    }
    // update all nametables
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->remapTiles(mapping);
    }
    update();
}

void MainWindow::on_action_Remove_Unused_triggered()
{
    QSet<int> usedTiles;
    // Get used tiles from all nametables
    // Find union of used tiles from nametables
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        usedTiles.unite(nameTable->usedTiles());
    }

    // Create mapping of tiles we will remove (shifting tiles up)
    QMap<int, int> mapping;
    for (int i = 0; i < 256; ++i) {
        if (!usedTiles.contains(i)) {
            // Find next used tile > i
            for (int j = i+1; j < 256; ++j) {
                if (usedTiles.contains(j)) {
                    // Move tile data from j to i
                    ui->tileSet->copyTile(j, i);
                    usedTiles.remove(j);
                    usedTiles.insert(i); // Now i is used, j isn't
                    // Add j to i to mapping
                    mapping.insert(j, i);
                    j = 256;
                }
            }
        }
    }

    // Now clear out all unused tiles
    for (int i = 0; i < 256; ++i) {
        if (!usedTiles.contains(i)) {
            // Clear out the tile
            ui->tileSet->clearTile(i);
        }
    }
    // update all nametables
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->remapTiles(mapping);
    }
    update();
}

void MainWindow::on_action_Preferences_triggered()
{
    mSettingsDialog->show();
}

void MainWindow::on_action_Open_Palettes_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open palettes file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        loadPalettes(filename);
    }
}

void MainWindow::on_action_Save_Palettes_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save palettes file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            char pal[16];
            for (int i = 0; i < 4; ++i) {
                pal[i]=mBgPal[0][i];
                pal[i+4]=mBgPal[1][i];
                pal[i+8]=mBgPal[2][i];
                pal[i+12]=mBgPal[3][i];
            }
            file.write(pal, 16);
            file.close();
        }
    }
}

void MainWindow::on_action_Open_CHR_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open tileset file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES Tileset (*.chr)");
    if (!filename.isEmpty()) {
        loadCHR(filename);
    }
}

void MainWindow::on_action_Save_CHR_As_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save tileset file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES Tileset (*.chr)");
    if (!filename.isEmpty()) {
        ui->tileSet->saveAs(filename);
    }
}

void MainWindow::on_action_Save_CHR_triggered()
{
    ui->tileSet->save();
}

void MainWindow::on_action_Open_NameTable_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open nametable file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES NameTable (*.nam)");
    if (!filename.isEmpty()) {
        loadNameTable(filename);
    }
}

void MainWindow::on_action_Save_NameTable_triggered()
{
    // Ask for each nametable filename
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save nametable file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES NameTable (*.nam)");
    if (!filename.isEmpty()) {
        mCurrentNameTable->saveAs(filename, mSettingsDialog->compressNameTables());
    }
}

void MainWindow::on_action_Save_All_NameTables_triggered()
{
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->save(mSettingsDialog->compressNameTables());
    }
}

void MainWindow::on_addNameTableButton_clicked()
{
    NameTable *nameTable = new NameTable(this);
    mNameTables.append(nameTable);
    ui->nameTableContents->layout()->addWidget(nameTable);
    nameTable->setTileSet(ui->tileSet);
    nameTable->setScale(mSettingsDialog->nameTableScale());

    // Send list of QColor to new nametable
    QList<QList<QColor> > palettes;
    for (int i = 0; i < 4; ++i) {
        QList<QColor> pal;
        for (int j = 0; j < 4; ++j) {
            pal.append(mBasePalette[mBgPal[i][j]]);
        }
        palettes.append(pal);
    }
    nameTable->setPalettes(palettes);
    connect(nameTable, SIGNAL(tileClicked(int,int)), this, SLOT(nameTableClicked(int,int)));
}

void MainWindow::updatePalettes()
{
    // Populate default palettes
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
            if (swatch) {
                QColor color(mBasePalette[mBgPal[i][j]]);
                swatch->setColor(color);
            }
        }
    }

    // Send color palette to tileSet
    QList<QColor> pal;
    pal.append(mBasePalette[mBgPal[mCurrentPalette][0]]);
    pal.append(mBasePalette[mBgPal[mCurrentPalette][1]]);
    pal.append(mBasePalette[mBgPal[mCurrentPalette][2]]);
    pal.append(mBasePalette[mBgPal[mCurrentPalette][3]]);
    ui->tileSet->setPalette(pal);

    // Send list of QColor to nametable
    QList<QList<QColor> > palettes;
    for (int i = 0; i < 4; ++i) {
        QList<QColor> pal;
        for (int j = 0; j < 4; ++j) {
            pal.append(mBasePalette[mBgPal[i][j]]);
        }
        palettes.append(pal);
    }
    Q_FOREACH(NameTable* nameTable, mNameTables) {
        nameTable->setPalettes(palettes);
    }
}

void MainWindow::openRecentPalettes()
{
    QAction *action = qobject_cast<QAction*>(sender());
    loadPalettes(action->text().remove('&'));
}

void MainWindow::openRecentCHR()
{
    QAction *action = qobject_cast<QAction*>(sender());
    loadCHR(action->text().remove('&'));
}

void MainWindow::openRecentNameTable()
{
    QAction *action = qobject_cast<QAction*>(sender());
    loadNameTable(action->text().remove('&'));
}

void MainWindow::nameTableClicked(int x, int y)
{
    NameTable *nameTable = qobject_cast<NameTable*>(sender());
    mCurrentNameTable = nameTable;
    setTitle(mCurrentNameTable->getName());
    if (ui->applyPalettesCheckBox->isChecked() && mCurrentPalette != -1) {
        nameTable->setAttr(x, y, mCurrentPalette);
    }
}

void MainWindow::setStatus(QString text)
{
    ui->statusBar->showMessage(text);
}

void MainWindow::onSettingsChanged()
{
    // Update tileset scale
    ui->tileSet->setScale(mSettingsDialog->tileSetScale());
    // Update nametable scales
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->setScale(mSettingsDialog->nameTableScale());
    }
}

void MainWindow::setTitle(QString name)
{
    setWindowTitle(QString("NES Tool v%1 - %2").arg(VERSION).arg(name));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mSettings->setValue(kWindowGeometryKey, saveGeometry());
    mSettings->setValue(kWindowStateKey, saveState());
}

void MainWindow::loadCHR(QString filename)
{
    if (ui->tileSet->load(filename)) {
        if (!mLastCHRFiles.contains(filename)) {
            mLastCHRFiles.append(filename);
            if (mLastCHRFiles.count() > mSettingsDialog->maxRecentFiles())
                mLastCHRFiles.removeFirst();
            updateRecentActions();
        }
        QFile file(filename);
        QFileInfo info(file);
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
    }
}

void MainWindow::loadNameTable(QString filename)
{
    if (mCurrentNameTable->load(filename)) {
        QFileInfo info(filename);
        if (!mLastNameTableFiles.contains(filename)) {
            mLastNameTableFiles.append(filename);
            if (mLastNameTableFiles.count() > mSettingsDialog->maxRecentFiles())
                mLastNameTableFiles.removeFirst();
            updateRecentActions();
        }
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
        setTitle(mCurrentNameTable->getName());
    } else {
        QMessageBox::warning(this, "Unable to load NameTable",
                             QString("Unable to load NameTable from file %1").arg(filename));
    }
}

void MainWindow::updateRecentActions()
{
    ui->menu_Recent_CHR->clear();
    ui->menu_Recent_Palettes->clear();
    ui->menu_Recent_NameTable->clear();

    Q_FOREACH(const QString &filename, mLastPaletteFiles) {
        ui->menu_Recent_Palettes->addAction(filename, this, SLOT(openRecentPalettes()));
    }

    Q_FOREACH(const QString &filename, mLastCHRFiles) {
        ui->menu_Recent_CHR->addAction(filename, this, SLOT(openRecentCHR()));
    }

    Q_FOREACH(const QString &filename, mLastNameTableFiles) {
        ui->menu_Recent_NameTable->addAction(filename, this, SLOT(openRecentNameTable()));
    }

    mSettings->beginWriteArray(kPreviousPalKey, mLastPaletteFiles.count());
    for (int i = 0; i < mLastPaletteFiles.count(); ++i) {
        mSettings->setArrayIndex(i);
        mSettings->setValue(kPreviousPathKey, mLastPaletteFiles.at(i));
    }
    mSettings->endArray();

    mSettings->beginWriteArray(kPreviousCHRKey, mLastCHRFiles.count());
    for (int i = 0; i < mLastCHRFiles.count(); ++i) {
        mSettings->setArrayIndex(i);
        mSettings->setValue(kPreviousPathKey, mLastCHRFiles.at(i));
    }
    mSettings->endArray();

    mSettings->beginWriteArray(kPreviousNameTablesKey, mLastNameTableFiles.count());
    for (int i = 0; i < mLastNameTableFiles.count(); ++i) {
        mSettings->setArrayIndex(i);
        mSettings->setValue(kPreviousPathKey, mLastNameTableFiles.at(i));
    }
    mSettings->endArray();

}

void MainWindow::loadPalettes(QString filename)
{
    QFile file(filename);
    if (file.exists()) {
        if (!mLastPaletteFiles.contains(filename)) {
            mLastPaletteFiles.append(filename);
            if (mLastPaletteFiles.count() > mSettingsDialog->maxRecentFiles())
                mLastPaletteFiles.removeFirst();
            updateRecentActions();
        }
        QFileInfo info(file);
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
        if (info.size() == 16 && file.open(QIODevice::ReadOnly)) {
            char pal[16];
            file.read(pal, 16);
            file.close();
            for (int i = 0; i < 4; ++i) {
                mBgPal[0][i]=pal[i];
                mBgPal[1][i]=pal[i+4];
                mBgPal[2][i]=pal[i+8];
                mBgPal[3][i]=pal[i+12];
            }
            updatePalettes();
        } else {
            QMessageBox::information(this, "Unable to read palettes file", "Palette file should be 16 bytes long");
        }
    }
}
