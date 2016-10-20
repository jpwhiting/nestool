#ifndef SWATCH_H
#define SWATCH_H

#include <QColor>
#include <QWidget>

class Swatch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ getColor WRITE setColor)
    Q_PROPERTY(bool selected READ getSelected WRITE setSelected)
public:
    explicit Swatch(QWidget *parent = 0);

    QColor getColor() const;
    void setColor(QColor &color);

    bool getSelected() const;
    void setSelected(bool selected);
signals:
    void clicked();

public slots:
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    QColor mColor;
    bool mSelected;
};

#endif // SWATCH_H
