#ifndef TILESET_H
#define TILESET_H

#include "tile.h"

#include <QList>
#include <QWidget>

class TileSet : public QWidget
{
    Q_OBJECT
public:
    explicit TileSet(QWidget *parent = 0);

    void setData(char *data);

    void setPalette(QList<QColor> colors);

public slots:
private:
    QList<Tile*> mTiles;
};

#endif // TILE_H
