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

#include <QGridLayout>

TileSet::TileSet(QWidget *parent) : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            Tile *tile = new Tile(this);
            tile->setFixedSize(QSize(16, 16));
            tile->setHoverText(QString("Tile: $%1").arg(i*16+j, 2, 16, QChar('0')));
            connect(tile, SIGNAL(hovered()), this, SLOT(tileHovered()));
            mTiles.append(tile);
            layout->addWidget(tile, i, j);
        }
    }
}

void TileSet::setData(char *data)
{
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            int which = i*16+j;
            mTiles.at(which)->setData(data + (which * 16));
        }
    }
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

void TileSet::tileHovered()
{
    Tile *tile = qobject_cast<Tile*>(sender());
    emit setStatus(tile->getHoverText());
}

