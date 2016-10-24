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
  connect(ui->nameTable, SIGNAL(tileClicked(int,int)), this, SLOT(nameTableClicked(int,int)));

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

  updateTileset();

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

void MainWindow::on_action_Save_CHR_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save tileset file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES Tileset (*.chr)");
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
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save nametable file",
                                                    mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                                                    "NES NameTable (*.nam)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        char *nameTableData = mCurrentNameTable->getData();
        if (file.open(QIODevice::WriteOnly)) {
            file.write(nameTableData, 1024);
            file.close();
        }
        // Save .h in rle compressed
        bool compress = mSettingsDialog->compressNameTables();
        QFileInfo info(filename);
        QString name = info.baseName();
        QString hFilename = info.canonicalPath() + "/" + name + ".h";
        qDebug() << "writing .h file " << hFilename;
        QFile headerFile(hFilename);

        unsigned char *dst;
        int stat[256];
        int i;
        int size = 1024;

        if (compress) {
            int min=256;
            int tag=255;

            dst = new unsigned char[size*2];
            for (i = 0; i < 256; ++i)
                stat[i] = 0;
            for (i=0; i < size; ++i)
                stat[nameTableData[i]]++;

            for (i = 0; i < 256; ++i) {
                if (stat[i]<min) {
                    min=stat[i];
                    tag = i;
                }
            }

            int pp=0;
            dst[pp++] = tag;
            int len=0;
            int sym=-1;

            for (i=0; i < size; ++i) {
                if (nameTableData[i]!= sym || len == 255 || i == size - 1) {
                    if (nameTableData[i] == sym && i == size - 1)
                        len++;
                    if (len)
                        dst[pp++] = sym;
                    if (len > 1) {
                        if (len == 2) {
                            dst[pp++] = sym;
                        } else {
                            dst[pp++] = tag;
                            dst[pp++] = len - 1;
                        }
                    }
                    sym = nameTableData[i];
                    len = 1;
                } else {
                    len++;
                }
            }
            dst[pp++] = tag;
            dst[pp++] = 0;
            size = pp;
        } else {
            dst = (unsigned char*)nameTableData;
        }
        if (headerFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QString nameString = QString("const unsigned char %1[%2]={\n").arg(name).arg(size);
            headerFile.write(nameString.toStdString().c_str(), nameString.length());

            for (i = 0; i < size; ++i) {
                QString numberString = QString("0x%1").arg(dst[i], 2, 16, QChar('0'));
                if (i<size-1) numberString += ",";
                if ((i&15) == 15 || i == (size-1))
                    numberString += "\n";
                headerFile.write(numberString.toStdString().c_str(), numberString.length());
            }
            QString endString("};\n");
            headerFile.write(endString.toStdString().c_str(), endString.length());
            headerFile.close();
        }
    }
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

void MainWindow::nameTableClicked(int x, int y)
{
    NameTable *nameTable = qobject_cast<NameTable*>(sender());
    mCurrentNameTable = nameTable;
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

void MainWindow::loadCHR(QString filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        if (!mLastCHRFiles.contains(filename)) {
            mLastCHRFiles.append(filename);
            if (mLastCHRFiles.count() > mSettingsDialog->maxRecentFiles())
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
            if (mLastNameTableFiles.count() > mSettingsDialog->maxRecentFiles())
                mLastNameTableFiles.removeFirst();
            updateRecentActions();
        }
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
        if (info.size() == 1024 || info.size() == 960) {
            char nametableData[1024];
            if (info.size() == 1024) {
                file.read(nametableData, 1024);
            } else {
                file.read(nametableData, 960);
                for (int i = 960; i < 1024; ++i)
                    nametableData[i] = 0;
            }
            mCurrentNameTable->setData(nametableData);
            mCurrentNameTable->setFileName(filename);
            file.close();
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
