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

class QGridLayout;
class QLabel;
class Palette;

class NameTable : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ getFileName)
public:
    explicit NameTable(QWidget *parent = 0);

    void setTileSet(TileSet *tileset); // Tell us the tileset so we can draw
    bool load(QString filename);
    void save(bool compress); // Need filename loaded or call saveAs first
    void saveAs(QString filename, bool compress);

    void setData(char *data);

    // Set all 4 palettes in order so we can apply the right one per tile.
    void setPalette(Palette *pal);
    // Set the given coordinate to the given tile
    void setTile(int x, int y, int tile);

    // Set whether this nametable is the selected nametable
    // When selected, show the rubberband(s)
    void setSelected(bool selected);

    // Set the given x, y, to the given palette
    void setAttr(int x, int y, int pal);

    void toggleShowGrid(bool checked);

    QString getName() const;
    QString getFileName() const;

    // Set filename without saving
    void setFilename(const QString &filename);

    void setScale(int scale); // Set scale for tiles

    char *getData() const;

    QSet<int> usedTiles() const;
    void remapTiles(QMap<int, int> mapping); // Update nametable using the given mapping

public slots:
    void tilesSwapped(int first, int second);
    // Shift the whole nametable down (or up if down is false) count tiles
    void shiftTilesVertically(int count, bool down);
    // Shift the whole nametable right (or left if right is false) count tiles
    void shiftTilesHorizontally(int count, bool right);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void tileClicked(int x, int y);
    void setStatus(QString text); // change status bar message

private Q_SLOTS:
    void tileClicked();
    void paletteChanged();
    void tilesChanged();
    void tileHovered();

private:
    // Get the palette for a given tile
    int getAttr(int x, int y);
    TileSet *mTileSet;
    QList<Tile*> mTiles;
    unsigned char mData[1024]; // Nametable
    unsigned char *mAttrs; // Attributes of the name table
    QLabel *mFileNameLabel; // Label to show filename
    QString mFileName; // Last Filename used to load or save this NameTable
    Palette *mPalette; // Nametable palette
    QGridLayout *mGridLayout; // Grid layout of tiles
    QRubberBand *mNameRubberBand; // Rubber band to show on name if selected
    bool mSelected;
};

#endif // NAMETABLE_H
