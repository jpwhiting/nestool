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

#include "swatch.h"

#include <QMouseEvent>
#include <QPainter>

Swatch::Swatch(QWidget *parent) : QWidget(parent)
{
    mColor = Qt::black;
    mSelected = false;
    setMouseTracking(true);
}

QColor Swatch::getColor() const
{
    return mColor;
}

void Swatch::setColor(const QColor &color)
{
    mColor = color;
    update();
}

bool Swatch::getSelected() const
{
    return mSelected;
}

void Swatch::setSelected(bool selected)
{
    mSelected = selected;
    update();
}

QString Swatch::getHoverText() const
{
    return mHoverText;
}

void Swatch::setHoverText(QString text)
{
    mHoverText = text;
}

void Swatch::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();

    QPen pen(mSelected ? Qt::white : Qt::black, 1, mSelected ? Qt::DashLine : Qt::SolidLine);
    painter.setPen(pen);
    painter.setBrush(mColor);
    painter.drawRect(0, 0, width()-1, height()-1);
    painter.restore();
}

void Swatch::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}

void Swatch::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        emit clicked();
    emit hovered();
}
