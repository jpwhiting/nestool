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

#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>

#include "colorchooserdialog.h"

#include <math.h>

#include <QDebug>

QList<QColor> Palette::mBasePalette;

Palette::Palette(QWidget *parent)
    :QWidget(parent)
{
    if (mBasePalette.size() == 0)
        initializeBasePalette();

    // Load defaults
    mPalData << 15 << 0 << 16 << 48
             << 15 << 1 << 33 << 49
             << 15 << 6 << 22 << 38
             << 15 << 9 << 25 << 41;
    QHBoxLayout *layout = new QHBoxLayout(this);
    for (int i = 0; i < 16; ++i) {
        Swatch *swatch = new Swatch(this);
        mSwatches << swatch;
        mColors << Qt::black;
        layout->addWidget(mSwatches.at(i));
        connect(swatch, SIGNAL(clicked()), this, SLOT(paletteClicked()));
    }
    layout->setSpacing(2);

    mFilenameLabel = new QLabel(this);
    layout->addWidget(mFilenameLabel);
    setLayout(layout);

    mCurrentColor = 0;
    mSwatches.at(0)->setSelected(true);
    updatePalettes();
}

void Palette::changeColor(int index)
{
    ColorChooserDialog *dialog = new ColorChooserDialog(this);
    dialog->setCurrentIndex(mPalData.at(index));
    if (dialog->exec() == QDialog::Accepted) {
        mPalData[index] = dialog->chosenIndex();
        mSwatches.at(index)->setColor(dialog->chosenColor());
        mColors[index] = dialog->chosenColor();
        // Change the swatch to the new color
        emit currentPaletteChanged();
    }
}

void Palette::updatePalettes()
{
    for (int i = 0; i < 16; ++i) {
        QColor color = mBasePalette.at(mPalData.at(i));
        mSwatches.at(i)->setColor(color);
        mColors[i] = color;
    }
}

QColor Palette::getColor(int which)
{
    if (which >= 0 && which < mColors.size())
        return mColors.at(which);

    return Qt::black;
}

QList<QColor> Palette::getCurrentPaletteColors() const
{
    QList<QColor> list;
    int start = 0;
    if (mCurrentColor < 4) {
        start = 0;
    } else if (mCurrentColor >=4 && mCurrentColor < 8) {
        start = 4;
    } else if (mCurrentColor >=8 && mCurrentColor < 12) {
        start = 8;
    } else {
        start = 12;
    }
    for (int i = start; i < start+4; ++i)
        list << mColors.at(i);

    return list;
}

QList<QList<QColor> > Palette::getAllColors() const
{
    QList<QList<QColor> >lists;
    for (int i = 0; i < 4; ++i) {
        QList<QColor> l;
        for (int c = 0; c < 4; ++c) {
            l << mColors.at(i*4+c);
        }
        lists.append(l);
    }
    return lists;
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

int Palette::getCurrentPalette() const
{
    return mCurrentColor /4;
}

void Palette::setCurrentPalette(int which)
{
    if (which *4 != mCurrentColor) {
        mCurrentColor = which * 4;
        emit currentPaletteChanged();
    }
}

QList<QColor> Palette::nesColors()
{
    if (mBasePalette.size() == 0)
        initializeBasePalette();
    return mBasePalette;
}

bool Palette::load(const QString &filename)
{
    QFile file(filename);
    QFileInfo info(file);
    if (info.size() == 16 && file.open(QIODevice::ReadOnly)) {
        char pal[16];
        file.read(pal, 16);
        file.close();
        for (int i = 0; i < 4; ++i) {
            mPalData[i]= pal[i];
            mPalData[i+4]=pal[i+4];
            mPalData[i+8]=pal[i+8];
            mPalData[i+12]=pal[i+12];
        }
        mFileName = filename;
        mFilenameLabel->setText(info.baseName());
        updatePalettes();
        emit currentPaletteChanged();
        return true;
    }

    QMessageBox::information(this, "Unable to read palettes file", "Palette file should be 16 bytes long");
    return false;
}

void Palette::save()
{
    QFile file(mFileName);
    QFileInfo info(mFileName);
    if (file.open(QIODevice::WriteOnly)) {
        char pal[16];
        for (int i = 0; i < 4; ++i) {
            pal[i]=mPalData[i];
            pal[i+4]=mPalData[i+4];
            pal[i+8]=mPalData[i+8];
            pal[i+12]=mPalData[i+12];
        }
        file.write(pal, 16);
        file.close();

        // Write to .h file also
        QFile headerFile(info.absolutePath() + "/" + info.baseName() + ".h");
        if (headerFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QString name = info.baseName();
            QString nameString = QString("const unsigned char %1[16]={\n").arg(name);
            headerFile.write(nameString.toStdString().c_str(), nameString.length());

            for (int i = 0; i < 16; ++i) {
                QString numberString = QString("0x%1").arg(pal[i], 2, 16, QChar('0'));
                if (i<15)
                    numberString += ",";
                if ((i&3) == 3 || i == 15)
                    numberString += "\n";
                headerFile.write(numberString.toStdString().c_str(), numberString.length());
            }
            QString endString("};\n");
            headerFile.write(endString.toStdString().c_str(), endString.length());
            headerFile.close();
        }
    }
}

void Palette::saveAs(QString filename)
{
    QFileInfo info(filename);
    mFileName = filename;
    mFilenameLabel->setText(info.baseName());
    save();
}

void Palette::paletteClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    QColor color = from->getColor();
    int index = mSwatches.indexOf(from);
    if (index != mCurrentColor) {
        mSwatches.at(mCurrentColor)->setSelected(false);
        mCurrentColor = index;
        mSwatches.at(mCurrentColor)->setSelected(true);
        update();
        emit currentPaletteChanged();
    } else {
        // Swatch is already selected, so open color picker
        changeColor(index);
    }
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

void Palette::initializeBasePalette()
{
    // Populate mBasePalette
    int pp = 0;
    for (int i = 0; i < 64; ++i) {
        mBasePalette << QColor(ntscPalette[pp], ntscPalette[pp+1],ntscPalette[pp+2]);
        pp+=3;
    }
}
