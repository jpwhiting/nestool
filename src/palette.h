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

#ifndef PALETTE_H
#define PALETTE_H

#include "swatch.h"

#include <QColor>
#include <QWidget>

class QLabel;

class Palette: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentColor READ getCurrentColor WRITE setCurrentColor)
    Q_PROPERTY(int currentPalette READ getCurrentPalette WRITE setCurrentPalette NOTIFY currentPaletteChanged)
public:
    explicit Palette(QWidget *parent = 0);

    QColor getColor(int which);
    QList<QColor> getCurrentPaletteColors() const;
    QList<QList<QColor> > getAllColors() const;

    int getCurrentColor() const;
    void setCurrentColor(int which);

    int getCurrentPalette() const;
    void setCurrentPalette(int which);

    static QList<QColor> nesColors();

    bool load(const QString &filename);
    void save(); // Need filename loaded or call saveAs first
    void saveAs(QString filename);

    static double colorRGBEuclideanDistance(const QColor& c1, const QColor& c2);
    static int closestColor(const QColor &c1, QColor colors[4]);

signals:
    // Either the user clicked on a different palette or the palette colors changed
    void currentPaletteChanged();

private slots:
    void paletteClicked();

private:
    static void initializeBasePalette();
    void changeColor(int index);
    void updatePalettes();

    QList<Swatch*> mSwatches;
    QList<QColor> mColors;
    int mCurrentColor;
    QList<unsigned char> mPalData;
    QString mFileName;
    QLabel *mFilenameLabel;
    static QList<QColor> mBasePalette;
};

//ntsc palette captured.pal by Kinopio

const unsigned char ntscPalette[64*3]={
    124,124,124,
    0,0,252,
    0,0,188,
    68,40,188,
    148,0,132,
    168,0,32,
    168,16,0,
    136,20,0,
    80,48,0,
    0,120,0,
    0,104,0,
    0,88,0,
    0,64,88,
    0,0,0,
    0,0,0,
    0,0,0,
    188,188,188,
    0,120,248,
    0,88,248,
    104,68,252,
    216,0,204,
    228,0,88,
    248,56,0,
    228,92,16,
    172,124,0,
    0,184,0,
    0,168,0,
    0,168,68,
    0,136,136,
    0,0,0,
    0,0,0,
    0,0,0,
    248,248,248,
    60,188,252,
    104,136,252,
    152,120,248,
    248,120,248,
    248,88,152,
    248,120,88,
    252,160,68,
    248,184,0,
    184,248,24,
    88,216,84,
    88,248,152,
    0,232,216,
    120,120,120,
    0,0,0,
    0,0,0,
    252,252,252,
    164,228,252,
    184,184,248,
    216,184,248,
    248,184,248,
    248,164,192,
    240,208,176,
    252,224,168,
    248,216,120,
    216,248,120,
    184,248,184,
    184,248,216,
    0,252,252,
    248,216,248,
    0,0,0,
    0,0,0
};

#endif
