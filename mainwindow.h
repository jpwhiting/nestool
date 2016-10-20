#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class Swatch;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private Q_SLOTS:
    void bgClicked();

    void paletteClicked();

    void on_action_Open_Palettes_triggered();
    void on_action_Save_Palettes_triggered();

    void on_action_Open_CHR_triggered();
    void on_action_Save_CHR_triggered();

    void on_bankAButton_toggled(bool set);
    void on_bankBButton_toggled(bool set);

    void updatePalettes();
    void updateTileset();

private:
  Ui::MainWindow *ui;
  QColor mBasePalette[64];
  unsigned char mBgPal[4][4]; // Palettes
  char mChr[8192]; // Tileset
  int mCurrentPalette; // Which palette to use
  unsigned char *mCurrentPal; // Which color is selected
  Swatch *mCurrentPalSwatch;
  QMap<QString, int> mColorIndexes; // Which color is which index in nes palette
};

#endif // MAINWINDOW_H
