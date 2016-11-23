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
class EditTileDialog;

namespace Ui {
class Tileset;
}

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

    int selectedTile() const;

    char *tileData(int tile); // Get the CHR data for a given tile

    void setPalette(QList<QColor> colors);

    void setScale(int scale); // Set scale for tiles

    void copyTile(int from, int to); // Copy tile data from tile from to tile to
    void clearTile(int index); // Clear out the given tile

    QList<QPair<int, int> > duplicateTiles(); // Find all duplicate tiles

    bool isModified() const; // Whether the tileset in memory has been modified
signals:
    void setStatus(QString text); // Signal to change the status bar message
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void tileHovered(); // Slot for when a tile is hovered over
    void updateTiles();
    void updateFromTiles(int index); // Update mData based on tile contents
    void copySelected(); // Copy button clicked
    void pasteSelected(); // Paste button clicked
    void toggleShowGrid(bool checked);

private:
    void setModified(bool modified);
    void editTile(int index);
    Ui::Tileset *ui;
    QList<Tile*> mTiles;
    char mData[8192]; // chr data
    QString mFileName; // Last Filename used to load or save this NameTable
    int mSelectedTile; // Which tile is currently selected
    EditTileDialog *mEditDialog; // Dialog to edit tiles
    int mCopiedTile; // Last tile "Copied"
    bool mModified;
};

#endif // TILESET_H
