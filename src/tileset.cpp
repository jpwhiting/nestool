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
#include <QRadioButton>
#include <QToolButton>

#include "edittiledialog.h"

#include "ui_tileset.h"

#include <QDebug>

char zeros[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

TileSet::TileSet(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::Tileset)
{
    ui->setupUi(this);

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
    mCopiedTile = -1;
    mTiles.at(mSelectedTile)->setSelected(true);
    setModified(false);

    connect(ui->bankAButton, SIGNAL(toggled(bool)), this, SLOT(updateTiles()));
    connect(ui->bankBButton, SIGNAL(toggled(bool)), this, SLOT(updateTiles()));
    connect(ui->copyButton, SIGNAL(clicked()), this, SLOT(copySelected()));
    connect(ui->pasteButton, SIGNAL(clicked()), this, SLOT(pasteSelected()));
    connect(ui->gridButton, SIGNAL(toggled(bool)), this, SLOT(toggleShowGrid(bool)));

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
            setModified(false);
            return true;
        } else if (info.size() == 4096) {
            // Load file into the currently selected bank and don't remember filename
            char *start = (ui->bankAButton->isChecked() ? mData : mData + 4096);
            file.read(start, 4096);
            file.close();
            updateTiles();
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

void TileSet::updateTiles()
{
    char *start = (ui->bankAButton->isChecked() ? mData : mData + 4096);
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            int which = i*16+j;
            mTiles.at(which)->setData(start + (which * 16));
        }
    }
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
    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage image = mTiles.at(mSelectedTile)->image();
    clipboard->setImage(image);
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
                }
            }
        }
        setModified(true);
    } else if (mCopiedTile > -1) {
        copyTile(mCopiedTile, mSelectedTile);
    }
}

void TileSet::toggleShowGrid(bool checked)
{
    ui->tileGridLayout->setHorizontalSpacing(checked ? 1 : 0);
    ui->tileGridLayout->setVerticalSpacing(checked ? 1 : 0);
}

void TileSet::setModified(bool modified)
{
    QFileInfo info(mFileName);
    mModified = modified;
    if (modified)
        ui->filenameLabel->setText(QString("%1 *").arg(info.baseName()));
    else
        ui->filenameLabel->setText(info.baseName());
}

void TileSet::editTile(int index)
{
    mEditDialog->setData(mTiles.at(index)->chrData());
    if (mEditDialog->exec() == QDialog::Accepted) {
        mTiles.at(index)->setData(mEditDialog->chrData());
    }
    updateFromTiles(index);
}

char *TileSet::tileData(int tile)
{
    if (tile < 0 || tile > 16*16)
        return NULL;
    return mTiles.at(tile)->chrData();
}

void TileSet::setPalette(QList<QColor> colors)
{
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(colors);
    }
    mEditDialog->setPalette(colors);
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

void TileSet::clearTile(int index)
{
    mTiles.at(index)->setData(zeros);
    updateFromTiles(index);
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
        if (!tile->getSelected()) {
            Q_FOREACH(Tile *t, mTiles) {
                t->setSelected(false);
            }

            tile->setSelected(true);
            int index = mTiles.indexOf(tile);
            mSelectedTile = index;
        } else {
            // Tile is already selected, so open edit mode
            int index = mTiles.indexOf(tile);
            editTile(index);
        }
    }
}

void TileSet::mouseMoveEvent(QMouseEvent *event)
{

}

void TileSet::tileHovered()
{
    Tile *tile = qobject_cast<Tile*>(sender());
    emit setStatus(tile->getHoverText());
}

