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

#include "colorchooserdialog.h"
#include "ui_colorchooserdialog.h"

#include "palette.h"
#include "swatch.h"

ColorChooserDialog::ColorChooserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorChooserDialog)
{
    ui->setupUi(this);

    mColors = Palette::nesColors();

    Q_ASSERT(mColors.size() == 64);
    for (int i = 0; i < 64; ++i) {
        Swatch *swatch = QWidget::findChild<Swatch *>(QString("swatch%1").arg(i));
        if (swatch) {
            swatch->setColor(mColors.at(i));
            swatch->setHoverText(QString("Color:$%1").arg(i, 2, 16, QChar('0')));
            connect(swatch, SIGNAL(hovered()), this, SLOT(paletteHovered()));
            mColorIndexes.insert(mColors.at(i).name(), i);
            connect(swatch, SIGNAL(clicked()), this, SLOT(paletteClicked()));
            mSwatches.append(swatch);
        }
    }
    mCurrentIndex = 0;
    mCurrentSwatch = mSwatches.at(0);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

ColorChooserDialog::~ColorChooserDialog()
{
    delete ui;
}

void ColorChooserDialog::setCurrentIndex(int index)
{
    mCurrentIndex = index;
    mCurrentSwatch = mSwatches.at(index);
}

int ColorChooserDialog::chosenIndex() const
{
    return mCurrentIndex;
}

QColor ColorChooserDialog::chosenColor() const
{
    return mColors.at(mCurrentIndex);
}

void ColorChooserDialog::paletteClicked()
{
    Swatch *swatch = qobject_cast<Swatch*>(sender());
    QColor color = swatch->getColor();
    int index = mColorIndexes.value(color.name());
    if (index != mCurrentIndex) {
        mCurrentSwatch->setSelected(false);
        swatch->setSelected(true);
        mCurrentSwatch = swatch;
        mCurrentIndex = index;
    }
}
