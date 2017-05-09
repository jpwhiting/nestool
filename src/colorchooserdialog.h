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

#ifndef COLORCHOOSERDIALOG_H
#define COLORCHOOSERDIALOG_H

#include <QDialog>
#include <QMap>

class Swatch;

namespace Ui
{
class ColorChooserDialog;
}

class ColorChooserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ColorChooserDialog(QWidget *parent = 0);
    ~ColorChooserDialog();

    void setTitle(const QString &title);

    // Set the current color to the given index
    void setCurrentIndex(int index);

    int chosenIndex() const;
    QColor chosenColor() const;

private slots:
    void paletteClicked();
    void paletteHovered();

private:
    Ui::ColorChooserDialog *ui;
    int mCurrentIndex;
    Swatch *mCurrentSwatch;
    QList<Swatch*> mSwatches;
    QList<QColor> mColors;
};

#endif // COLORCHOOSERDIALOG_H
