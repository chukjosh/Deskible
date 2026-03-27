#include "settingsdialog.h"
#include "mainwindow.h"
#include "startupmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QApplication>

SettingsDialog::SettingsDialog(MainWindow *parent)
    : QDialog(parent), m_mainWindow(parent)
{
    setWindowTitle(tr("Deskible Settings"));
    setMinimumWidth(450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Auto-Switching
    QGroupBox *autoGroup = new QGroupBox(tr("Auto-Switching"), this);
    QVBoxLayout *autoLayout = new QVBoxLayout(autoGroup);
    m_autoSwitchCheck = new QCheckBox(tr("Automatically switch verses"), this);
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalIcon = new QLabel(this);
    intervalIcon->setPixmap(QIcon(":/icons/icons/interval.svg").pixmap(16, 16));
    intervalLayout->addWidget(intervalIcon);
    intervalLayout->addWidget(new QLabel(tr("Interval (seconds):"), this));
    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(5, 86400);
    intervalLayout->addWidget(m_intervalSpin);
    autoLayout->addWidget(m_autoSwitchCheck);
    autoLayout->addLayout(intervalLayout);
    mainLayout->addWidget(autoGroup);

    // Appearance
    QGroupBox *appearanceGroup = new QGroupBox(tr("Appearance"), this);
    QVBoxLayout *appLayout = new QVBoxLayout(appearanceGroup);
    
    QPushButton *fontBtn = new QPushButton(QIcon(":/icons/icons/font.svg"), tr("Choose Font..."), this);
    connect(fontBtn, &QPushButton::clicked, this, &SettingsDialog::onPickFont);
    appLayout->addWidget(fontBtn);

    QHBoxLayout *colorLayout = new QHBoxLayout();
    QPushButton *vColorBtn = new QPushButton(QIcon(":/icons/icons/color.svg"), tr("Verse Color..."), this);
    connect(vColorBtn, &QPushButton::clicked, this, &SettingsDialog::onPickVerseColor);
    QPushButton *rColorBtn = new QPushButton(QIcon(":/icons/icons/color.svg"), tr("Ref Color..."), this);
    connect(rColorBtn, &QPushButton::clicked, this, &SettingsDialog::onPickRefColor);
    colorLayout->addWidget(vColorBtn);
    colorLayout->addWidget(rColorBtn);
    appLayout->addLayout(colorLayout);

    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityIcon = new QLabel(this);
    opacityIcon->setPixmap(QIcon(":/icons/icons/opacity.svg").pixmap(16, 16));
    opacityLayout->addWidget(opacityIcon);
    opacityLayout->addWidget(new QLabel(tr("Opacity (%):"), this));
    m_opacitySpin = new QSpinBox(this);
    m_opacitySpin->setRange(20, 100);
    opacityLayout->addWidget(m_opacitySpin);
    appLayout->addLayout(opacityLayout);

    QHBoxLayout *widthLayout = new QHBoxLayout();
    QLabel *widthIcon = new QLabel(this);
    widthIcon->setPixmap(QIcon(":/icons/icons/width.svg").pixmap(16, 16));
    widthLayout->addWidget(widthIcon);
    widthLayout->addWidget(new QLabel(tr("Max Width (px):"), this));
    m_maxWidthSpin = new QSpinBox(this);
    m_maxWidthSpin->setRange(200, 800);
    widthLayout->addWidget(m_maxWidthSpin);
    appLayout->addLayout(widthLayout);

    mainLayout->addWidget(appearanceGroup);

    // Bible File
    QGroupBox *fileGroup = new QGroupBox(tr("Bible File"), this);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
    fileLayout->addWidget(new QLabel(tr("King James Version (KJV)"), this));
    
    QHBoxLayout *browseLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setReadOnly(true);
    QPushButton *browseBtn = new QPushButton(QIcon(":/icons/icons/path.svg"), tr("Browse..."), this);
    connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseClicked);
    browseLayout->addWidget(m_pathEdit);
    browseLayout->addWidget(browseBtn);
    fileLayout->addLayout(browseLayout);
    
    m_statusLabel = new QLabel(this);
    fileLayout->addWidget(m_statusLabel);
    fileLayout->addWidget(new QLabel(tr("Place additional Bible versions as .txt files in bibleversions/"), this));
    
    mainLayout->addWidget(fileGroup);

    // Startup
    QGroupBox *startupGroup = new QGroupBox(tr("Startup"), this);
    QVBoxLayout *startupLayout = new QVBoxLayout(startupGroup);
    m_startupCheck = new QCheckBox(tr("Launch Deskible at system startup"), this);
    startupLayout->addWidget(m_startupCheck);
    mainLayout->addWidget(startupGroup);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton(QIcon(":/icons/icons/save.svg"), tr("OK"), this);
    QPushButton *cancelBtn = new QPushButton(QIcon(":/icons/icons/cancel.svg"), tr("Cancel"), this);
    connect(okBtn, &QPushButton::clicked, this, &SettingsDialog::onAccepted);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    loadCurrentValues();
    applyStyle();
}

void SettingsDialog::loadCurrentValues()
{
    m_autoSwitchCheck->setChecked(m_mainWindow->autoSwitch());
    m_intervalSpin->setValue(m_mainWindow->switchInterval());
    m_currentFont = m_mainWindow->verseFont();
    m_currentVerseColor = m_mainWindow->verseColor();
    m_currentRefColor = m_mainWindow->refColor();
    m_opacitySpin->setValue(static_cast<int>(m_mainWindow->opacityValue() * 100));
    m_maxWidthSpin->setValue(m_mainWindow->maxWidthValue());
    m_pathEdit->setText(m_mainWindow->biblePath());
    m_startupCheck->setChecked(StartupManager::isStartupEnabled());
}

void SettingsDialog::onBrowseClicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Bible Text File"), 
                                                m_pathEdit->text(), tr("Text Files (*.txt)"));
    if (!path.isEmpty()) {
        m_pathEdit->setText(path);
        if (m_mainWindow->reader()->openFile(path)) {
            m_statusLabel->setText(tr("✓ Loaded 31102 verses"));
            m_statusLabel->setStyleSheet("color: #4CAF50;");
        } else {
            m_statusLabel->setText(tr("⚠ Failed to load file"));
            m_statusLabel->setStyleSheet("color: #F44336;");
        }
    }
}

void SettingsDialog::onPickFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_currentFont, this);
    if (ok) m_currentFont = font;
}

void SettingsDialog::onPickVerseColor()
{
    QColor color = QColorDialog::getColor(m_currentVerseColor, this, tr("Pick Verse Color"), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) m_currentVerseColor = color;
}

void SettingsDialog::onPickRefColor()
{
    QColor color = QColorDialog::getColor(m_currentRefColor, this, tr("Pick Reference Color"), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) m_currentRefColor = color;
}

void SettingsDialog::onAccepted()
{
    m_mainWindow->setAutoSwitch(m_autoSwitchCheck->isChecked());
    m_mainWindow->setSwitchInterval(m_intervalSpin->value());
    m_mainWindow->setVerseFont(m_currentFont);
    m_mainWindow->setVerseColor(m_currentVerseColor);
    m_mainWindow->setRefColor(m_currentRefColor);
    m_mainWindow->setOpacity(m_opacitySpin->value() / 100.0);
    m_mainWindow->setMaxWidth(m_maxWidthSpin->value());
    m_mainWindow->setBiblePath(m_pathEdit->text());
    
    StartupManager::setStartupEnabled(m_startupCheck->isChecked());

    m_mainWindow->saveSettings();
    m_mainWindow->applySettings();
    accept();
}

void SettingsDialog::applyStyle()
{
    setStyleSheet(
        "QDialog { background-color: #12122a; color: #e8e8f8; }"
        "QGroupBox { border: 1px solid #333355; border-radius: 8px; margin-top: 10px; padding-top: 10px; color: #e8e8f8; font-weight: bold; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; }"
        "QLabel { color: #e8e8f8; }"
        "QPushButton { background-color: #2a2a5e; color: white; border-radius: 4px; padding: 6px 12px; border: 1px solid #333355; }"
        "QPushButton:hover { background-color: #3a3a7e; }"
        "QLineEdit { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 4px; padding: 4px; }"
        "QSpinBox { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 4px; padding: 4px; }"
        "QCheckBox { color: #e8e8f8; }"
    );
}
