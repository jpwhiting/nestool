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

#ifndef TILESET_H
#define TILESET_H

#include "tile.h"

#include <QList>
#include <QWidget>

class TileSet : public QWidget
{
    Q_OBJECT
public:
    explicit TileSet(QWidget *parent = 0);

    void setData(char *data);

    char *tileData(int tile); // Get the CHR data for a given tile

    void setPalette(QList<QColor> colors);

    void setScale(int scale); // Set scale for tiles

    void copyTile(int from, int to); // Copy tile data from tile from to tile to
    void clearTile(int index); // Clear out the given tile
signals:
    void setStatus(QString text); // Signal to change the status bar message
private slots:
    void tileHovered(); // Slot for when a tile is hovered over
private:
    QList<Tile*> mTiles;
};

#endif // TILESET_H
