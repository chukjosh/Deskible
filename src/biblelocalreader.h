#ifndef BIBLELOCALREADER_H
#define BIBLELOCALREADER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QStringList>

struct VerseRecord {
    QString bookName;   // e.g. "Genesis", "1 Samuel", "Song of Solomon"
    int     chapter;    // e.g. 1
    int     verse;      // e.g. 1
    QString text;       // full verse text
    QString reference;  // e.g. "Genesis 1:1"
};

class BibleLocalReader : public QObject
{
    Q_OBJECT

public:
    explicit BibleLocalReader(QObject *parent = nullptr);

    bool openFile(const QString &path);
    bool isLoaded() const;
    QString errorMessage() const;
    QString filePath() const;
    int verseCount() const;
    
    void fetchRandomVerse();
    void fetchVerse(int index);
    QStringList availableBooks() const;

signals:
    void verseReady(QString text, QString reference, QString version);
    void errorOccurred(QString message);

private:
    QVector<VerseRecord> m_verses;
    bool m_loaded = false;
    QString m_error;
    QString m_filePath;
    QStringList m_books;
};

#endif // BIBLELOCALREADER_H
