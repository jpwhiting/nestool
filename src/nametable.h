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

#ifndef NAMETABLE_H
#define NAMETABLE_H

#include "tile.h"
#include "tileset.h"

#include <QList>
#include <QWidget>

class NameTable : public QWidget
{
    Q_OBJECT
public:
    explicit NameTable(QWidget *parent = 0);

    void setTileSet(TileSet *tileset); // Tell us the tileset so we can draw
    void setData(char *data);

    // Set all 4 palettes in order so we can apply the right one per tile.
    void setPalette(QList<QColor> colors);

public slots:
private:
    TileSet *mTileSet;
    QList<Tile*> mTiles;
    unsigned char mData[1024]; // Nametable
    QList<QColor> colors; // All 4 palettes in order
};

#endif // NAMETABLE_H