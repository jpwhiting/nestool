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

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "nametable.h"
#include "palette.h"
#include "project.h"
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
    mProject(new Project),
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
    ui->nameTable->setPalette(ui->backgroundPalette);
    mCurrentNameTable = mNameTables.at(0);
    setTitle(mCurrentNameTable->getName());
    connect(ui->nameTable, SIGNAL(tileClicked(int,int)), this, SLOT(nameTableClicked(int,int)));

    ui->tileSet->setPalette(ui->backgroundPalette, true);
    ui->tileSet->setPalette(ui->spritesPalette, false);

    connect(mSettingsDialog, SIGNAL(settingsChanged()), this, SLOT(onSettingsChanged()));
    connect(ui->backgroundPalette, SIGNAL(paletteHovered(QString)), this, SLOT(paletteHovered(QString)));
    connect(ui->spritesPalette, SIGNAL(paletteHovered(QString)), this, SLOT(paletteHovered(QString)));
    onSettingsChanged(); // Update scales

    connect(mProject, SIGNAL(loadBackgroundPalette(QString)), this, SLOT(loadBGPalettes(QString)));
    connect(mProject, SIGNAL(loadSpritesPalette(QString)), this, SLOT(loadSPPalettes(QString)));
    connect(mProject, SIGNAL(loadTileset(QString)), this, SLOT(loadTileSet(QString)));
    connect(mProject, SIGNAL(loadNameTables(QStringList)), this, SLOT(loadNameTables(QStringList)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mSettings;
}


void MainWindow::paletteHovered(QString name)
{
    setStatus(name);
}

void MainWindow::on_action_Open_Project_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open project",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "Project (*.ini)");
    if (!filename.isEmpty()) {
        mProject->load(filename);
    }
}

void MainWindow::on_action_Save_Project_As_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                       "Save project",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "Project (*.ini)");
    mProject->saveAs(filename);
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

void MainWindow::importImageSection(QImage *image, int x, int y, int &nextTile, QList<QColor> palette)
{
    Tile *testTile = new Tile(this);
    testTile->setPalette(palette);
    if (nextTile < 256) {

    }
}

void MainWindow::on_action_Import_From_Image_triggered()
{
    // Get image filename
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open image file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "Images (*.png)");
    if (!filename.isEmpty()) {
        // Read image
        QImage image(filename);
        // Figure out how many nametables we need
        int nametablesWide = (image.width() / (32*8));
        int nametablesHigh = (image.height() / (30*8));
        int nametablesNeeded = nametablesWide * nametablesHigh;
        // Create nametables
        while (mNameTables.count() < nametablesNeeded)
            on_addNameTableButton_clicked();

        qDebug() << "need " << nametablesNeeded << "nametables to handle the given image";
        // Figure out palettes from image
        QList<int> attributes = ui->backgroundPalette->calculateFromImage(&image);
//        QList<int> attributes = ui->backgroundPalette->getAttributesFromImage(&image);
        qDebug() << "Got " << attributes.size() << " attributes for given image";
        QList<QList<QColor> > allColors = ui->backgroundPalette->getAllColors();
        // Read tileset from image
        // Fill nametables based on image
        Tile *testTile = new Tile(this);
        int nextTile = 0;
        int attrIndex = 0;
        for (int j = 0; j < image.height(); j+=16) {
            for (int i= 0; i < image.width(); i+=16) {
                NameTable *nt = mNameTables[i/(32*8) + (j/(30*8) * nametablesWide)];
                int x = i/8 % 32;
                int y = j/8 % 30;
                if (attrIndex < attributes.size()) {
                    testTile->setPalette(allColors.at(attributes.at(attrIndex)));
                    nt->setAttr(x, y, attributes.at(attrIndex));
                }
                testTile->setImage(image, i, j); // Top left corner
                int tile = ui->tileSet->hasTile(testTile);
                if (tile == -1) {
                    // We don't have the tile
                    ui->tileSet->setTileData(nextTile, testTile->chrData());
                    tile = nextTile;
                    nextTile++;
                } else if (nextTile <= tile) { // If we matched a later tile
                    nextTile = tile+1; // set nextTile to one past the one matched
                }
                nt->setTile(x, y, tile);
                testTile->setImage(image, i+8, j); // Top right corner
                tile = ui->tileSet->hasTile(testTile);
                if (tile == -1) {
                    ui->tileSet->setTileData(nextTile, testTile->chrData());
                    tile = nextTile;
                    nextTile++;
                } else if (nextTile <= tile) { // If we matched a later tile
                    nextTile = tile+1; // set nextTile to one past the one matched
                }
                nt->setTile(x + 1, y, tile);
                testTile->setImage(image, i, j+8); // Bottom left corner
                tile = ui->tileSet->hasTile(testTile);
                if (tile == -1) {
                    ui->tileSet->setTileData(nextTile, testTile->chrData());
                    tile = nextTile;
                    nextTile++;
                } else if (nextTile <= tile) { // If we matched a later tile
                    nextTile = tile+1; // set nextTile to one past the one matched
                }
                nt->setTile(x, y + 1, tile);
                testTile->setImage(image, i+8, j+8); // Bottom right corner
                tile = ui->tileSet->hasTile(testTile);
                if (tile == -1) {
                    ui->tileSet->setTileData(nextTile, testTile->chrData());
                    tile = nextTile;
                    nextTile++;
                } else if (nextTile <= tile) { // If we matched a later tile
                    nextTile = tile+1; // set nextTile to one past the one matched
                }
                nt->setTile(x + 1, y + 1, tile);
                attrIndex++;
                if (attrIndex > attributes.size()) {
                    QMessageBox::critical(this, "Attribute calculation failure", "Not enough attributes returned by palette scanner");
                    return;
                }
            }
        }
    }
}

void MainWindow::on_action_Preferences_triggered()
{
    mSettingsDialog->show();
}

void MainWindow::on_action_Open_Background_Palettes_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open palettes file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        loadPalettes(filename, true);
        mProject->setBackgroundPaletteFilename(filename);
    }
}

void MainWindow::on_action_Save_Background_Palettes_As_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                       "Save palettes file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        ui->backgroundPalette->saveAs(filename);
        mProject->setBackgroundPaletteFilename(filename);
    }
}

void MainWindow::on_action_Save_Background_Palettes_triggered()
{
    ui->backgroundPalette->save();
}

void MainWindow::on_action_Open_Sprites_Palettes_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open palettes file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        loadPalettes(filename, false);
        mProject->setSpritesPaletteFilename(filename);
    }
}

void MainWindow::on_action_Save_Sprites_Palettes_As_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                       "Save palettes file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        ui->spritesPalette->saveAs(filename);
        mProject->setSpritesPaletteFilename(filename);
    }
}

void MainWindow::on_action_Save_Sprites_Palettes_triggered()
{
    ui->spritesPalette->save();
}

void MainWindow::on_action_Open_CHR_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                       "Open tileset file",
                       mSettings->value(kLastOpenPathKey, QDir::home().absolutePath()).toString(),
                       "NES Tileset (*.chr)");
    if (!filename.isEmpty()) {
        loadTileSet(filename);
        mProject->setTileSetFilename(filename);
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
        mProject->setTileSetFilename(filename);
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
        updateNameTableList();
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
        updateNameTableList();
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
    nameTable->setPalette(ui->backgroundPalette);
    nameTable->toggleShowGrid(ui->nametableGridButton->isChecked());
    connect(nameTable, SIGNAL(tileClicked(int,int)), this, SLOT(nameTableClicked(int,int)));
}

void MainWindow::on_nametableGridButton_toggled(bool checked)
{
    Q_FOREACH(NameTable *nameTable, mNameTables) {
        nameTable->toggleShowGrid(checked);
    }
}

void MainWindow::openRecentBackgroundPalettes()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString filename = action->text().remove('&');
    loadPalettes(filename, true);
    mProject->setBackgroundPaletteFilename(filename);
}

void MainWindow::openRecentSpritesPalettes()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString filename = action->text().remove('&');
    loadPalettes(filename, false);
    mProject->setSpritesPaletteFilename(filename);
}

void MainWindow::openRecentCHR()
{
    QAction *action = qobject_cast<QAction*>(sender());
    QString filename = action->text().remove('&');
    loadTileSet(filename);
    mProject->setTileSetFilename(filename);
}

void MainWindow::openRecentNameTable()
{
    QAction *action = qobject_cast<QAction*>(sender());
    loadNameTable(action->text().remove('&'));
    updateNameTableList();
}

void MainWindow::nameTableClicked(int x, int y)
{
    NameTable *nameTable = qobject_cast<NameTable*>(sender());
    mCurrentNameTable = nameTable;
    setTitle(mCurrentNameTable->getName());
    if (ui->applyPalettesCheckBox->isChecked()) {
        nameTable->setAttr(x, y, ui->backgroundPalette->getCurrentPalette());
    }
    if (ui->applyTilesCheckBox->isChecked()) {
        nameTable->setTile(x, y, ui->tileSet->selectedTile());
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
    if (ui->tileSet->isModified()) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
                                             "Save Tileset?",
                                             "Tileset has been modified. Save?",
                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (answer == QMessageBox::Yes) {
            ui->tileSet->save();
        } else if (answer == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    mSettings->setValue(kWindowGeometryKey, saveGeometry());
    mSettings->setValue(kWindowStateKey, saveState());
}

void MainWindow::loadBGPalettes(QString filename)
{
    loadPalettes(filename, true);
    // Called from project signal, so no need to tell project the filename
}

void MainWindow::loadSPPalettes(QString filename)
{
    loadPalettes(filename, false);
    // Called from project signal, so no need to tell project the filename
}

void MainWindow::loadTileSet(QString filename)
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

void MainWindow::loadNameTables(QStringList filenames)
{
    while (mNameTables.size() < filenames.size())
        on_addNameTableButton_clicked();

    int i = 0;
    Q_FOREACH(QString filename, filenames) {
        mNameTables.at(i)->load(filename);
        ++i;
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

void MainWindow::updateNameTableList()
{
    QStringList filenames;
    Q_FOREACH(NameTable *table, mNameTables) {
        if (!table->getFileName().isEmpty())
            filenames.append(table->getFileName());
    }
    mProject->setNameTables(filenames);
}

void MainWindow::updateRecentActions()
{
    ui->menu_Recent_CHR->clear();
    ui->menu_Recent_Background_Palettes->clear();
    ui->menu_Recent_Sprites_Palettes->clear();
    ui->menu_Recent_NameTable->clear();

    Q_FOREACH(const QString &filename, mLastPaletteFiles) {
        ui->menu_Recent_Background_Palettes->addAction(filename, this,
                SLOT(openRecentBackgroundPalettes()));
        ui->menu_Recent_Sprites_Palettes->addAction(filename, this,
                SLOT(openRecentSpritesPalettes()));
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

void MainWindow::loadPalettes(QString filename, bool background)
{
    if (background ? ui->backgroundPalette->load(filename) :
            ui->spritesPalette->load(filename)) {
        QFileInfo info(filename);
        if (!mLastPaletteFiles.contains(filename)) {
            mLastPaletteFiles.append(filename);
            if (mLastPaletteFiles.count() > mSettingsDialog->maxRecentFiles())
                mLastPaletteFiles.removeFirst();
            updateRecentActions();
        }
        mSettings->setValue(kLastOpenPathKey, info.absolutePath());
    } else {
        QMessageBox::warning(this, "Unable to load palettes file",
                             QString("Unable to load palettes from file %1").arg(filename));
    }
}
