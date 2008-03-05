#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <interfaces/interfaces.h>

class Item;

class Core : public QObject
{
    Q_OBJECT

    Core ();
    QMap<QString, QObject*> Providers_;
    struct PendingJob
    {
        enum Role
        {
            RFeedAdded
            , RFeedUpdated
        } Role_;
        QString URL_;
        QString Filename_;
    };
    QMap<int, PendingJob> PendingJobs_;

    struct Feed
    {
        QByteArray Previous_;
        QDateTime LastUpdate_;
    };
    QMap<QString, Feed> Feeds_;
public:
    static Core& Instance ();
    void Release ();

    void SetProvider (QObject*, const QString&);

    void AddFeed (const QString&);
private slots:
    void handleGotItem (const Item&);
    void handleJobFinished (int);
    void handleJobRemoved (int);
    void handleJobError (int, IDirectDownload::Error);
signals:
    void error (const QString&);
};

#endif

