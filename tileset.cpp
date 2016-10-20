#include "tileset.h"

#include <QGridLayout>

TileSet::TileSet(QWidget *parent) : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            Tile *tile = new Tile(this);
            tile->setFixedSize(QSize(18, 18));
            mTiles.append(tile);
            layout->addWidget(tile, i, j);
        }
    }
}

void TileSet::setData(char *data)
{
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            int which = i*16+j;
            mTiles.at(which)->setData(data + (which * 16));
        }
    }
}

void TileSet::setPalette(QList<QColor> colors)
{
    Q_FOREACH(Tile *tile, mTiles) {
        tile->setPalette(colors);
    }
}

