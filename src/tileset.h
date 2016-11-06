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

class QLabel;
class QRadioButton;

class TileSet : public QWidget
{
    Q_OBJECT
public:
    explicit TileSet(QWidget *parent = 0);

    // Load tile data from the given file in chr format
    // Return true if it loaded
    bool load(QString &filename);
    void save(); // Need filename loaded or call saveAs first
    void saveAs(QString filename);

    char *tileData(int tile); // Get the CHR data for a given tile

    void setPalette(QList<QColor> colors);

    void setScale(int scale); // Set scale for tiles

    void copyTile(int from, int to); // Copy tile data from tile from to tile to
    void clearTile(int index); // Clear out the given tile

    QList<QPair<int, int> > duplicateTiles(); // Find all duplicate tiles
signals:
    void setStatus(QString text); // Signal to change the status bar message
private slots:
    void tileHovered(); // Slot for when a tile is hovered over
    void updateTiles();

private:
    QList<Tile*> mTiles;
    char mData[8192]; // chr data
    QRadioButton *mBankAButton;
    QRadioButton *mBankBButton;
    QLabel *mFileNameLabel; // Label to show filename
    QString mFileName; // Last Filename used to load or save this NameTable
};

#endif // TILESET_H
