#include "text_file_writter.h"


/////////////
/// Class ///
/////////////
TextFileWritter::TextFileWritter(QObject *parent) : QObject{parent}
{

}

TextFileWritter::~TextFileWritter()
{

}



///////////////////
/// Handle File ///
///////////////////
bool TextFileWritter::createFile(const QString &filePath)
{
    file.setFileName(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        return true;
    else
        return false;
}

void TextFileWritter::closeFile()
{
    if(file.isOpen())
        file.close();
}

void TextFileWritter::writeRawSentences(const QString &type, const QString &nmeaText)
{
    Q_UNUSED(type);

    if (file.isOpen())
    {
        QTextStream out(&file);
        if(add_timestamp)
            out << getTimeStamp();
        out << nmeaText << '\n';
        out.flush();
    }
}

void TextFileWritter::updateAddTimestamp(bool checked)
{
    add_timestamp = checked;
}
