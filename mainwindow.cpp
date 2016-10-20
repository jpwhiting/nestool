#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "palette.h"
#include "swatch.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  mBgPal{{15, 0, 16, 48}, {15, 1, 33, 49}, {15, 6, 22, 38}, {15, 9, 25, 41}},
  mCurrentPal(0),
  mCurrentPalSwatch(0)
{
  ui->setupUi(this);

  // Populate mBasePalette
  int pp = 0;
  for (int i = 0; i < 64; ++i) {
      mBasePalette[i] = QColor(ntscPalette[pp], ntscPalette[pp+1],ntscPalette[pp+2]);
      Swatch *swatch = ui->paletteGroupBox->findChild<Swatch *>(QString("swatch%1").arg(i));
      if (swatch) {
          QColor color(ntscPalette[pp], ntscPalette[pp+1], ntscPalette[pp+2]);
          swatch->setColor(color);
          mColorIndexes.insert(color.name(), i);
          connect(swatch, SIGNAL(clicked()), this, SLOT(paletteClicked()));
      }
      pp+=3;
  }

  updatePalettes();

  // Connect to palette swatch click signals
  for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
          Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
          connect(swatch, SIGNAL(clicked()), this, SLOT(bgClicked()));
      }
  }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bgClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
            if (swatch == from) {
                swatch->setSelected(true);
                mCurrentPal = &mBgPal[i][j];
                mCurrentPalette = i;
                mCurrentPalSwatch = swatch;
            } else {
                swatch->setSelected(false);
            }
        }
    }
}

void MainWindow::paletteClicked()
{
    Swatch *from = qobject_cast<Swatch*>(sender());
    QColor color = from->getColor();
    int index = mColorIndexes.value(color.name());
    if (mCurrentPal != 0) {
        *mCurrentPal = index;
        mCurrentPalSwatch->setColor(color);
    }
}

void MainWindow::on_action_Open_Palettes_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open palettes file", QDir::home().absolutePath(), "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.exists()) {
            QFileInfo info(file);
            if (info.size() == 16 && file.open(QIODevice::ReadOnly)) {
                char pal[16];
                file.read(pal, 16);
                file.close();
                for (int i = 0; i < 4; ++i) {
                    mBgPal[0][i]=pal[i];
                    mBgPal[1][i]=pal[i+4];
                    mBgPal[2][i]=pal[i+8];
                    mBgPal[3][i]=pal[i+12];
                }
                updatePalettes();
            } else {
                QMessageBox::information(this, "Unable to read palettes file", "Palette file should be 16 bytes long");
            }
        }
    }
}

void MainWindow::on_action_Save_Palettes_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save palettes file", QDir::home().absolutePath(), "NES Palettes (*.pal)");
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            char pal[16];
            for (int i = 0; i < 4; ++i) {
                pal[i]=mBgPal[0][i];
                pal[i+4]=mBgPal[1][i];
                pal[i+8]=mBgPal[2][i];
                pal[i+12]=mBgPal[3][i];
            }
            file.write(pal, 16);
            file.close();
        }
    }
}

void MainWindow::updatePalettes()
{
    // Populate default palettes
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            Swatch *swatch = ui->paletteGroupBox->findChild<Swatch*>(QString("swatch%1_%2").arg(i).arg(j));
            if (swatch) {
                QColor color(mBasePalette[mBgPal[i][j]]);
                swatch->setColor(color);
            }
        }
    }
}
