#include "tile.h"

#include <QPainter>

Tile::Tile(QWidget *parent) : QWidget(parent)
{
    mSelected = false;
}

void Tile::setData(char *data)
{
    for (int i = 0; i < 16; ++i) {
        mData[i] = data[i];
    }
    update();
}

void Tile::setPalette(QList<QColor> colors)
{
    for (int i = 0; i < 4; ++i) {
        mPalette[i] = colors[i];
    }
    update();
}

bool Tile::getSelected() const
{
    return mSelected;
}

void Tile::setSelected(bool selected)
{
    mSelected = selected;
    update();
}

void Tile::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();

    if (mSelected) {
        painter.setPen(Qt::white);
        painter.drawRect(0, 0, width(), height());
    }

    painter.setPen(Qt::NoPen);

    int cellWidth = (width() - 1) / 8;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int col = (((mData[r]<<c)&128)>>7)|(((mData[r+8]<<c)&128)>>6);
            painter.setBrush(mPalette[col]);
            painter.drawRect(c*cellWidth, r*cellWidth, cellWidth, cellWidth);
        }
    }
    painter.restore();
}

void Tile::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
