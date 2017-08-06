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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>

namespace Ui
{
class ImportDialog;
}

class QSettings;

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent = 0);
    ~ImportDialog();

    QString filename() const;
    QString nametableName() const;

    bool importColors() const;
    bool importTiles() const;

public slots:
    virtual void accept() Q_DECL_OVERRIDE;

private slots:
    void on_browseToolButton_clicked();

    void updateButtons();

private:
    Ui::ImportDialog *ui;

    bool acceptableInput();

    QSettings *mSettings;
};

#endif // IMPORTDIALOG_H
