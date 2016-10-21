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

#include "nametable.h"

#include <QGridLayout>

NameTable::NameTable(QWidget *parent) : QWidget(parent), mTileSet(0)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 32; ++j) {
            Tile *tile = new Tile(this);
            tile->setFixedSize(QSize(18, 18));
            mTiles.append(tile);
            layout->addWidget(tile, i, j);
        }
    }
}

void NameTable::setTileSet(TileSet *tileset)
{
    mTileSet = tileset;
}

void NameTable::setData(char *data)
{
    for (int i = 0; i < 1024; ++i) {
        mData[i] = (unsigned char)data[i];
    }

    if (mTileSet) { // Update tiles if we have a tileset
        for (int i = 0; i < 960; ++i) {
            char *tileData = mTileSet->tileData(mData[i]);
            if (tileData)
                mTiles.at(i)->setData(tileData);
        }
    }
}

void NameTable::setPalette(QList<QColor> colors)
{
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(colors);
    }
}

