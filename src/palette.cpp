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

#include "palette.h"

#include <QHBoxLayout>

#include <math.h>

Palette::Palette(QWidget *parent)
    :QWidget(parent)
{
    mSwatches << new Swatch(this);
    mSwatches << new Swatch(this);
    mSwatches << new Swatch(this);
    mSwatches << new Swatch(this);

    mColors << Qt::black;
    mColors << Qt::black;
    mColors << Qt::black;
    mColors << Qt::black;
    mCurrentColor = 0;

    QHBoxLayout *layout = new QHBoxLayout(this);
    for (int i = 0; i < 4; ++i) {
        layout->addWidget(mSwatches.at(i));
    }
    setLayout(layout);
}

double Palette::colorRGBEuclideanDistance(const QColor &c1, const QColor &c2)
{
    return pow(c1.red()-c2.red(), 2) +
           pow(c1.green()-c2.green(), 2) +
           pow(c1.blue()-c2.blue(), 2);
}

int Palette::closestColor(const QColor &c1, QColor colors[4])
{
    int result = 0;
    double closest = 3*pow(255,2);
    for (int i = 0; i < 4; ++i) {
        double distance = colorRGBEuclideanDistance(c1, colors[i]);
        if (distance < closest) {
            result = i;
            closest = distance;
        }
    }
    return result;
}

QColor Palette::getColor(int which)
{
    if (which >= 0 && which < mColors.size())
        return mColors.at(which);

    return Qt::black;
}

int Palette::getCurrentColor() const
{
    return mCurrentColor;
}

void Palette::setCurrentColor(int which)
{
    mSwatches.at(mCurrentColor)->setSelected(false);
    mCurrentColor = which;
    mSwatches.at(which)->setSelected(true);
}
