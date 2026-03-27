#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPoint>
#include <QTimer>
#include <QVector>
#include <QFont>
#include <QColor>
#include <QPropertyAnimation>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include "biblelocalreader.h"
#include "common.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadSettings();
    void saveSettings();
    void applySettings();

    // Setters for SettingsDialog
    void setAutoSwitch(bool enabled);
    void setSwitchInterval(int seconds);
    void setOpacity(double opacity);
    void setHoverOpacity(double opacity); // New setter
    void setMaxWidth(int width);
    void setMaxHeight(int height);
    void setTheme(Theme theme);
    void setSizeMode(SizeMode mode);
    void setVerseFont(const QFont &font);
    void setVerseScale(double scale);
    void setVerseColor(const QColor &color);
    void setAccentColor(const QColor &color);
    void setRefColor(const QColor &color);
    void setRefScale(double scale);
    void setBiblePath(const QString &path);
    void resetToDefaults();

    // Verse navigation
    void nextVerse();
    void previousVerse();
    void randomVerse();

    BibleLocalReader* reader() const { return m_reader; }
    QString currentVerseFull() const { return m_currentText + " (" + m_currentRef + ")"; }
    
    // Getters for SettingsDialog
    bool autoSwitch() const { return m_autoSwitch; }
    int switchInterval() const { return m_switchInterval; }
    double opacityValue() const { return m_opacity; }
    double hoverOpacity() const { return m_hoverOpacity; } // New getter
    int maxWidthValue() const { return m_maxWidth; }
    int maxHeightValue() const { return m_maxHeight; }
    Theme theme() const { return m_theme; }
    SizeMode sizeMode() const { return m_sizeMode; }
    QFont verseFont() const { return m_verseFont; }
    double verseScale() const { return m_verseScale; }
    QColor verseColor() const { return m_verseColor; }
    QColor accentColor() const { return m_accentColor; }
    QColor refColor() const { return m_refColor; }
    double refScale() const { return m_refScale; }
    QString biblePath() const { return m_biblePath; }

signals:
    void verseChanged(const QString &text, const QString &reference);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void changeEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onVerseReady(QString text, QString reference, QString version);
    void onAutoSwitchTimer();

private:
    void updateWindowSize();
    void addToHistory(const VerseRecord &record);

    BibleLocalReader *m_reader;
    QTimer *m_timer;

    // Current verse
    QString m_currentText;
    QString m_currentRef;
    QString m_currentVersion;

    // History
    QVector<VerseRecord> m_history;
    int m_historyIndex = -1;

    // Settings
    bool m_autoSwitch = true;
    int m_switchInterval = 60;
    int m_maxWidth = 420;
    int m_maxHeight = 350;
    Theme m_theme = Theme::System;
    SizeMode m_sizeMode = SizeMode::Dynamic;
    QFont m_verseFont;
    double m_verseScale = 1.0;
    QColor m_verseColor;
    QColor m_accentColor = QColor("#5555FF"); // Modern Indigo
    QColor m_refColor;
    double m_refScale = 0.78;
    QString m_biblePath;
    double m_opacity = 0.0; // Moved and changed initial value
    double m_hoverOpacity = 0.0; // Animated value

    // Interaction
    bool m_dragging = false;
    bool m_hovered = false;
    QPoint m_dragPos;
    QVariantAnimation *m_fadeAnimation = nullptr;
    QPropertyAnimation *m_sizeAnimation = nullptr;
    QLabel *m_toastLabel = nullptr;
    void showToast(const QString &msg);
};

#endif // MAINWINDOW_H
