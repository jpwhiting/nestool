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
#include "palette.h"

#include <QMouseEvent>
#include <QPainter>

#include <QDebug>

Tile::Tile(QWidget *parent) : QWidget(parent)
{
    mSelected = false;
    mEditable = false;
    mShowGrid = false;
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

void Tile::setCurrentColor(int color)
{
    mCurrentColor = color;
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

bool Tile::getEditable() const
{
    return mEditable;
}

void Tile::setEditable(bool editable)
{
    mEditable = editable;
    update();
}

bool Tile::getShowGrid() const
{
    return mShowGrid;
}

void Tile::setShowGrid(bool show)
{
    mShowGrid = show;
    update();
}

void Tile::hFlip()
{
    for (int i = 0; i < 16; ++i) {
        char d = mData[i];
        d = (d & 0xF0) >> 4 | (d & 0x0F) << 4;
        d = (d & 0xCC) >> 2 | (d & 0x33) << 2;
        d = (d & 0xAA) >> 1 | (d & 0x55) << 1;
        mData[i] = d;
    }
    update();
}

void Tile::vFlip()
{
    for (int i = 0; i < 4; ++i) {
        qDebug() << "swapping bytes " << i << " and " << 7-i;
        qDebug() << "swapping bytes " << i+8 << " and " << (7-i) + 8;
        char t = mData[i];
        mData[i] = mData[7-i];
        mData[7-i] = t;
        t = mData[i+8];
        mData[i+8] = mData[(7-i)+8];
        mData[(7-i)+8] = t;
    }
    update();
}

void Tile::rotateCounterClockwise()
{
    char result[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            result[i] = ((mData[j] & (1 << i)) >> i) << (7-j) | result[i];
            result[i+8] = ((mData[j+8] & (1 << i)) >> i) << (7-j) | result[i+8];
        }
    }
    for (int i = 0; i < 16; ++i) {
        mData[i] = result[i];
    }
    update();
}

void Tile::rotateClockwise()
{
    char result[16] = {0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            result[i] = (((mData[j] & (1 << (7-i))) >> (7-i)) << j) | result[i];
            result[i+8] = (((mData[j+8] & (1 << (7-i))) >> (7-i)) << j) | result[i+8];
        }
    }
    for (int i = 0; i < 16; ++i) {
        mData[i] = result[i];
    }
    update();
}

void Tile::fill()
{
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            setPixel(i, j, mCurrentColor);
        }
    }
    update();
}

bool Tile::identical(Tile *other)
{
    bool same = true;
    for (int i = 0; i < 16; ++i) {
        if (mData[i] != other->mData[i]) {
            same = false;
            i = 16;
        }
    }
    return same;
}

QImage Tile::image() const
{
    QImage pic(8, 8, QImage::Format_Indexed8);
    for (int i = 0; i < 4; ++i) {
        pic.setColor(i, mPalette[i].rgb());
    }
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            pic.setPixel(x, y, getPixel(x, y));
        }
    }
    return pic;
}

void Tile::setImage(QImage image, int x, int y)
{
    for (int r = 0; r < 8; ++r) { // Rows
        for (int c = 0; c < 8; ++c) { // Columns
            QColor color = image.pixelColor(x + c, y + r);
            setPixel(c, r, Palette::closestColor(color, mPalette));
        }
    }
    update();
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
            int col = getPixel(c, r);
            painter.setBrush(mPalette[col]);
            painter.drawRect(c*cellWidth, r*cellWidth, cellWidth, cellWidth);
        }
        if (mShowGrid) {
            painter.setPen(Qt::white);
            painter.drawLine(0, r*cellWidth, width(), r*cellWidth);
        }
    }
    if (mSelected) {
        painter.setPen(Qt::white);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(0, 0, width() - 1, height() - 1);
    }

    painter.restore();
}

void Tile::mousePressEvent(QMouseEvent *event)
{
    if (mEditable) {
        // Find which pixel was clicked and change it to current color
        int cellWidth = width() / 8;
        int x = event->x() / cellWidth;
        int y = event->y() / cellWidth;
        setPixel(x, y, mCurrentColor);
        update();
    }
    emit clicked();
    event->ignore();
}

void Tile::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        emit clicked();
    emit hovered();
    event->ignore();
}

void Tile::setPixel(int x, int y, int color)
{
    // Check for valid values
    if (x < 0 || x > 7 || y < 0 || y > 7 || color < 0 || color > 3)
        return;

    int pp = y;
    int mask = 1;
    int palLow = color&1;
    int palHigh = color&2;
    palHigh >>=1;

    palLow <<= (7-x);
    palHigh <<= (7-x);
    mask <<=(7-x);

    mData[pp]=(mData[pp] & (mask^255))|palLow;
    mData[pp+8]=(mData[pp+8] & (mask^255))|palHigh;
}

int Tile::getPixel(int x, int y) const
{
    return (((mData[y]<<x)&128)>>7)|(((mData[y+8]<<x)&128)>>6);
}
