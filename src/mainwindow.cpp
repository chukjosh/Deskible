#include "mainwindow.h"
#include "settingsdialog.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    m_reader = new BibleLocalReader(this);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onAutoSwitchTimer);
    connect(m_reader, &BibleLocalReader::verseReady, this, &MainWindow::onVerseReady);
    
    setWindowIcon(QIcon(":/icons/icons/logo.png"));

    // Ensure config dir exists
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);

    loadSettings();

    if (!m_reader->openFile(m_biblePath)) {
        m_currentText = tr("⚠ File not found.\nRight-click → Settings to locate kjv.txt");
        m_currentRef = "";
        qDebug() << "Deskible: Bible file not found at" << m_biblePath;
    } else {
        randomVerse();
        qDebug() << "Deskible: Bible file loaded from" << m_biblePath;
    }

    applySettings();
    qDebug() << "Deskible: Window initialized at" << pos() << "with size" << size();
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadSettings()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.json";
    QFile file(configPath);
    QJsonObject settings;
    if (file.open(QIODevice::ReadOnly)) {
        settings = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    } else {
        // Fallback for first run or missing file
        qDebug() << "Deskible: No config.json found at" << configPath << ", using defaults.";
    }

    m_switchInterval = settings.contains("switchInterval") ? settings["switchInterval"].toInt() : 60;
    m_autoSwitch = settings.contains("autoSwitch") ? settings["autoSwitch"].toBool() : true;
    m_opacity = settings.contains("opacity") ? settings["opacity"].toDouble() : 0.0;
    m_maxWidth = settings.contains("maxWidth") ? settings["maxWidth"].toInt() : 420;
    m_maxHeight = settings.contains("maxHeight") ? settings["maxHeight"].toInt() : 350;
    m_theme = static_cast<Theme>(settings.contains("theme") ? settings["theme"].toInt() : 0);
    m_sizeMode = static_cast<SizeMode>(settings.contains("sizeMode") ? settings["sizeMode"].toInt() : 0);
    
    if (settings.contains("verseFont")) {
        m_verseFont.fromString(settings["verseFont"].toString());
    } else {
        m_verseFont = QFont("Georgia", 13);
    }
    m_verseScale = settings.contains("verseScale") ? settings["verseScale"].toDouble() : 1.0;
    m_verseColor = QColor(settings.contains("verseColor") ? settings["verseColor"].toString() : "#FFFFFFFF");
    m_accentColor = QColor(settings.contains("accentColor") ? settings["accentColor"].toString() : "#5555FF");
    m_refColor = QColor(settings.contains("refColor") ? settings["refColor"].toString() : "#FFAAAAFF");
    m_refScale = settings.contains("refScale") ? settings["refScale"].toDouble() : 0.78;
    
    m_biblePath = settings.contains("filePath") ? settings["filePath"].toString() : "";
    if (m_biblePath.isEmpty()) {
        QString defaultPath = QCoreApplication::applicationDirPath() + "/bibleversions/kjv.txt";
        if (QFile::exists(defaultPath)) m_biblePath = defaultPath;
        else m_biblePath = defaultPath;
    }

    if (settings.contains("x") && settings.contains("y")) {
        move(settings["x"].toInt(), settings["y"].toInt());
    } else {
        QRect screenGeom = QApplication::primaryScreen()->availableGeometry();
        move(screenGeom.right() - 440, screenGeom.bottom() - 150); 
    }
}

void MainWindow::saveSettings()
{
    QJsonObject settings;
    settings["switchInterval"] = m_switchInterval;
    settings["autoSwitch"] = m_autoSwitch;
    settings["opacity"] = m_opacity;
    settings["maxWidth"] = m_maxWidth;
    settings["maxHeight"] = m_maxHeight;
    settings["theme"] = static_cast<int>(m_theme);
    settings["sizeMode"] = static_cast<int>(m_sizeMode);
    settings["verseFont"] = m_verseFont.toString();
    settings["verseScale"] = m_verseScale;
    settings["verseColor"] = m_verseColor.name(QColor::HexArgb);
    settings["accentColor"] = m_accentColor.name(QColor::HexArgb);
    settings["refColor"] = m_refColor.name(QColor::HexArgb);
    settings["refScale"] = m_refScale;
    settings["filePath"] = m_biblePath;
    settings["x"] = pos().x();
    settings["y"] = pos().y();

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.json";
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(settings).toJson());
        file.close();
    } else {
        qDebug() << "Deskible: Failed to save config.json to" << configPath;
    }
}

void MainWindow::applySettings()
{
    // Reload Bible if path changed
    if (m_reader->filePath() != m_biblePath) {
        if (!m_reader->openFile(m_biblePath)) {
            m_currentText = tr("⚠ File not found.\nRight-click → Settings to locate kjv.txt");
            m_currentRef = "";
        } else {
            randomVerse();
        }
    }

    if (m_autoSwitch) {
        m_timer->start(m_switchInterval * 1000);
    } else {
        m_timer->stop();
    }
    
    updateWindowSize();
    update();
}

void MainWindow::setAutoSwitch(bool enabled) { m_autoSwitch = enabled; }
void MainWindow::setSwitchInterval(int seconds) { m_switchInterval = seconds; }
void MainWindow::setOpacity(double opacity) { m_opacity = opacity; }
void MainWindow::setMaxWidth(int width) { m_maxWidth = width; }
void MainWindow::setMaxHeight(int height) { m_maxHeight = height; }
void MainWindow::setTheme(Theme theme) { m_theme = theme; }
void MainWindow::setSizeMode(SizeMode mode) { m_sizeMode = mode; }
void MainWindow::setVerseFont(const QFont &font) { m_verseFont = font; }
void MainWindow::setVerseScale(double scale) { m_verseScale = scale; }
void MainWindow::setVerseColor(const QColor &color) { m_verseColor = color; }
void MainWindow::setAccentColor(const QColor &color) { m_accentColor = color; }
void MainWindow::setHoverOpacity(double opacity) { m_hoverOpacity = opacity; update(); }
void MainWindow::setRefColor(const QColor &color) { m_refColor = color; }
void MainWindow::setRefScale(double scale) { m_refScale = scale; }

void MainWindow::resetToDefaults()
{
    m_switchInterval = 60;
    m_autoSwitch = true;
    m_opacity = 0.0;
    m_maxWidth = 420;
    m_maxHeight = 350;
    m_theme = Theme::System;
    m_sizeMode = SizeMode::Dynamic;
    m_verseFont = QFont("Georgia", 13);
    m_verseScale = 1.0;
    m_verseColor = QColor("#FFFFFFFF");
    m_accentColor = QColor("#5555FF");
    m_refColor = QColor("#FFAAAAFF");
    m_refScale = 0.78;
    applySettings();
}

void MainWindow::setBiblePath(const QString &path) { m_biblePath = path; }

void MainWindow::nextVerse()
{
    if (m_historyIndex < m_history.size() - 1) {
        m_historyIndex++;
        const auto &v = m_history.at(m_historyIndex);
        onVerseReady(v.text, v.reference, "KJV");
    } else {
        randomVerse();
    }
}

void MainWindow::previousVerse()
{
    if (m_historyIndex > 0) {
        m_historyIndex--;
        const auto &v = m_history.at(m_historyIndex);
        onVerseReady(v.text, v.reference, "KJV");
    }
}

void MainWindow::randomVerse()
{
    m_reader->fetchRandomVerse();
}

void MainWindow::onVerseReady(QString text, QString reference, QString version)
{
    m_currentText = QString("\u201C %1 \u201D").arg(text);
    m_currentRef = reference;
    m_currentVersion = version;

    // Update history if it's a new verse (not from history navigation)
    bool fromNav = false;
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
       if (m_history.at(m_historyIndex).reference == reference) fromNav = true;
    }

    if (!fromNav) {
        VerseRecord rec;
        rec.text = text;
        rec.reference = reference;
        addToHistory(rec);
    }

    emit verseChanged(m_currentText, m_currentRef);
    updateWindowSize();
    update();
}

void MainWindow::addToHistory(const VerseRecord &record)
{
    if (m_history.size() >= 50) m_history.removeFirst();
    m_history.append(record);
    m_historyIndex = m_history.size() - 1;
}

void MainWindow::onAutoSwitchTimer()
{
    randomVerse();
}

void MainWindow::updateWindowSize()
{
    if (m_sizeMode == SizeMode::Dynamic) {
        setFixedWidth(m_maxWidth);
        
        // Calculate needed height
        int padding = 18;
        int margin = 10;
        int contentWidth = m_maxWidth - (margin * 2) - (padding * 2);
        
        QFont vFont = m_verseFont;
        vFont.setPointSizeF(m_verseFont.pointSizeF() * m_verseScale);
        QFontMetrics fm(vFont);
        QRect textRect = fm.boundingRect(0, 0, contentWidth, 1000, 
                                          Qt::AlignLeft | Qt::TextWordWrap, m_currentText);
        
        int neededHeight = textRect.height() + (margin * 2) + (padding * 2);
        
        // Add room for the reference if it exists
        if (!m_currentRef.isEmpty()) {
             neededHeight += fm.height() * 0.8 + 10;
        }

        neededHeight = qBound(80, neededHeight, m_maxHeight);
        
        // Calculate new geometry keeping the CENTER position
        QPoint oldCenter = geometry().center();
        QRect newRect(0, 0, m_maxWidth, neededHeight);
        newRect.moveCenter(oldCenter);

        // Animate size change while keeping center
        if (m_sizeAnimation) m_sizeAnimation->stop();
        m_sizeAnimation = new QPropertyAnimation(this, "geometry");
        m_sizeAnimation->setDuration(400);
        m_sizeAnimation->setStartValue(geometry());
        m_sizeAnimation->setEndValue(newRect);
        m_sizeAnimation->setEasingCurve(QEasingCurve::OutBack);
        m_sizeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        setFixedSize(m_maxWidth, m_maxHeight);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int margin = 10;
    QRect rect = this->rect().adjusted(margin, margin, -margin, -margin);

    // Background logic based on Theme and Opacity
    bool isDark = true;
    if (m_theme == Theme::Dark) isDark = true;
    else if (m_theme == Theme::Light) isDark = false;
    else {
        // System theme detection
        QPalette p = qApp->palette();
        isDark = p.color(QPalette::WindowText).lightness() > p.color(QPalette::Window).lightness();
    }

    // Background Colors
    QColor baseBg = isDark ? QColor(15, 15, 25) : QColor(250, 250, 255);
    QColor accentLayer = m_accentColor;
    
    double totalOpacity = m_opacity + (m_hoverOpacity * 0.25);
    if (totalOpacity > 0.95) totalOpacity = 0.95;

    baseBg.setAlphaF(totalOpacity);
    accentLayer.setAlphaF(m_hoverOpacity * 0.1); // Subtle tint on hover

    if (totalOpacity < 0.01) {
        // Truly transparent when not hovered and base opacity is 0
        return; 
    }

    // --- Layer 1: Outer Shadow ---
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.drawRoundedRect(rect.adjusted(2, 2, 2, 2), 14, 14);

    // --- Layer 2: Main Body ---
    painter.setBrush(baseBg);
    painter.drawRoundedRect(rect, 12, 12);

    // --- Layer 3: Accent Tint ---
    if (m_hoverOpacity > 0.01) {
        painter.setBrush(accentLayer);
        painter.drawRoundedRect(rect, 12, 12);
    }

    // --- Layer 4: Modern Border (Glass effect) ---
    QLinearGradient borderGrad(rect.topLeft(), rect.bottomRight());
    if (isDark) {
        borderGrad.setColorAt(0, QColor(255, 255, 255, 60));
        borderGrad.setColorAt(0.5, QColor(255, 255, 255, 20));
        borderGrad.setColorAt(1, QColor(255, 255, 255, 40));
    } else {
        borderGrad.setColorAt(0, QColor(0, 0, 0, 40));
        borderGrad.setColorAt(1, QColor(0, 0, 0, 10));
    }
    
    QPen borderPen(borderGrad, 1.2);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), 12, 12);

    // --- Layer 5: Accent Indicator (Left line) ---
    if (m_hoverOpacity > 0.1) {
        painter.setPen(Qt::NoPen);
        QColor indicatorCol = m_accentColor;
        indicatorCol.setAlphaF(m_hoverOpacity);
        painter.setBrush(indicatorCol);
        painter.drawRoundedRect(2, 20, 3, rect.height() - 40, 1, 1);
    }

    int padding = 18;
    QRect contentRect = rect.adjusted(padding, padding, -padding, -padding);

    // Top subtle highlight (glass effect)
    QPainterPath highlight;
    highlight.moveTo(rect.left() + 16, rect.top() + 1);
    highlight.lineTo(rect.right() - 16, rect.top() + 1);
    painter.setPen(QPen(isDark ? QColor(255, 255, 255, 45) : QColor(255, 255, 255, 120), 1));
    painter.drawPath(highlight);

    // Content area already defined above as contentRect
    
    // Verse Text
    QFont vFont = m_verseFont;
    vFont.setPointSizeF(m_verseFont.pointSizeF() * m_verseScale);
    painter.setFont(vFont);
    
    // Modern Text Shadow (Subtle)
    painter.setPen(QColor(0, 0, 0, 100)); // Darker shadow
    painter.drawText(contentRect.translated(1, 1), Qt::AlignLeft | Qt::TextWordWrap, m_currentText);
    
    painter.setPen(m_verseColor);
    painter.drawText(contentRect, Qt::AlignLeft | Qt::TextWordWrap, m_currentText);

    // Reference Text
    if (!m_currentRef.isEmpty()) {
        QFont refFont = m_verseFont; 
        refFont.setPointSizeF(m_verseFont.pointSizeF() * m_refScale);
        refFont.setItalic(true);
        painter.setFont(refFont);
        
        // Shadow for reference
        painter.setPen(QColor(0, 0, 0, 80));
        painter.drawText(contentRect.translated(1, 1), Qt::AlignRight | Qt::AlignBottom, 
                         QString("%1  ·  %2").arg(m_currentRef, m_currentVersion));
                         
        painter.setPen(m_refColor);
        painter.drawText(contentRect, Qt::AlignRight | Qt::AlignBottom, 
                         QString("%1  ·  %2").arg(m_currentRef, m_currentVersion));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // If clicking the bottom 30% area (where reference usually is), copy it
        if (event->pos().y() > height() * 0.7) {
            QApplication::clipboard()->setText(m_currentRef);
            showToast(tr("Copied: %1").arg(m_currentRef));
        }
        
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragging) {
        m_dragging = false;
        saveSettings();
        event->accept();
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        randomVerse();
        event->accept();
    }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    bool isDark = true;
    if (m_theme == Theme::Dark) isDark = true;
    else if (m_theme == Theme::Light) isDark = false;
    else {
        QPalette p = qApp->palette();
        isDark = p.color(QPalette::WindowText).lightness() > p.color(QPalette::Window).lightness();
    }

    if (isDark) {
        menu.setStyleSheet(
            "QMenu { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 8px; padding: 4px; }"
            "QMenu::item { padding: 6px 20px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: #2a2a5e; }"
        );
    } else {
        menu.setStyleSheet(
            "QMenu { background-color: #ffffff; color: #1a1a2e; border: 1px solid #d0d0df; border-radius: 8px; padding: 4px; }"
            "QMenu::item { padding: 6px 20px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: #f0f0ff; }"
        );
    }

    QString iconPrefix = isDark ? ":/icons/icons/white/" : ":/icons/icons/dark/";

    menu.addAction(QIcon(iconPrefix + "next.svg"), tr("Next Verse"), this, &MainWindow::nextVerse);
    menu.addAction(QIcon(iconPrefix + "previous.svg"), tr("Previous Verse"), this, &MainWindow::previousVerse);
    menu.addAction(QIcon(iconPrefix + "random.svg"), tr("Random Verse"), this, &MainWindow::randomVerse);
    menu.addSeparator();
    menu.addAction(QIcon(iconPrefix + "copy.svg"), tr("Copy Verse"), [this]() {
        QApplication::clipboard()->setText(currentVerseFull());
        showToast(tr("Verse Copied!"));
    });
    menu.addSeparator();
    menu.addAction(QIcon(iconPrefix + "settings.svg"), tr("Settings"), [this]() {
        SettingsDialog dialog(this);
        dialog.exec();
    });
    menu.addSeparator();
    menu.addAction(QIcon(iconPrefix + "close.svg"), tr("Quit"), qApp, &QCoreApplication::quit);

    menu.exec(event->globalPos());
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        if (m_theme == Theme::System) applySettings();
    }
    QWidget::changeEvent(event);
}

void MainWindow::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_hovered = true;
    
    if (m_fadeAnimation) m_fadeAnimation->stop();
    m_fadeAnimation = new QVariantAnimation(this);
    m_fadeAnimation->setDuration(300);
    m_fadeAnimation->setStartValue(m_hoverOpacity);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_fadeAnimation, &QVariantAnimation::valueChanged, [this](const QVariant &val) {
        m_hoverOpacity = val.toDouble();
        update();
    });
    m_fadeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = false;
    
    if (m_fadeAnimation) m_fadeAnimation->stop();
    m_fadeAnimation = new QVariantAnimation(this);
    m_fadeAnimation->setDuration(400);
    m_fadeAnimation->setStartValue(m_hoverOpacity);
    m_fadeAnimation->setEndValue(0.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::InSine);
    connect(m_fadeAnimation, &QVariantAnimation::valueChanged, [this](const QVariant &val) {
        m_hoverOpacity = val.toDouble();
        update();
    });
    m_fadeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showToast(const QString &msg)
{
    if (!m_toastLabel) {
        m_toastLabel = new QLabel(this);
        m_toastLabel->setAlignment(Qt::AlignCenter);
        m_toastLabel->setStyleSheet(
            "background-color: rgba(30,30,50, 200); color: white; border-radius: 12px; "
            "padding: 8px 16px; border: 1px solid rgba(85, 85, 255, 120); font-weight: bold;"
        );
    }
    
    m_toastLabel->setText(msg);
    m_toastLabel->adjustSize();
    m_toastLabel->move((width() - m_toastLabel->width())/2, (height() - m_toastLabel->height())/2);
    m_toastLabel->show();
    m_toastLabel->raise();
    
    // Animate opacity since labels don't support windowOpacity well as children
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    m_toastLabel->setGraphicsEffect(eff);
    QPropertyAnimation *toastAnim = new QPropertyAnimation(eff, "opacity");
    toastAnim->setDuration(1500);
    toastAnim->setStartValue(1.0);
    toastAnim->setEndValue(0.0);
    toastAnim->setEasingCurve(QEasingCurve::InExpo);
    connect(toastAnim, &QPropertyAnimation::finished, m_toastLabel, &QLabel::hide);
    toastAnim->start(QAbstractAnimation::DeleteWhenStopped);
}


