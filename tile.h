#ifndef TILE_H
#define TILE_H

#include <QColor>
#include <QWidget>

class Tile : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool selected READ getSelected WRITE setSelected)
public:
    explicit Tile(QWidget *parent = 0);

    void setData(char *data);

    void setPalette(QList<QColor> colors);

    bool getSelected() const;
    void setSelected(bool selected);
signals:
    void clicked();

public slots:
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    bool mSelected;
    char mData[16];
    QColor mPalette[4];
};

#endif // TILE_H
