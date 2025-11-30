#pragma once

#include <QObject>
#include <QCoreApplication>
#include <QDebug>
#include <QString>

#include "nmea_list.h"

class OutputWriter : public QObject
{
    Q_OBJECT

    public:
        explicit OutputWriter(QObject *parent = nullptr);
        virtual ~OutputWriter();

        virtual void sendData(const QString &line) = 0;

        // Get
        bool getSocketOutputActivated();

        // Update authorized outputs
        void updateSocketOutputActivated(bool check);
        void updateOutputNMEA(const QString &type, bool check);

    public slots:
        void publishNMEA(const QString &type, const QString &nmeaText);

    private:
        bool socketOutputActivated = 0;
        QMap<QString, bool> outputFlags;
};
