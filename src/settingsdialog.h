#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QFont>
#include <QColor>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>

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
    void refreshIcons();
    void loadCurrentValues();

    MainWindow *m_mainWindow;

    QComboBox *m_themeCombo;
    QButtonGroup *m_sizeModeGroup;

    QFont m_currentFont;
    QColor m_currentVerseColor;
    QColor m_currentRefColor;
    QSpinBox *m_opacitySpin;
    QSpinBox *m_maxWidthSpin;
    QSpinBox *m_maxHeightSpin;

    QLineEdit *m_pathEdit;
    QLabel *m_statusLabel;
    
    QCheckBox *m_startupCheck;
};

#endif // SETTINGSDIALOG_H
