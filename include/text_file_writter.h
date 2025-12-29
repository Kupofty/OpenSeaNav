#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "utils.h"

class TextFileWritter : public QObject
{
    Q_OBJECT

    public:
        explicit TextFileWritter(QObject *parent = nullptr);
        ~TextFileWritter();

        bool createFile(const QString &file_path);
        void closeFile();

    private:
        QFile file;
        bool add_timestamp;

    public slots:
        void writeRawSentences(const QString &type, const QString &nmeaText);
        void updateAddTimestamp(bool checked);

};
