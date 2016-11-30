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

#include "edittiledialog.h"
#include "ui_edittiledialog.h"

EditTileDialog::EditTileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTileDialog)
{
    ui->setupUi(this);

    ui->tileWidget->setFixedSize(QSize(160, 160));
    ui->tileWidget->setEditable(true);
    ui->swatch0->setSelected(true);
    connect(ui->swatch0, SIGNAL(clicked()), this, SLOT(paletteClicked()));
    connect(ui->swatch1, SIGNAL(clicked()), this, SLOT(paletteClicked()));
    connect(ui->swatch2, SIGNAL(clicked()), this, SLOT(paletteClicked()));
    connect(ui->swatch3, SIGNAL(clicked()), this, SLOT(paletteClicked()));
    mCurrentColor = 0;
}

EditTileDialog::~EditTileDialog()
{
    delete ui;
}

void EditTileDialog::setData(char *data)
{
    ui->tileWidget->setData(data);
}

char *EditTileDialog::chrData()
{
    return ui->tileWidget->chrData();
}

void EditTileDialog::setPalette(QList<QColor> colors)
{
    ui->tileWidget->setPalette(colors);
    ui->swatch0->setColor(colors.at(0));
    ui->swatch1->setColor(colors.at(1));
    ui->swatch2->setColor(colors.at(2));
    ui->swatch3->setColor(colors.at(3));
}

void EditTileDialog::on_hFlipToolButton_clicked()
{
    ui->tileWidget->hFlip();
}

void EditTileDialog::on_vFlipToolButton_clicked()
{
    ui->tileWidget->vFlip();
}

void EditTileDialog::on_cCWToolButton_clicked()
{
    ui->tileWidget->rotateCounterClockwise();
}

void EditTileDialog::on_cWToolButton_clicked()
{
    ui->tileWidget->rotateClockwise();
}

void EditTileDialog::on_fillToolButton_clicked()
{
    ui->tileWidget->fill();
}

void EditTileDialog::paletteClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    for (int i = 0; i < 4; ++i) {
        Swatch *swatch = this->findChild<Swatch *>(QString("swatch%1").arg(i));
        if (swatch) {
            if (swatch == from) {
                mCurrentColor = i;
                from->setSelected(true);
            } else
                swatch->setSelected(false);
        }
    }
    ui->tileWidget->setCurrentColor(mCurrentColor);
}

