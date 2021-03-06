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

#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QRubberBand>

#include "palette.h"

#include <QDebug>

NameTable::NameTable(QWidget *parent) :
    QWidget(parent),
    mTileSet(0),
    mNameRubberBand(0),
    mSelected(false)
{
    mNameRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    mNameRubberBand->raise();

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    mFileNameLabel = new QLabel(this);

    mGridLayout = new QGridLayout(this);
    mGridLayout->setVerticalSpacing(0);
    mGridLayout->setHorizontalSpacing(0);
    mGridLayout->setContentsMargins(0, 0, 0, 0);

    mAttrs = &mData[960];

    int NAMETABLES [4] = { 8192, 9216, 10240, 11264 };
    int ATTRTABLES [4] = { 9152, 10176, 11200, 12224 };

    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 32; ++j) {
            int nameOffset = i*32 + j;
            int attrOffset = (j - (j % 4)) / 4;
            attrOffset += ((i - (i % 4)) / 4) * 8;
            Tile *tile = new Tile(this);

            tile->setFixedSize(QSize(24, 24));
            tile->setHoverText(QString("Nametable offset: $%1 ($%2, $%3, $%4, $%5)     Attribute offset: $%6 ($%7, $%8, $%9, $%10)")
                .arg(nameOffset, 4, 16, QChar('0'))
                .arg(NAMETABLES[0] + nameOffset, 4, 16, QChar('0'))
                .arg(NAMETABLES[1] + nameOffset, 4, 16, QChar('0'))
                .arg(NAMETABLES[2] + nameOffset, 4, 16, QChar('0'))
                .arg(NAMETABLES[3] + nameOffset, 4, 16, QChar('0'))
                .arg(attrOffset, 2, 16, QChar('0'))
                .arg(ATTRTABLES[0] + attrOffset, 4, 16, QChar('0'))
                .arg(ATTRTABLES[1] + attrOffset, 4, 16, QChar('0'))
                .arg(ATTRTABLES[2] + attrOffset, 4, 16, QChar('0'))
                .arg(ATTRTABLES[3] + attrOffset, 4, 16, QChar('0')));
            connect(tile, SIGNAL(hovered()), this, SLOT(tileHovered()));
            mTiles.append(tile);
            mGridLayout->addWidget(tile, i, j);
        }
    }

    for (int i = 0; i < 1024; ++i) {
        mData[i] = 0;
    }
    setLayout(vLayout);
    vLayout->addWidget(mFileNameLabel);
    vLayout->addLayout(mGridLayout);
    mNameRubberBand->setGeometry(mFileNameLabel->geometry());
    setMouseTracking(true);
}

void NameTable::setTileSet(TileSet *tileset)
{
    mTileSet = tileset;
    connect(mTileSet, SIGNAL(tilesChanged()), this, SLOT(tilesChanged()));
    tilesChanged();
}

bool NameTable::load(QString filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        if (info.size() == 1024 || info.size() == 960) {
            char nametableData[1024];
            if (info.size() == 1024) {
                file.read(nametableData, 1024);
            } else {
                file.read(nametableData, 960);
                for (int i = 960; i < 1024; ++i)
                    nametableData[i] = 0;
            }
            setData(nametableData);
            mFileName = filename;
            mFileNameLabel->setText(getName());
            file.close();
            return true;
        } else {
            // Check size and import accordingly
        }
    }

    return false;
}

void NameTable::save(bool compress)
{
    // Save .nam file
    QFile file(mFileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write((const char*)mData, 1024);
        file.close();
    }
    // Save .h in rle compressed
    QFileInfo info(mFileName);
    QString name = info.baseName();
    QString hFilename = info.canonicalPath() + "/" + name + ".h";
    QFile headerFile(hFilename);

    unsigned char *dst;
    int stat[256];
    int i;
    int size = 1024;

    if (compress) {
        int min=256;
        int tag=255;

        dst = new unsigned char[size*2];
        for (i = 0; i < 256; ++i)
            stat[i] = 0;
        for (i=0; i < size; ++i)
            stat[mData[i]]++;

        for (i = 0; i < 256; ++i) {
            if (stat[i]<min) {
                min=stat[i];
                tag = i;
            }
        }

        int pp=0;
        dst[pp++] = tag;
        int len=0;
        int sym=-1;

        for (i=0; i < size; ++i) {
            if (mData[i]!= sym || len == 255 || i == size - 1) {
                if (mData[i]==sym && i == size - 1) len++;
                if (len)
                    dst[pp++]=sym;
                if (len > 1) {
                    if (len == 2) {
                        dst[pp++] = sym;
                    } else {
                        dst[pp++] = tag;
                        dst[pp++] = len - 1;
                    }
                }
                sym = mData[i];
                len = 1;
            } else {
                len++;
            }
        }
        dst[pp++] = tag;
        dst[pp++] = 0;
        size = pp;
    } else {
        dst = (unsigned char*)mData;
    }
    if (headerFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QString nameString = QString("const unsigned char %1[%2]={\n").arg(name).arg(size);
        headerFile.write(nameString.toStdString().c_str(), nameString.length());

        for (i = 0; i < size; ++i) {
            QString numberString = QString("0x%1").arg(dst[i], 2, 16, QChar('0'));
            if (i<size-1) numberString += ",";
            if ((i&15) == 15 || i == (size-1))
                numberString += "\n";
            headerFile.write(numberString.toStdString().c_str(), numberString.length());
        }
        QString endString("};\n");
        headerFile.write(endString.toStdString().c_str(), endString.length());
        headerFile.close();
    }
}

void NameTable::saveAs(QString filename, bool compress)
{
    setFilename(filename);
    save(compress);
}

void NameTable::setData(char *data)
{
    for (int i = 0; i < 1024; ++i) {
        mData[i] = (unsigned char)data[i];
    }

    paletteChanged();
    tilesChanged();
}

void NameTable::setPalette(Palette *pal)
{
    mPalette = pal;
    connect(pal, SIGNAL(currentPaletteChanged()), this, SLOT(paletteChanged()));
    paletteChanged();
}

void NameTable::setTile(int x, int y, int tile)
{
    qDebug() << "setTile called with x " << x
             << " and y " << y;
    int i = x + y*32;
    qDebug() << "i is " << i;
    if (i < 1024) {
        mData[i] = tile;
        char *tileData = mTileSet->tileData(mData[i]);
        if (tileData) {
            mTiles.at(i)->setData(tileData);
        }
    } else {
        qDebug() << "i was out of bounds of data of size 1024";
    }
}

void NameTable::setSelected(bool selected)
{
    mSelected = selected;
    if (selected)
        mNameRubberBand->show();
    else
        mNameRubberBand->hide();
    update();
}

void NameTable::setAttr(int x, int y, int pal)
{
    if (x < 0 || x > 31 || y < 0 || y > 29) {
        qDebug() << "got invalid coordinates " << x << y;
        return;
    }

    int pp = y/4*8+x/4;
    int mask = 3;
    pal = pal&3;

    QList<QList<QColor> > allColors = mPalette->getAllColors();
    QList<QColor> palette = allColors.at(pal);

    if (x&2) {
        pal  <<=2;
        mask <<=2;
    }

    if (y&2) {
        pal  <<=4;
        mask <<=4;
    }

    mAttrs[pp]=(mAttrs[pp] & (mask^255))|pal;

    int index = x + (y*32);
    int otherindex = (x%2 == 0) ? index + 1 : index - 1;

    mTiles.at(index)->setPalette(palette);
    mTiles.at(otherindex)->setPalette(palette);
    if (y%2 == 0) { // Y is even, so also set the palettes below
        mTiles.at(index + 32)->setPalette(palette);
        mTiles.at(otherindex + 32)->setPalette(palette);
    } else { // Y is odd, so also set palettes above
        mTiles.at(index - 32)->setPalette(palette);
        mTiles.at(otherindex - 32)->setPalette(palette);
    }

    update();
}

QString NameTable::getName() const
{
    QFileInfo info(mFileName);
    if (info.exists())
        return info.baseName();
    else
        return QString("New NameTable");
}

QString NameTable::getFileName() const
{
    return mFileName;
}

void NameTable::setFilename(const QString &filename)
{
    mFileName = filename;
    mFileNameLabel->setText(getName());
}

void NameTable::setScale(int scale)
{
    if (scale < 1 || scale > 6)
        return;

    Q_FOREACH(Tile *tile, mTiles) {
        tile->setFixedSize(QSize(8*scale, 8*scale));
    }
}

char *NameTable::getData() const
{
    return (char*)mData;
}

QSet<int> NameTable::usedTiles() const
{
    QSet<int> tiles;
    for (int i = 0; i < 960; ++i) {
        tiles.insert(mData[i]);
    }
    return tiles;
}

void NameTable::remapTiles(QMap<int, int> mapping)
{
    for (int i = 0; i < 960; ++i) {
        if (mapping.contains(mData[i])) {
            mData[i] = mapping.value(mData[i]);
        }
    }
}

void NameTable::tilesSwapped(int first, int second)
{
    qDebug() << "tiles " << first << " and " << second
             << "swapped" << " name is " << mFileName;
    for (int i = 0; i < 960; ++i) {
        if (mData[i] == first)
            mData[i] = second;
        else if (mData[i] == second)
            mData[i] = first;
    }
    update();
}

void NameTable::shiftTilesVertically(int count, bool down)
{
    if (down) {
        // Shifting down, so start at the bottom
        char data[32];
        for (int c = 0; c < 32; ++c) {
            data[c] = mData[29*32 + c];
        }

        for (int row = 29; row >= 1; --row) {
            for (int c = 0; c < 32; ++c) {
                // copy from the row above into this row
                int dest = row * 32 + c;
                int source = dest - 32;
                if (source < 0)
                    source += 960;
                mData[dest] = mData[source];
            }
        }

        // Then put temp data into top row
        for (int c = 0; c < 32; ++c) {
            mData[c] = data[c];
        }
    } else {
        // Shifting up, so start from the top
        char data[32];
        for (int c = 0; c < 32; ++c) {
            // Save top row of data
            data[c] = mData[c];
        }

        for (int row = 0; row < 30; ++row) {
            for (int c = 0; c < 32; ++c) {
                // copy from the row above into this row
                int dest = row * 32 + c;
                int source = dest + 32;
                if (source > 960)
                    source -= 960;
                mData[dest] = mData[source];
            }
        }

        // Then put temp data into top row
        for (int c = 0; c < 32; ++c) {
            mData[32*29 + c] = data[c];
        }
    }
    // Update tiles to reflect changes
    tilesChanged();
}

void NameTable::shiftTilesHorizontally(int count, bool right)
{

}

void NameTable::tileClicked()
{
    Tile *tile = qobject_cast<Tile*>(sender());
    int index = mTiles.indexOf(tile);
    if (index >= 0 && index < mTiles.size()) {
        emit tileClicked(index % 32, index / 32);
    }
}

void NameTable::paletteChanged()
{
    // Get all the colors
    QList<QList<QColor> > allColors = mPalette->getAllColors();
    int x = 0;
    int y = 0;
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(allColors.at(getAttr(x, y)));
        x++;
        if (x == 32) {
            x = 0;
            y++;
        }
    }

}

void NameTable::tilesChanged()
{
    if (mTileSet) { // Update tiles if we have a tileset
        for (int i = 0; i < 960; ++i) {
            char *tileData = mTileSet->tileData(mData[i]);
            if (tileData) {
                mTiles.at(i)->setData(tileData);
            }
        }
    }
}

void NameTable::toggleShowGrid(bool checked)
{
    mGridLayout->setHorizontalSpacing(checked ? 1 : 0);
    mGridLayout->setVerticalSpacing(checked ? 1 : 0);
    update();
}

int NameTable::getAttr(int x, int y)
{
    int pal;

    pal = mAttrs[y/4*8+x/4];
    if (x&2)
        pal>>=2;
    if (y&2)
        pal>>=4;

    return pal&3;
}

void NameTable::mousePressEvent(QMouseEvent *event)
{
    // Find the tile
    Tile *tile = qobject_cast<Tile*>(childAt(event->x(), event->y()));
    if (tile) {
        int index = mTiles.indexOf(tile);
        emit tileClicked(index %32, index / 32);
    }
}

void NameTable::mouseMoveEvent(QMouseEvent *event)
{
    // Find the tile
    if (event->buttons() & Qt::LeftButton) {
        Tile *tile = qobject_cast<Tile*>(childAt(event->x(), event->y()));
        if (tile) {
            int index = mTiles.indexOf(tile);
            emit tileClicked(index %32, index / 32);
        }
    }
}

void NameTable::tileHovered()
{
  Tile *tile = qobject_cast<Tile*>(sender());
  emit setStatus(tile->getHoverText());
}
