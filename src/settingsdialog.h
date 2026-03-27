#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QFont>
#include <QColor>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QComboBox>
#include "common.h"

class MainWindow;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(MainWindow *parent);

private slots:
    void onAccepted();
    void onBrowseClicked();
    void onPickFont();
    void onPickVerseColor();
    void onPickRefColor();
    void onThemeChanged(int index);
    void reject() override;

private:
    void applyStyle();
    void refreshIcons();
    void loadCurrentValues();

    MainWindow *m_mainWindow;
    QScrollArea *m_scrollArea;
    Theme m_originalTheme;

    QComboBox *m_themeCombo;
    QButtonGroup *m_sizeModeGroup;

    QCheckBox *m_autoSwitchCheck;
    QSpinBox *m_intervalSpin;

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
