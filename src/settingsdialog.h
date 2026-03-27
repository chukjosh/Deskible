#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QFont>
#include <QColor>

class MainWindow;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(MainWindow *parent);

private slots:
    void onBrowseClicked();
    void onPickFont();
    void onPickVerseColor();
    void onPickRefColor();
    void onAccepted();

private:
    void applyStyle();
    void loadCurrentValues();

    MainWindow *m_mainWindow;

    QCheckBox *m_autoSwitchCheck;
    QSpinBox *m_intervalSpin;
    
    QFont m_currentFont;
    QColor m_currentVerseColor;
    QColor m_currentRefColor;
    QSpinBox *m_opacitySpin;
    QSpinBox *m_maxWidthSpin;

    QLineEdit *m_pathEdit;
    QLabel *m_statusLabel;
    
    QCheckBox *m_startupCheck;
};

#endif // SETTINGSDIALOG_H
