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
QMap<QRgb, int> Palette::mClosestColorMap;

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
        connect(swatch, SIGNAL(hovered()), this, SLOT(swatchHovered()));
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
        setColor(index, dialog->chosenIndex());
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

QList<int> Palette::calculateFromImage(QImage *image)
{
    QList<int> attributes; // Which palette to use for each 16x16 block
    QList<QList<QColor> > newColors;
    // Find most common color
    ColorChooserDialog *colorDialog = new ColorChooserDialog(this);
    colorDialog->setTitle("Select the main color for this image");
    colorDialog->exec();
    QColor mainColor = colorDialog->chosenColor();
//    QMap<QRgb, int> colorUses;
//    int maxUses = 1;
//    QColor maxColor;
//    for (int y = 0; y < image->height(); ++y) {
//        for (int x = 0; x < image->width(); ++x) {
//            QColor pixelColor = image->pixelColor(x, y);
//            if (colorUses.contains(pixelColor.rgb())) {
//                int count = colorUses.value(pixelColor.rgb()) + 1;
//                if (count > maxUses) {
//                    maxUses = count;
//                    maxColor = pixelColor;
//                }
//                colorUses.insert(pixelColor.rgb(), count);
//            } else {
//                colorUses.insert(pixelColor.rgb(), 1);
//            }
//        }
//    }

    for (int i = 0; i < 4; ++i) {
        // Set mainColor as the first color of all 4 new palettes
        QList<QColor> newList;
        newList.append(mainColor);
        newColors.append(newList);
    }

    // Find palettes for each 16x16 block
    for (int y = 0; y < image->height(); y+=16) {
        for (int x = 0; x < image->width(); x+=16) {
            QList<QColor> sectionPalette = colorsFromImageSection(image, x, y);
            // Should have 4 or less colors
            for (int p = 0; p < 4; ++p) {
                int m = matches(sectionPalette, newColors[p]);
                if (equal(sectionPalette, newColors[p])) {
                    qDebug() << "sectionPalette is contained in newColors[" << p << "]";
                    attributes.append(p); // Palettes match (or sectionPalette is contained in newColors[p]
                    break;
                } else if (sectionPalette.size() - m <= 4 - newColors[p].size()) {
                    qDebug() << "*** section palette size fits in palette " << p
                             << " m is " << m
                             << "sectionPalette.size is " << sectionPalette.size()
                             << " newColors[p].size is " << newColors[p].size();
                    addNonMatches(sectionPalette, newColors[p]);
                    attributes.append(p);
                    break;
                } else if (p == 3) {
                    qDebug() << "couldn't find palette with colors ";
                    for (int c=0; c < sectionPalette.size(); ++c)
                        qDebug() << "color " << c << " is "
                                 << QString("Color:$%1").arg(closestNesColor(sectionPalette.at(c)), 2, 16, QChar('0'));
                    qDebug() << "for image at " << x << ", " << y;
                    attributes.append(0); // Use palette 0 if nothing else fits
                    break;
                }
            }
        }
    }

    qDebug() << "colors calculated are " << newColors.at(0).size()
             << ", " << newColors.at(1).size()
             << ", " << newColors.at(2).size()
             << ", " << newColors.at(3).size();

    // Set the colors from the new palettes
    for (int p = 0; p < 4; ++p) {
        for (int c = 0; c < 4; ++c) {
            // If we didn't fill the palette, use pink to fill in the rest
            if (c < newColors.at(p).size()) {
                int index = closestNesColor(newColors.at(p).at(c));
                setColor(c + p*4, index);
            } else {
                setColor(c + p*4, 0x25);
            }
        }
    }
    emit currentPaletteChanged();

    return attributes;
}

QList<int> Palette::getAttributesFromImage(QImage *image)
{
    QList<int> attributes; // Which palette to use for each 16x16 block

    // Find palettes for each 16x16 block
    for (int y = 0; y < image->height(); y+=16) {
        for (int x = 0; x < image->width(); x+=16) {
            QList<QColor> sectionPalette = colorsFromImageSection(image, x, y);
            // Should have 4 or less colors
            int pal = whichPalette(sectionPalette);
            if (pal == -1) {
                qDebug() << "found no matching palette for colors: "
                         << sectionPalette.at(0)
                         << " image at " << x << ", " << y;
                attributes.append(0);
            } else {
                attributes.append(pal);
            }
        }
    }

    return attributes;
}

QList<QColor> Palette::colorsFromImageSection(QImage *image, int x, int y)
{
    QList<QColor> colorList;
    for (int xo = 0; xo < 16 && x + xo < image->width(); ++xo) {
        for (int yo = 0; yo < 16 && y + yo < image->height(); ++yo) {
            QColor pixelColor = image->pixelColor(x+xo, y+yo);
            if (!colorList.contains(pixelColor)) {
                colorList.append(pixelColor);
            }
        }
    }
    // Normalize colors based on nes palette
    for (int i = 0; i < colorList.size(); ++i) {
        int nesColor = closestNesColor(colorList.at(i));
        qDebug() << "Closest nes color for " << colorList.at(i).name()
                 << " is " << mBasePalette.at(nesColor).name();
        colorList[i] = mBasePalette.at(nesColor);
    }
    return colorList;
}

bool Palette::equal(QList<QColor> &c1, QList<QColor> &c2)
{
    if (c1.size() > c2.size()) {
        return false;
    }

    bool result = true;

    //compare each color in palette 1 to each color in palette 2
    for (int i = 0; i < 4; i++) {
        if (c1.size() <= i)
            break;

        for (int j = 0; j < 4; j++) {
            //if color match found, set result to true and move to next color in palette 1
            if(c2[j] == c1[i]) {
                result = true;
                break;
            }
            //if no color match found and we are at the end of palette 2, then the palettes are not equal
            else if(j == c2.size() - 1) {
                result = false;
                break;
            }
        }
        if (!result)
            break;
    }

    return result;
}

int Palette::matches(QList<QColor> &c1, QList<QColor> &c2)
{
    int matches = 0;

    for(int i = 0; i < c1.size(); i++) {
        for(int j = 0; j < c2.size(); j++) {
            if (colorRGBEuclideanDistance(c1.at(i), c2.at(j)) < 100.0) {
                matches++;
                break;
            }
        }
    }

    return matches;
}

void Palette::addNonMatches(QList<QColor> &c1, QList<QColor> &c2)
{
    bool foundMatch = false;

    // Add colors from c1 to c2 if they are not there already (non matches)
    for(int i = 0; i < c1.size(); i++) {
        foundMatch = false;
        for(int j = 0; j < c2.size(); j++) {
            if (colorRGBEuclideanDistance(c1.at(i), c2.at(j)) < 100.0) {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
            c2.append(c1.at(i));
    }
}

int Palette::whichPalette(QList<QColor> &c)
{
    for (int i = 0; i < 4; ++i) {
        int ix = i*4;
        QList<QColor> c2;
        c2 << mColors.at(ix) << mColors.at(ix+1)
           << mColors.at(ix + 2) << mColors.at(ix + 3);
        int m = matches(c, c2);
        if (m == c.size())
            return i;
    }
    return -1;
}

void Palette::setColor(int index, int which)
{
    mPalData[index] = which;
    mColors[index] = mBasePalette[which];
    mSwatches.at(index)->setColor(mColors.at(index));
    mSwatches.at(index)->setHoverText(QString("Color:$%1 %2").arg(which, 2, 16, QChar('0')).arg(mColors[index].name()));
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

void Palette::swatchHovered()
{
    Swatch *swatch = qobject_cast<Swatch*>(sender());
    emit paletteHovered(swatch->getHoverText());
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

int Palette::closestNesColor(const QColor &c1)
{
    if (c1 == Qt::black) // Prefer 0x0F for black
        return 0x0F;
    if (mClosestColorMap.contains(c1.rgb())) {
        return mClosestColorMap.value(c1.rgb());
    }
    int result = 0;
    double closest = 3*pow(255,2);
    for (int i = 0; i < mBasePalette.size(); ++i) {
        double distance = colorRGBEuclideanDistance(c1, mBasePalette.at(i));
        if (distance < closest) {
            result = i;
            closest = distance;
        }
    }
    mClosestColorMap.insert(c1.rgb(), result);
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
