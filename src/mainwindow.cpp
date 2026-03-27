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
    QSettings settings("Deskible", "Deskible");
    m_switchInterval = settings.value("switchInterval", 60).toInt();
    m_autoSwitch = settings.value("autoSwitch", true).toBool();
    m_opacity = settings.value("opacity", 0.75).toDouble();
    m_maxWidth = settings.value("maxWidth", 420).toInt();
    m_maxHeight = settings.value("maxHeight", 350).toInt();
    m_theme = static_cast<Theme>(settings.value("theme", 0).toInt());
    m_sizeMode = static_cast<SizeMode>(settings.value("sizeMode", 0).toInt());
    
    m_verseFont = settings.value("verseFont", QFont("Georgia", 13)).value<QFont>();
    m_verseColor = QColor(settings.value("verseColor", "#FFFFFFFF").toString());
    m_refColor = QColor(settings.value("refColor", "#FFAAAAFF").toString());
    
    QString defaultPath = QCoreApplication::applicationDirPath() + "/bibleversions/kjv.txt";
    QString alternatePath = QDir::currentPath() + "/bibleversions/kjv.txt";
    QString sourcePath = QDir::currentPath() + "/../Deskible/bibleversions/kjv.txt"; // Common for CMake build dirs
    
    m_biblePath = settings.value("filePath", "").toString();
    if (m_biblePath.isEmpty()) {
        if (QFile::exists(defaultPath)) m_biblePath = defaultPath;
        else if (QFile::exists(alternatePath)) m_biblePath = alternatePath;
        else if (QFile::exists(sourcePath)) m_biblePath = sourcePath;
        else m_biblePath = defaultPath; // Fallback to original
    }

    QPoint defaultPos;
    if (settings.contains("position")) {
        move(settings.value("position").toPoint());
    } else {
        QRect screenGeom = QApplication::primaryScreen()->availableGeometry();
        move(screenGeom.right() - width() - 440, screenGeom.bottom() - height() - 100); 
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("Deskible", "Deskible");
    settings.setValue("switchInterval", m_switchInterval);
    settings.setValue("autoSwitch", m_autoSwitch);
    settings.setValue("opacity", m_opacity);
    settings.setValue("maxWidth", m_maxWidth);
    settings.setValue("maxHeight", m_maxHeight);
    settings.setValue("theme", static_cast<int>(m_theme));
    settings.setValue("sizeMode", static_cast<int>(m_sizeMode));
    settings.setValue("verseFont", m_verseFont);
    settings.setValue("verseColor", m_verseColor.name(QColor::HexArgb));
    settings.setValue("refColor", m_refColor.name(QColor::HexArgb));
    settings.setValue("filePath", m_biblePath);
    settings.setValue("position", pos());
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
void MainWindow::setVerseColor(const QColor &color) { m_verseColor = color; }
void MainWindow::setRefColor(const QColor &color) { m_refColor = color; }
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
        
        QFontMetrics fm(m_verseFont);
        QRect textRect = fm.boundingRect(0, 0, contentWidth, 1000, 
                                          Qt::AlignLeft | Qt::TextWordWrap, m_currentText);
        
        int neededHeight = textRect.height() + (margin * 2) + (padding * 2);
        
        // Add room for the reference if it exists
        if (!m_currentRef.isEmpty()) {
             neededHeight += fm.height() * 0.8 + 10;
        }

        neededHeight = qBound(80, neededHeight, m_maxHeight);
        setFixedHeight(neededHeight);
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

    QColor bgColor = isDark ? QColor(20, 20, 35) : QColor(245, 245, 250);
    bgColor.setAlphaF(m_opacity);
    
    QColor shadowColor = isDark ? QColor(0, 0, 0) : QColor(100, 100, 120);
    int shadowAlpha = isDark ? 12 : 8;

    // Drop shadow
    for (int i = 6; i >= 1; --i) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(shadowColor.red(), shadowColor.green(), shadowColor.blue(), shadowAlpha * i));
        painter.drawRoundedRect(rect.adjusted(-i, -i, i, i), 16 + i, 16 + i);
    }

    // Background
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, 16, 16);

    // Subtle edge highlight
    painter.setPen(QPen(isDark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 20), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect, 16, 16);

    // Top subtle highlight (glass effect)
    QPainterPath highlight;
    highlight.moveTo(rect.left() + 16, rect.top() + 1);
    highlight.lineTo(rect.right() - 16, rect.top() + 1);
    painter.setPen(QPen(isDark ? QColor(255, 255, 255, 45) : QColor(255, 255, 255, 120), 1));
    painter.drawPath(highlight);

    // Content
    int padding = 18;
    QRect contentRect = rect.adjusted(padding, padding, -padding, -padding);

    // Verse Text
    painter.setFont(m_verseFont);
    painter.setPen(m_verseColor);
    painter.drawText(contentRect, Qt::AlignLeft | Qt::TextWordWrap, m_currentText);

    // Reference Text
    if (!m_currentRef.isEmpty()) {
        QFont refFont = m_verseFont;
        refFont.setPointSizeF(m_verseFont.pointSizeF() * 0.78);
        refFont.setItalic(true);
        painter.setFont(refFont);
        painter.setPen(m_refColor);
        
        QString refText = QString("%1  ·  %2").arg(m_currentRef, m_currentVersion);
        painter.drawText(contentRect, Qt::AlignRight | Qt::AlignBottom, refText);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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

