#include "swatch.h"

#include <QPainter>

Swatch::Swatch(QWidget *parent) : QWidget(parent)
{
    mColor = Qt::black;
    mSelected = false;
}

QColor Swatch::getColor() const
{
    return mColor;
}

void Swatch::setColor(QColor &color)
{
    mColor = color;
    update();
}

bool Swatch::getSelected() const
{
    return mSelected;
}

void Swatch::setSelected(bool selected)
{
    mSelected = selected;
    update();
}

void Swatch::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();

    painter.setPen(mSelected ? Qt::white : mColor);
    painter.setBrush(mColor);
    painter.drawRect(0, 0, width()-1, height()-1);
    painter.restore();
}

void Swatch::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
}
