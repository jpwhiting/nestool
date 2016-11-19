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

#ifndef EDITTILEDIALOG_H
#define EDITTILEDIALOG_H

#include <QDialog>

namespace Ui {
class EditTileDialog;
}

class EditTileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditTileDialog(QWidget *parent = 0);
    ~EditTileDialog();

    void setData(char *data);
    char *chrData(); // Get this tile's chr data

    void setPalette(QList<QColor> colors);

private Q_SLOTS:
    void on_hFlipToolButton_clicked();
    void on_vFlipToolButton_clicked();
    void on_cCWToolButton_clicked();
    void on_cWToolButton_clicked();
    void on_fillToolButton_clicked();
    void paletteClicked();

private:
    Ui::EditTileDialog *ui;
    int mCurrentColor;
};

#endif // EDITTILEDIALOG_H
