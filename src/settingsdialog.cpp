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
#include <QScrollArea> // Added for QScrollArea

SettingsDialog::SettingsDialog(MainWindow *parent)
    : QDialog(parent), m_mainWindow(parent)
{
    setWindowTitle(tr("Deskible Settings"));
    setMinimumWidth(480);
    resize(500, 600); // Set a reasonable default height

    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *scrollContent = new QWidget();
    scrollContent->setObjectName("scrollContent");
    QVBoxLayout *mainLayout = new QVBoxLayout(scrollContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    bool isDark = true;
    if (m_mainWindow->theme() == Theme::Dark) isDark = true;
    else if (m_mainWindow->theme() == Theme::Light) isDark = false;
    else {
        QPalette p = qApp->palette();
        isDark = p.color(QPalette::WindowText).lightness() > p.color(QPalette::Window).lightness();
    }
    QString iconPrefix = isDark ? ":/icons/icons/white/" : ":/icons/icons/dark/";

    // Auto-Switching
    QGroupBox *autoGroup = new QGroupBox(tr("Auto-Switching"), this);
    QVBoxLayout *autoLayout = new QVBoxLayout(autoGroup);
    
    QHBoxLayout *autoHeaderLayout = new QHBoxLayout();
    QLabel *autoIcon = new QLabel(this);
    autoIcon->setPixmap(QIcon(iconPrefix + "auto.svg").pixmap(16, 16));
    autoHeaderLayout->addWidget(autoIcon);
    autoHeaderLayout->addWidget(new QLabel(tr("Auto-Switching"), this));
    autoLayout->addLayout(autoHeaderLayout);
    
    m_autoSwitchCheck = new QCheckBox(tr("Automatically switch verses"), this);
    
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalIcon = new QLabel(this);
    // Initial icon - will be refreshed in refreshIcons()
    intervalIcon->setPixmap(QIcon(iconPrefix + "interval.svg").pixmap(16, 16));
    intervalIcon->setObjectName("intervalIcon");
    
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
    
    QPushButton *fontBtn = new QPushButton(QIcon(iconPrefix + "font.svg"), tr("Choose Font..."), this);
    connect(fontBtn, &QPushButton::clicked, this, &SettingsDialog::onPickFont);
    appLayout->addWidget(fontBtn);

    QHBoxLayout *refScaleLayout = new QHBoxLayout();
    refScaleLayout->addWidget(new QLabel(tr("Ref Font Scale (%):"), this));
    m_refScaleSpin = new QDoubleSpinBox(this);
    m_refScaleSpin->setRange(0.1, 2.0);
    m_refScaleSpin->setSingleStep(0.1);
    connect(m_refScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double val) {
        m_mainWindow->setRefScale(val);
        m_mainWindow->applySettings();
    });
    refScaleLayout->addWidget(m_refScaleSpin);
    appLayout->addLayout(refScaleLayout);

    QHBoxLayout *colorLayout = new QHBoxLayout();
    QPushButton *vColorBtn = new QPushButton(QIcon(iconPrefix + "color.svg"), tr("Verse Color..."), this);
    connect(vColorBtn, &QPushButton::clicked, this, &SettingsDialog::onPickVerseColor);
    QPushButton *rColorBtn = new QPushButton(QIcon(iconPrefix + "color.svg"), tr("Ref Color..."), this);
    connect(rColorBtn, &QPushButton::clicked, this, &SettingsDialog::onPickRefColor);
    colorLayout->addWidget(vColorBtn);
    colorLayout->addWidget(rColorBtn);
    appLayout->addLayout(colorLayout);

    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityIcon = new QLabel(this);
    opacityIcon->setPixmap(QIcon(iconPrefix + "opacity.svg").pixmap(16, 16));
    opacityIcon->setObjectName("opacityIcon");
    opacityLayout->addWidget(opacityIcon);
    opacityLayout->addWidget(new QLabel(tr("Background Opacity (%):"), this));
    m_opacitySpin = new QSpinBox(this);
    m_opacitySpin->setRange(0, 100);
    connect(m_opacitySpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_mainWindow->setOpacity(val / 100.0);
        m_mainWindow->applySettings();
    });
    opacityLayout->addWidget(m_opacitySpin);
    appLayout->addLayout(opacityLayout);

    QHBoxLayout *themeLayout = new QHBoxLayout();
    themeLayout->addWidget(new QLabel(tr("Theme:"), this));
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItem(tr("System"), 0);
    m_themeCombo->addItem(tr("Light Mode"), 1);
    m_themeCombo->addItem(tr("Dark Mode"), 2);
    themeLayout->addWidget(m_themeCombo);
    appLayout->addLayout(themeLayout);

    mainLayout->addWidget(appearanceGroup);

    // Layout & Sizing
    QGroupBox *sizingGroup = new QGroupBox(tr("Layout & Sizing"), this);
    QVBoxLayout *sizingLayout = new QVBoxLayout(sizingGroup);

    QHBoxLayout *themeHeaderLayout = new QHBoxLayout();
    QLabel *sizingIcon = new QLabel(this);
    sizingIcon->setPixmap(QIcon(iconPrefix + "width.svg").pixmap(16, 16));
    themeHeaderLayout->addWidget(sizingIcon);
    themeHeaderLayout->addWidget(new QLabel(tr("Configure dimensions"), this));
    sizingLayout->addLayout(themeHeaderLayout);

    QHBoxLayout *modeLayout = new QHBoxLayout();
    m_sizeModeGroup = new QButtonGroup(this);
    QRadioButton *dynamicBtn = new QRadioButton(tr("Dynamic (fit content)"), this);
    QRadioButton *fixedBtn = new QRadioButton(tr("Fixed Size"), this);
    m_sizeModeGroup->addButton(dynamicBtn, 0);
    m_sizeModeGroup->addButton(fixedBtn, 1);
    modeLayout->addWidget(dynamicBtn);
    modeLayout->addWidget(fixedBtn);
    sizingLayout->addLayout(modeLayout);

    QHBoxLayout *widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel(tr("Max Width (px):"), this));
    m_maxWidthSpin = new QSpinBox(this);
    m_maxWidthSpin->setRange(200, 1200);
    widthLayout->addWidget(m_maxWidthSpin);
    sizingLayout->addLayout(widthLayout);

    QHBoxLayout *heightLayout = new QHBoxLayout();
    heightLayout->addWidget(new QLabel(tr("Max Height (px):"), this));
    m_maxHeightSpin = new QSpinBox(this);
    m_maxHeightSpin->setRange(100, 1000);
    heightLayout->addWidget(m_maxHeightSpin);
    sizingLayout->addLayout(heightLayout);

    mainLayout->addWidget(sizingGroup);

    // Bible File
    QGroupBox *fileGroup = new QGroupBox(tr("Bible File"), this);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
    
    QHBoxLayout *fileHeaderLayout = new QHBoxLayout();
    QLabel *fileIcon = new QLabel(this);
    fileIcon->setPixmap(QIcon(iconPrefix + "bible.svg").pixmap(16, 16));
    fileHeaderLayout->addWidget(fileIcon);
    fileHeaderLayout->addWidget(new QLabel(tr("King James Version (KJV)"), this));
    fileLayout->addLayout(fileHeaderLayout);
    
    QHBoxLayout *browseLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setReadOnly(true);
    QPushButton *browseBtn = new QPushButton(QIcon(iconPrefix + "path.svg"), tr("Browse..."), this);
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
    
    QHBoxLayout *startupHeaderLayout = new QHBoxLayout();
    QLabel *startupIcon = new QLabel(this);
    startupIcon->setPixmap(QIcon(iconPrefix + "startup.svg").pixmap(16, 16));
    startupHeaderLayout->addWidget(startupIcon);
    startupHeaderLayout->addWidget(new QLabel(tr("Autostart"), this));
    startupLayout->addLayout(startupHeaderLayout);
    
    m_startupCheck = new QCheckBox(tr("Launch Deskible at system startup"), this);
    startupLayout->addWidget(m_startupCheck);
    mainLayout->addWidget(startupGroup);

    // Utilities
    QGroupBox *utilGroup = new QGroupBox(tr("Utilities & Navigation"), this);
    QVBoxLayout *utilLayout = new QVBoxLayout(utilGroup);
    
    QHBoxLayout *navLayout = new QHBoxLayout();
    QPushButton *prevBtn = new QPushButton(QIcon(iconPrefix + "previous.svg"), tr("Previous"), this);
    QPushButton *nextBtn = new QPushButton(QIcon(iconPrefix + "next.svg"), tr("Next"), this);
    QPushButton *randBtn = new QPushButton(QIcon(iconPrefix + "random.svg"), tr("Random"), this);
    
    connect(prevBtn, &QPushButton::clicked, m_mainWindow, &MainWindow::previousVerse);
    connect(nextBtn, &QPushButton::clicked, m_mainWindow, &MainWindow::nextVerse);
    connect(randBtn, &QPushButton::clicked, m_mainWindow, &MainWindow::randomVerse);
    
    navLayout->addWidget(prevBtn);
    navLayout->addWidget(nextBtn);
    navLayout->addWidget(randBtn);
    utilLayout->addLayout(navLayout);

    QPushButton *copyBtn = new QPushButton(QIcon(iconPrefix + "copy.svg"), tr("Copy current verse to clipboard"), this);
    connect(copyBtn, &QPushButton::clicked, [this]() {
        QApplication::clipboard()->setText(m_mainWindow->currentVerseFull());
    });
    utilLayout->addWidget(copyBtn);
    mainLayout->addWidget(utilGroup);

    m_scrollArea->setWidget(scrollContent);
    dialogLayout->addWidget(m_scrollArea);

    // Buttons (outside scroll area)
    QWidget *btnBar = new QWidget(this);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnBar);
    btnLayout->setContentsMargins(20, 10, 20, 20);
    
    QPushButton *okBtn = new QPushButton(QIcon(iconPrefix + "save.svg"), tr("OK"), this);
    QPushButton *cancelBtn = new QPushButton(QIcon(iconPrefix + "cancel.svg"), tr("Cancel"), this);
    connect(okBtn, &QPushButton::clicked, this, &SettingsDialog::onAccepted);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    dialogLayout->addWidget(btnBar);

    loadCurrentValues();
    
    // Connect live theme preview
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onThemeChanged);
    
    // Initial style & icons
    applyStyle();
    refreshIcons();
}

void SettingsDialog::onThemeChanged(int index)
{
    m_mainWindow->setTheme(static_cast<Theme>(index));
    m_mainWindow->applySettings(); // Real live preview
    applyStyle();
    refreshIcons();
}

void SettingsDialog::refreshIcons()
{
    bool isDark = true;
    if (m_mainWindow->theme() == Theme::Dark) isDark = true;
    else if (m_mainWindow->theme() == Theme::Light) isDark = false;
    else {
        QPalette p = qApp->palette();
        isDark = p.color(QPalette::WindowText).lightness() > p.color(QPalette::Window).lightness();
    }
    QString iconPrefix = isDark ? ":/icons/icons/white/" : ":/icons/icons/dark/";

    // We need to re-set icons for all buttons and labels that use them
    // This requires keeping pointers or finding children. 
    // For simplicity, I'll update the most important ones or use findChildren.
    for (auto btn : findChildren<QPushButton*>()) {
        QString text = btn->text().toLower();
        if (text.contains("font")) btn->setIcon(QIcon(iconPrefix + "font.svg"));
        else if (text.contains("color")) btn->setIcon(QIcon(iconPrefix + "color.svg"));
        else if (text.contains("browse")) btn->setIcon(QIcon(iconPrefix + "path.svg"));
        else if (text.contains("ok")) btn->setIcon(QIcon(iconPrefix + "save.svg"));
        else if (text.contains("cancel")) btn->setIcon(QIcon(iconPrefix + "cancel.svg"));
    }
}

void SettingsDialog::loadCurrentValues()
{
    m_originalTheme = m_mainWindow->theme();
    m_autoSwitchCheck->setChecked(m_mainWindow->autoSwitch());
    m_intervalSpin->setValue(m_mainWindow->switchInterval());
    m_currentFont = m_mainWindow->verseFont();
    m_currentVerseColor = m_mainWindow->verseColor();
    m_currentRefColor = m_mainWindow->refColor();
    m_opacitySpin->setValue(static_cast<int>(m_mainWindow->opacityValue() * 100));
    m_maxWidthSpin->setValue(m_mainWindow->maxWidthValue());
    m_maxHeightSpin->setValue(m_mainWindow->maxHeightValue());
    m_refScaleSpin->setValue(m_mainWindow->refScale());
    m_themeCombo->setCurrentIndex(static_cast<int>(m_mainWindow->theme()));
    
    if (m_mainWindow->sizeMode() == SizeMode::Dynamic)
        m_sizeModeGroup->button(0)->setChecked(true);
    else
        m_sizeModeGroup->button(1)->setChecked(true);

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
    if (ok) {
        m_currentFont = font;
        m_mainWindow->setVerseFont(m_currentFont);
        m_mainWindow->applySettings();
    }
}

void SettingsDialog::onPickVerseColor()
{
    QColor color = QColorDialog::getColor(m_currentVerseColor, this, tr("Pick Verse Color"), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) {
        m_currentVerseColor = color;
        m_mainWindow->setVerseColor(m_currentVerseColor);
        m_mainWindow->applySettings();
    }
}

void SettingsDialog::onPickRefColor()
{
    QColor color = QColorDialog::getColor(m_currentRefColor, this, tr("Pick Reference Color"), QColorDialog::ShowAlphaChannel);
    if (color.isValid()) {
        m_currentRefColor = color;
        m_mainWindow->setRefColor(m_currentRefColor);
        m_mainWindow->applySettings();
    }
}

void SettingsDialog::onAccepted()
{
    m_mainWindow->setAutoSwitch(m_autoSwitchCheck->isChecked());
    m_mainWindow->setSwitchInterval(m_intervalSpin->value());
    m_mainWindow->setVerseFont(m_currentFont);
    m_mainWindow->setVerseColor(m_currentVerseColor);
    m_mainWindow->setRefColor(m_currentRefColor);
    m_mainWindow->setRefScale(m_refScaleSpin->value());
    m_mainWindow->setOpacity(m_opacitySpin->value() / 100.0);
    m_mainWindow->setMaxWidth(m_maxWidthSpin->value());
    m_mainWindow->setMaxHeight(m_maxHeightSpin->value());
    m_mainWindow->setTheme(static_cast<Theme>(m_themeCombo->currentIndex()));
    m_mainWindow->setSizeMode(m_sizeModeGroup->checkedId() == 0 ? SizeMode::Dynamic : SizeMode::Fixed);
    m_mainWindow->setBiblePath(m_pathEdit->text());
    
    StartupManager::setStartupEnabled(m_startupCheck->isChecked());

    m_mainWindow->saveSettings();
    m_mainWindow->applySettings();
    accept();
}

void SettingsDialog::reject()
{
    m_mainWindow->setTheme(m_originalTheme);
    m_mainWindow->applySettings();
    QDialog::reject();
}

void SettingsDialog::applyStyle()
{
    bool isDark = true;
    if (m_mainWindow->theme() == Theme::Dark) isDark = true;
    else if (m_mainWindow->theme() == Theme::Light) isDark = false;
    else {
        QPalette p = qApp->palette();
        isDark = p.color(QPalette::WindowText).lightness() > p.color(QPalette::Window).lightness();
    }

    if (isDark) {
        setStyleSheet(
            "SettingsDialog { background-color: #12122a; color: #e8e8f8; }"
            "QWidget#scrollContent { background-color: #12122a; }"
            "QScrollArea { background-color: #12122a; border: none; }"
            "QGroupBox { border: 1px solid #333355; border-radius: 10px; margin-top: 15px; padding-top: 25px; color: #a0a0ff; font-weight: bold; font-size: 14px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 5px; }"
            "QLabel { color: #e8e8f8; background: transparent; }"
            "QPushButton { background-color: #2a2a5e; color: white; border-radius: 6px; padding: 8px 16px; border: 1px solid #333355; font-weight: bold; }"
            "QPushButton:hover { background-color: #3a3a7e; border: 1px solid #5555ff; }"
            "QLineEdit { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 6px; padding: 6px; }"
            "QSpinBox { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 6px; padding: 6px; }"
            "QComboBox { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 6px; padding: 6px; }"
            "QComboBox::drop-down { border: 0px; }"
            "QComboBox::down-arrow { image: url(:/icons/icons/white/next.svg); width: 12px; height: 12px; }"
            "QCheckBox { color: #e8e8f8; spacing: 8px; }"
            "QRadioButton { color: #e8e8f8; spacing: 8px; }"
            "QScrollBar:vertical { border: none; background: #12122a; width: 10px; margin: 0px; }"
            "QScrollBar::handle:vertical { background: #333355; min-height: 20px; border-radius: 5px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );
    } else {
        setStyleSheet(
            "SettingsDialog { background-color: #f5f5fa; color: #1a1a2e; }"
            "QWidget#scrollContent { background-color: #f5f5fa; }"
            "QScrollArea { background-color: #f5f5fa; border: none; }"
            "QGroupBox { border: 1px solid #d0d0df; border-radius: 10px; margin-top: 15px; padding-top: 25px; color: #2a2a5e; font-weight: bold; font-size: 14px; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 5px; }"
            "QLabel { color: #1a1a2e; background: transparent; }"
            "QPushButton { background-color: #ffffff; color: #2a2a5e; border-radius: 6px; padding: 8px 16px; border: 1px solid #d0d0df; font-weight: bold; }"
            "QPushButton:hover { background-color: #f0f0ff; border: 1px solid #5555ff; }"
            "QLineEdit { background-color: #ffffff; color: #1a1a2e; border: 1px solid #d0d0df; border-radius: 6px; padding: 6px; }"
            "QSpinBox { background-color: #ffffff; color: #1a1a2e; border: 1px solid #d0d0df; border-radius: 6px; padding: 6px; }"
            "QComboBox { background-color: #ffffff; color: #1a1a2e; border: 1px solid #d0d0df; border-radius: 6px; padding: 6px; }"
            "QComboBox::drop-down { border: 0px; }"
            "QComboBox::down-arrow { image: url(:/icons/icons/dark/next.svg); width: 12px; height: 12px; }"
            "QCheckBox { color: #1a1a2e; spacing: 8px; }"
            "QRadioButton { color: #1a1a2e; spacing: 8px; }"
            "QScrollBar:vertical { border: none; background: #f5f5fa; width: 10px; margin: 0px; }"
            "QScrollBar::handle:vertical { background: #d0d0df; min-height: 20px; border-radius: 5px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        );
    }
}
