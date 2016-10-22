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

#include <QDebug>

NameTable::NameTable(QWidget *parent) : QWidget(parent), mTileSet(0)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    mAttrs = &mData[960];

    for (int i = 0; i < 30; ++i) {
        for (int j = 0; j < 32; ++j) {
            Tile *tile = new Tile(this);
            connect(tile, SIGNAL(clicked()), this, SLOT(tileClicked()));
            tile->setFixedSize(QSize(24, 24));
            mTiles.append(tile);
            layout->addWidget(tile, i, j);
        }
    }

    for (int i = 0; i < 1024; ++i) {
        mData[i] = 0;
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

    int x = 0;
    int y = 0;
    if (mTileSet) { // Update tiles if we have a tileset
        for (int i = 0; i < 960; ++i) {
            char *tileData = mTileSet->tileData(mData[i]);
            if (tileData) {
                mTiles.at(i)->setData(tileData);
                mTiles.at(i)->setPalette(mPalettes.at(getAttr(x, y)));
                x++;
                if (x == 32) {
                    x = 0;
                    y++;
                }
            }
        }
    }
}

void NameTable::setPalettes(QList<QList<QColor> > colors)
{
    mPalettes = colors;
    int x = 0;
    int y = 0;
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(mPalettes.at(getAttr(x, y)));
        x++;
        if (x == 32) {
            x = 0;
            y++;
        }
    }
}

void NameTable::setAttr(int x, int y, int pal)
{
    if (x < 0 || x > 31 || y < 0 || y > 29) {
        qDebug() << "got invalid coordinates " << x << y;
        return;
    }

    int pp = y/4*8+x/4;
    qDebug() << "pp is " << pp;
    int mask = 3;
    pal = pal&3;

    int which = pal;

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
    qDebug() << "which is " << which << " size is " << mPalettes.size();
    mTiles.at(index)->setPalette(mPalettes.at(which));

    update();
}

char *NameTable::getData() const
{
    return (char*)mData;
}

void NameTable::tileClicked()
{
    Tile *tile = qobject_cast<Tile*>(sender());
    int index = mTiles.indexOf(tile);
    if (index >= 0 && index < mTiles.size()) {
        qDebug() << "tile clicked " << index % 32 << " " << index / 32;
        emit tileClicked(index % 32, index / 32);
    }
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

