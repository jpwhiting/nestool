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

#ifndef TILE_H
#define TILE_H

#include <QColor>
#include <QWidget>

class Tile : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selected READ getSelected WRITE setSelected)
    Q_PROPERTY(QString hoverText READ getHoverText WRITE setHoverText)
public:
    explicit Tile(QWidget *parent = 0);

    void setData(char *data);
    char *chrData(); // Get this tile's chr data

    void setPalette(QList<QColor> colors);

    bool getSelected() const;
    void setSelected(bool selected);

    QString getHoverText() const;
    void setHoverText(QString text);
signals:
    void clicked();
    void hovered();

public slots:
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    bool mSelected;
    char mData[16];
    QColor mPalette[4];
    QString mHoverText;
};

#endif // TILE_H
