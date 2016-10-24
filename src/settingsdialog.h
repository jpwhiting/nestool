#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class QSettings;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    bool compressNameTables() const;
    int maxRecentFiles() const;
    int tileSetScale() const;
    int nameTableScale() const;

signals:
    void settingsChanged(); // Signal that some settings have changed

private slots:
    void on_rleRadioButton_toggled();
    void on_recentFilesSpinBox_valueChanged();
    void on_tileSetScaleSpinBox_valueChanged();
    void on_nameTableScaleSpinBox_valueChanged();


private:
    Ui::SettingsDialog *ui;
    QSettings *mSettings;
};

#endif // SETTINGSDIALOG_H
