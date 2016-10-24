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

#include "tile.h"

#include <QMouseEvent>
#include <QPainter>

Tile::Tile(QWidget *parent) : QWidget(parent)
{
    mSelected = false;
    for (int i = 0; i < 16; ++i) {
        mData[i] = 0;
    }
    setMouseTracking(true);
}

void Tile::setData(char *data)
{
    for (int i = 0; i < 16; ++i) {
        mData[i] = data[i];
    }
    update();
}

char *Tile::chrData()
{
    return mData;
}

void Tile::setPalette(QList<QColor> colors)
{
    for (int i = 0; i < 4; ++i) {
        mPalette[i] = colors[i];
    }
    update();
}

bool Tile::getSelected() const
{
    return mSelected;
}

void Tile::setSelected(bool selected)
{
    mSelected = selected;
    update();
}

QString Tile::getHoverText() const
{
    return mHoverText;
}

void Tile::setHoverText(QString text)
{
    mHoverText = text;
}

void Tile::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();

    if (mSelected) {
        painter.setPen(Qt::white);
        painter.drawRect(0, 0, width(), height());
    }

    painter.setPen(Qt::NoPen);

    int cellWidth = width() / 8;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int col = (((mData[r]<<c)&128)>>7)|(((mData[r+8]<<c)&128)>>6);
            painter.setBrush(mPalette[col]);
            painter.drawRect(c*cellWidth, r*cellWidth, cellWidth, cellWidth);
        }
    }
    painter.restore();
}

void Tile::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

void Tile::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton == Qt::LeftButton)
        emit clicked();
    emit hovered();
}
