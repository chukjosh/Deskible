#include "systemtraymanager.h"
#include "mainwindow.h"
#include "settingsdialog.h"
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QMenu>

SystemTrayManager::SystemTrayManager(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(createTrayIcon());
    m_trayIcon->setToolTip("Deskible");

    m_trayMenu = new QMenu();
    m_trayMenu->setStyleSheet(
        "QMenu { background-color: #1a1a2e; color: white; border: 1px solid #333355; border-radius: 8px; padding: 4px; }"
        "QMenu::item { padding: 6px 20px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: #2a2a5e; }"
    );

    m_trayMenu->addAction(QIcon(":/icons/icons/bible.svg"), tr("Show/Hide"), [this]() {
        if (m_window->isVisible()) m_window->hide();
        else m_window->show();
    });
    m_trayMenu->addAction(QIcon(":/icons/icons/next.svg"), tr("Next Verse"), m_window, &MainWindow::nextVerse);
    m_trayMenu->addAction(QIcon(":/icons/icons/random.svg"), tr("Random Verse"), m_window, &MainWindow::randomVerse);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon(":/icons/icons/settings.svg"), tr("Settings"), [this]() {
        SettingsDialog dialog(m_window);
        dialog.exec();
    });
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QIcon(":/icons/icons/close.svg"), tr("Quit"), qApp, &QCoreApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &SystemTrayManager::onTrayIconActivated);
    connect(m_window, &MainWindow::verseChanged, this, &SystemTrayManager::updateTooltip);
}

void SystemTrayManager::show()
{
    m_trayIcon->show();
}

void SystemTrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (m_window->isVisible()) m_window->hide();
        else m_window->show();
    }
}

void SystemTrayManager::updateTooltip(const QString &text, const QString &reference)
{
    QString snippet = text;
    if (snippet.length() > 64) snippet = snippet.left(60) + "...";
    m_trayIcon->setToolTip(QString("Deskible\n%1\n— %2").arg(snippet, reference));
}

QIcon SystemTrayManager::createTrayIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Navy circle
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(10, 10, 40));
    painter.drawEllipse(2, 2, 28, 28);
    
    // White cross
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(16, 8, 16, 24);
    painter.drawLine(10, 14, 22, 14);
    
    return QIcon(pixmap);
}
