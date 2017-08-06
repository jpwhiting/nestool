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

#include "tileset.h"

#include <QClipboard>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QRadioButton>
#include <QSettings>
#include <QToolButton>

#include "defines.h"
#include "edittiledialog.h"
#include "palette.h"

#include "ui_tileset.h"

#include <QDebug>

char zeros[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

TileSet::TileSet(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Tileset),
      mSettings(new QSettings())
{
    ui->setupUi(this);

    mRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    mRubberBand->raise();

    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            Tile *tile = QWidget::findChild<Tile *>(QString("tile_%1").arg(i*16+j));
            tile->setFixedSize(QSize(16, 16));
            tile->setHoverText(QString("Tile: $%1").arg(i*16+j, 2, 16, QChar('0')));
            connect(tile, SIGNAL(hovered()), this, SLOT(tileHovered()));
            mTiles.append(tile);
        }
    }

    mSelectedTile = 0;
    mRubberBand->show();
    mSelectedWidth = 1;
    mSelectedHeight = 1;
    updateSelection();
    mCopiedTile = -1;
//    mTiles.at(mSelectedTile)->setSelected(true);
    setModified(false);

    connect(ui->bankAButton, SIGNAL(toggled(bool)), this, SLOT(updateTiles()));
    connect(ui->bankBButton, SIGNAL(toggled(bool)), this, SLOT(updateTiles()));
    connect(ui->copyButton, SIGNAL(clicked()), this, SLOT(copySelected()));
    connect(ui->pasteButton, SIGNAL(clicked()), this, SLOT(pasteSelected()));
    connect(ui->swapButton, SIGNAL(clicked()), this, SLOT(swapSelected()));
    connect(ui->gridButton, SIGNAL(toggled(bool)), this, SLOT(toggleShowGrid(bool)));

    ui->gridButton->setChecked(mSettings->value(kTilesetGridKey, false).toBool());

    mEditDialog = new EditTileDialog(this);
}

bool TileSet::load(QString &filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        if (info.size() == 8192) {
            mFileName = filename; // Only update mFileName if importing an 8k chr
            ui->filenameLabel->setText(info.baseName());
            file.read(mData, 8192);
            file.close();
            updateTiles();
            emit tilesChanged();
            setModified(false);
            return true;
        } else if (info.size() == 4096) {
            // Load file into the currently selected bank and don't remember filename
            char *start = (ui->bankAButton->isChecked() ? mData : mData + 4096);
            file.read(start, 4096);
            file.close();
            updateTiles();
            emit tilesChanged();
            setModified(true);
            return true;
        } else {
            // Check size and import accordingly
        }
    }
    return false;
}

void TileSet::save()
{
    QFile file(mFileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(mData, 8192);
        file.close();
    }
    setModified(false);
}

void TileSet::saveAs(QString filename)
{
    QFileInfo info(filename);
    mFileName = filename;
    ui->filenameLabel->setText(info.baseName());
    save();
    setModified(false);
}

int TileSet::selectedTile() const
{
    return mSelectedTile;
}

int TileSet::selectedTilesWidth() const
{
    return mSelectedWidth;
}

int TileSet::selectedTilesHeight() const
{
    return mSelectedHeight;
}

void TileSet::updateTiles()
{
    char *start = (ui->bankAButton->isChecked() ? mData : mData + 4096);
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            int which = i*16+j;
            mTiles.at(which)->setData(start + (which * 16));
        }
    }
    ui->backgroundCheckBox->setChecked(ui->bankAButton->isChecked());
    paletteChanged();
}

void TileSet::updateFromTiles(int index)
{
    char *start = (ui->bankAButton->isChecked() ? mData : mData + 4096);
    char *data = mTiles.at(index)->chrData();
    start = start + (index * 16);
    for (int x = 0; x < 16; ++x) {
        *start++ = data[x];
    }
    setModified(true);
}

void TileSet::copySelected()
{
    // Save selected tile chr data for pasting
    mCopiedTile = mSelectedTile;
    mCopiedWidth = mSelectedWidth;
    mCopiedHeight = mSelectedHeight;

    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage result = QImage(8 * mSelectedWidth, 8*mSelectedHeight, QImage::Format_RGB32);
    QPainter painter (&result);
    for (int c = 0; c < mSelectedWidth; ++c) {
        for (int r = 0; r < mSelectedHeight; ++r) {
            QImage image = mTiles.at(mSelectedTile + c + r*16)->image();
            painter.drawImage(c*8, r*8, image);
        }
    }
    ui->swapButton->setEnabled(true);
    clipboard->setImage(result);
}

void TileSet::pasteSelected()
{
    // Overwrite currently selected tile with copied chr data
    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage image = clipboard->image();
    if (!image.isNull()) { // TODO: Check the size of the image
        for (int i= 0; i < image.width(); i+=8) {
            for (int j = 0; j < image.height(); j+=8) {
                int which = mSelectedTile + (i/8) + ((j/8)*16);
                if (which < mTiles.size()) {
                    mTiles.at(which)->setImage(image, i, j);
                    updateFromTiles(which);
                }
            }
        }
        setModified(true);
    } else if (mCopiedTile > -1) {
        copyTile(mCopiedTile, mSelectedTile);
    }
}

void TileSet::swapSelected()
{
    if (mCopiedTile > -1) {
        for (int i = 0; i < mCopiedWidth; ++i) {
            for (int j = 0; j < mCopiedHeight; ++j) {
                swapTiles(mCopiedTile + i + (j*16), mSelectedTile + i + (j*16));
            }
        }
    }
}

void TileSet::toggleShowGrid(bool checked)
{
    ui->tileGridLayout->setHorizontalSpacing(checked ? 1 : 0);
    ui->tileGridLayout->setVerticalSpacing(checked ? 1 : 0);
    mSettings->setValue(kTilesetGridKey, checked);
    update();
}

void TileSet::paletteChanged()
{
    QList<QColor> colors = (ui->backgroundCheckBox->isChecked() ?
                            mBackgroundPalette->getCurrentPaletteColors() :
                            mSpritePalette->getCurrentPaletteColors());
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(colors);
    }
    mEditDialog->setPalette(colors);
}

void TileSet::setModified(bool modified)
{
    QFileInfo info(mFileName);
    mModified = modified;
    if (modified) {
        ui->filenameLabel->setText(QString("%1 *").arg(info.baseName()));
        emit tilesChanged();
    } else {
        ui->filenameLabel->setText(info.baseName());
    }
}

void TileSet::editTile(int index)
{
    mEditDialog->setData(mTiles.at(index)->chrData());
    if (mEditDialog->exec() == QDialog::Accepted) {
        mTiles.at(index)->setData(mEditDialog->chrData());
    }
    updateFromTiles(index);
}

void TileSet::updateSelection()
{
    int x = mTiles.at(mSelectedTile)->x();
    int y = mTiles.at(mSelectedTile)->y();
    int grid = ui->gridButton->isChecked() ? 1 : 0;
    int w = mSelectedWidth * (mTiles.at(0)->width() + grid);
    int h = mSelectedHeight * (mTiles.at(0)->height() + grid);
    mRubberBand->setGeometry(x, y, w, h);
}

char *TileSet::tileData(int tile)
{
    if (tile < 0 || tile > 16*16)
        return NULL;
    return mTiles.at(tile)->chrData();
}

void TileSet::setTileData(int tile, char *data)
{
    if (tile < 0 || tile >= mTiles.size()) {
        qDebug() << "Tried to set tile data for tile " << tile
                 << " which is out of range for " << mTiles.size();
        return;
    }
    mTiles.at(tile)->setData(data);
    updateFromTiles(tile);
}

int TileSet::hasTile(Tile *tile)
{
    for (int i = 0; i < mTiles.size(); ++i) {
        if (mTiles.at(i)->identical(tile))
            return i;
    }
    return -1;
}

void TileSet::setPalette(Palette *pal, bool background)
{
    if (background) {
        mBackgroundPalette = pal;
    } else {
        mSpritePalette = pal;
    }
    connect(pal, SIGNAL(currentPaletteChanged()), this, SLOT(paletteChanged()));
    paletteChanged();
}

void TileSet::setScale(int scale)
{
    // TODO: Add some sanity checking
    if (scale < 1 || scale > 6)
        return;
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setFixedSize(QSize(8*scale, 8*scale));
    }
}

void TileSet::copyTile(int from, int to)
{
    mTiles.at(to)->setData(mTiles.at(from)->chrData());
    updateFromTiles(to);
}

void TileSet::swapTiles(int first, int second)
{
    char data[16];
    char *d2 = mTiles.at(first)->chrData();
    for (int i = 0; i < 16; ++i) {
        data[i] = d2[i];
    }

    mTiles.at(first)->setData(mTiles.at(second)->chrData());
    mTiles.at(second)->setData(data);
    updateFromTiles(first);
    updateFromTiles(second);
    emit tilesSwapped(first, second);
    setModified(true);
}

void TileSet::clearTile(int index)
{
    mTiles.at(index)->setData(zeros);
    updateFromTiles(index);
}

void TileSet::clear()
{
    for (int i = 0; i < mTiles.size(); ++i) {
        clearTile(i);
    }
}

QList<QPair<int, int> > TileSet::duplicateTiles()
{
    QList<QPair<int, int> >duplicates;
    for (int i = 0; i < mTiles.size() - 1; ++i) {
        for (int j = i + 1; j < mTiles.size(); ++j) {
            if (mTiles.at(i)->identical(mTiles.at(j))) {
                QPair<int, int> pair(i, j);
                duplicates.append(pair);
            }
        }
    }
    return duplicates;
}

bool TileSet::isModified() const
{
    return mModified;
}

void TileSet::mousePressEvent(QMouseEvent *event)
{
    // Find the tile
    Tile *tile = qobject_cast<Tile*>(childAt(event->x(), event->y()));
    if (tile) {
        int index = mTiles.indexOf(tile);
        if (index != mSelectedTile) {
            if (event->modifiers() == Qt::ShiftModifier) {
                int newX = index%16;
                int newY = index/16;
                int oldX = mSelectedTile%16;
                int oldY = mSelectedTile/16;
                // Is this new tile less than the previously selected tile?
                mSelectedWidth = qAbs(newX - oldX) + 1;
                mSelectedHeight = qAbs(newY - oldY) + 1;

                if (newY < oldY && newX > oldX) {
                    // Higher row, but to the right
                    mSelectedTile = oldX + newY * 16;
                } else if (newX < oldX && newY > oldY) {
                    // It's to the left, but lower
                    mSelectedTile = newX + oldY * 16;
                } else if (index < mSelectedTile) {
                    // To the left and/or above
                    mSelectedTile = index;
                }
            } else {
                mSelectedTile = index;
                mSelectedWidth = 1;
                mSelectedHeight = 1;
            }
            updateSelection();
            update();
        } else {
            // Tile is already selected, so open edit mode
            editTile(index);
        }
    }
}

void TileSet::mouseMoveEvent(QMouseEvent *event)
{

}

void TileSet::resizeEvent(QResizeEvent *event)
{
    updateSelection();
}

void TileSet::tileHovered()
{
    Tile *tile = qobject_cast<Tile*>(sender());
    emit setStatus(tile->getHoverText());
}
