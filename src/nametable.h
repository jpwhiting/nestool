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

class QLabel;

class NameTable : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString filename WRITE setFileName READ getFileName)
public:
    explicit NameTable(QWidget *parent = 0);

    void setTileSet(TileSet *tileset); // Tell us the tileset so we can draw
    void setData(char *data);

    // Set all 4 palettes in order so we can apply the right one per tile.
    void setPalettes(QList<QList<QColor> > colors);

    // Set the given x, y, to the given palette
    void setAttr(int x, int y, int pal);

    QString getName() const;
    QString getFileName() const;
    void setFileName(QString &filename);

    void setScale(int scale); // Set scale for tiles

    char *getData() const;

    QSet<int> usedTiles() const;
    void remapTiles(QMap<int, int> mapping); // Update nametable using the given mapping
Q_SIGNALS:
    void tileClicked(int x, int y);

private Q_SLOTS:
    void tileClicked();

private:
    // Get the palette for a given tile
    int getAttr(int x, int y);
    TileSet *mTileSet;
    QList<Tile*> mTiles;
    unsigned char mData[1024]; // Nametable
    unsigned char *mAttrs; // Attributes of the name table
    QList<QList<QColor> > mPalettes; // All 4 palettes in order
    QLabel *mFileNameLabel; // Label to show filename
};

#endif // NAMETABLE_H
