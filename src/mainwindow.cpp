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
#include "swatch.h"

#include <QDebug>

// Keep track of last 5 filenames of each type
#define kLastFileCount 5

#define kLastOpenPathKey "lastOpenPath"
#define kPreviousPalKey "previousPalFiles"
#define kPreviousCHRKey "previousCHRFiles"
#define kPreviousNameTablesKey "previousNameTableFiles"
#define kPreviousPathKey "path"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  mBgPal{{15, 0, 16, 48}, {15, 1, 33, 49}, {15, 6, 22, 38}, {15, 9, 25, 41}},
  mCurrentPal(0),
  mCurrentPalSwatch(0),
  mSettings(new QSettings())
{
  ui->setupUi(this);

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

  // Create one nametable
  mNameTables.append(ui->nameTable);
  ui->nameTable->setTileSet(ui->tileSet);

  // Clear out mChr
  for (int i = 0; i < 8192; ++i) {
      mChr[i] = 0;
  }

  // Populate mBasePalette
  int pp = 0;
  for (int i = 0; i < 64; ++i) {
      mBasePalette[i] = QColor(ntscPalette[pp], ntscPalette[pp+1],ntscPalette[pp+2]);
      Swatch *swatch = ui->paletteGroupBox->findChild<Swatch *>(QString("swatch%1").arg(i));
      if (swatch) {
          QColor color(ntscPalette[pp], ntscPalette[pp+1], ntscPalette[pp+2]);
          swatch->setColor(color);
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

  updateTileset();
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
                ui->nameTable->setPalette(pal);
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
    }
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
    QString filename = QFileDialog::getSaveFileName(this, "Save palettes file", QDir::home().absolutePath(), "NES Palettes (*.pal)");
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

void MainWindow::on_action_Save_CHR_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save tileset file", QDir::home().absolutePath(), "NES Tileset (*.chr)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(mChr, 8192);
            file.close();
        }
    }
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
}

void MainWindow::on_bankAButton_toggled(bool set)
{
    if (set)
        updateTileset();
}

void MainWindow::on_bankBButton_toggled(bool set)
{
    if (set)
        updateTileset();
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
}

void MainWindow::openRecentPalettes()
{
    QAction *action = qobject_cast<QAction*>(sender());
    loadPalettes(action->text().remove('&'));
}

void MainWindow::updateTileset()
{
    if (ui->bankAButton->isChecked())
        ui->tileSet->setData(mChr);
    else
        ui->tileSet->setData(mChr + 4096);
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

void MainWindow::loadCHR(QString filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        if (!mLastCHRFiles.contains(filename)) {
            mLastCHRFiles.append(filename);
            if (mLastCHRFiles.count() > kLastFileCount)
                mLastCHRFiles.removeFirst();
            updateRecentActions();
        }
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
        if (info.size() == 8192) {
            file.read(mChr, 8192);
            file.close();
            updateTileset();
        } else if (info.size() == 4096) {

        } else {
            // Check size and import accordingly
        }
    }
}

void MainWindow::loadNameTable(QString filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        if (!mLastNameTableFiles.contains(filename)) {
            mLastNameTableFiles.append(filename);
            if (mLastNameTableFiles.count() > kLastFileCount)
                mLastNameTableFiles.removeFirst();
            updateRecentActions();
        }
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
        if (info.size() == 1024) {
            char nametableData[1024];
            file.read(nametableData, 1024);
            ui->nameTable->setData(nametableData);
            file.close();
        } else if (info.size() == 960) {

        } else {
            // Check size and import accordingly
        }
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
            if (mLastPaletteFiles.count() > kLastFileCount)
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
