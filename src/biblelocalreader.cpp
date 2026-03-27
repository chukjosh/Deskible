#include "biblelocalreader.h"
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDebug>

BibleLocalReader::BibleLocalReader(QObject *parent)
    : QObject(parent)
{
}

bool BibleLocalReader::openFile(const QString &path)
{
    m_loaded = false;
    m_error.clear();
    m_verses.clear();
    m_books.clear();
    m_filePath = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_error = tr("Could not open file: %1").arg(file.errorString());
        emit errorOccurred(m_error);
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // Skip line 1 (BOM + version label) and line 2 (attribution)
    if (in.atEnd()) return false;
    in.readLine(); 
    if (in.atEnd()) return false;
    in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        // Split on FIRST tab only
        int tabIdx = line.indexOf('\t');
        if (tabIdx == -1) continue;

        QString refStr = line.left(tabIdx).trimmed(); // e.g. "Genesis 1:1"
        QString verseText = line.mid(tabIdx + 1).trimmed();

        // Parse reference: find LAST colon
        int colonIdx = refStr.lastIndexOf(':');
        if (colonIdx == -1) continue;

        QString bookChapter = refStr.left(colonIdx).trimmed(); // e.g. "Genesis 1"
        QString verseStr = refStr.mid(colonIdx + 1).trimmed();

        // Parse book + chapter: split on last space
        int spaceIdx = bookChapter.lastIndexOf(' ');
        if (spaceIdx == -1) continue;

        QString bookName = bookChapter.left(spaceIdx).trimmed();
        QString chapterStr = bookChapter.mid(spaceIdx + 1).trimmed();

        bool okC, okV;
        int chapterNum = chapterStr.toInt(&okC);
        int verseNum = verseStr.toInt(&okV);

        if (!okC || !okV) continue;

        VerseRecord record;
        record.bookName = bookName;
        record.chapter = chapterNum;
        record.verse = verseNum;
        record.text = verseText;
        record.reference = refStr;

        m_verses.append(record);
        
        if (!m_books.contains(bookName)) {
            m_books.append(bookName);
        }
    }

    if (m_verses.size() == 31102) {
        m_loaded = true;
        return true;
    } else {
        m_error = tr("Parsed %1 verses, expected 31102.").arg(m_verses.size());
        emit errorOccurred(m_error);
        return false;
    }
}

bool BibleLocalReader::isLoaded() const
{
    return m_loaded;
}

QString BibleLocalReader::errorMessage() const
{
    return m_error;
}

QString BibleLocalReader::filePath() const
{
    return m_filePath;
}

int BibleLocalReader::verseCount() const
{
    return m_verses.size();
}

void BibleLocalReader::fetchRandomVerse()
{
    if (m_verses.isEmpty()) return;
    int idx = QRandomGenerator::global()->bounded(m_verses.size());
    fetchVerse(idx);
}

void BibleLocalReader::fetchVerse(int index)
{
    if (index >= 0 && index < m_verses.size()) {
        const auto &v = m_verses.at(index);
        emit verseReady(v.text, v.reference, "KJV");
    }
}

QStringList BibleLocalReader::availableBooks() const
{
    return m_books;
}
