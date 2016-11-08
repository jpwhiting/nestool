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
    Q_PROPERTY(bool editable READ getEditable WRITE setEditable)
    Q_PROPERTY(bool showGrid READ getShowGrid WRITE setShowGrid)
public:
    explicit Tile(QWidget *parent = 0);

    void setData(char *data);
    char *chrData(); // Get this tile's chr data

    void setPalette(QList<QColor> colors);
    void setCurrentColor(int color);

    bool getSelected() const;
    void setSelected(bool selected);

    QString getHoverText() const;
    void setHoverText(QString text);

    bool getEditable() const;
    void setEditable(bool editable);

    bool getShowGrid() const;
    void setShowGrid(bool show);

    bool identical(Tile *other); // Tell if this tile is identical to other
signals:
    void clicked();
    void hovered();

public slots:
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    // set the given pixel at x and y to color
    void setPixel(int x, int y, int color);

    bool mSelected;
    char mData[16];
    QColor mPalette[4];
    QString mHoverText;
    bool mEditable;
    bool mShowGrid;
    int mCurrentColor;
};

#endif // TILE_H
