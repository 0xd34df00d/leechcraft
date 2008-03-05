#include <QtDebug>
#include <QTemporaryFile>
#include "core.h"
#include "parserfactory.h"
#include "rss20parser.h"

Core::Core ()
{
    ParserFactory::Instance ().Register (&RSS20Parser::Instance ());
}

Core& Core::Instance ()
{
    static Core core;
    return core;
}

void Core::Release ()
{
}

void Core::SetProvider (QObject *provider, const QString& feature)
{
    Providers_ [feature] = provider;
    connect (provider, SIGNAL (jobFinished (int)), this, SLOT (handleJobFinished (int)));
    connect (provider, SIGNAL (jobRemoved (int)), this, SLOT (handleJobRemoved (int)));
    connect (provider, SIGNAL (jobError (int, IDirectDownload::Error)), this, SLOT (handleJobError (int, IDirectDownload::Error)));
}

void Core::AddFeed (const QString& url)
{
    QObject *provider = Providers_ ["http"];
    IDirectDownload *idd = qobject_cast<IDirectDownload*> (provider);
    if (!provider || !idd)
    {
        emit error (tr ("Strange, but no suitable provider found"));
        return;
    }
    if (!idd->CouldDownload (url))
    {
        emit error (tr ("Could not handle URL %1").arg (url));
        return;
    }
    QTemporaryFile file;
    file.open ();
    DirectDownloadParams params = { url, file.fileName (), true, false };
    PendingJob pj = { PendingJob :: RFeedAdded, url, file.fileName () };
    int id = idd->AddJob (params);
    PendingJobs_ [id] = pj;
    file.close ();
}

void Core::handleJobFinished (int id)
{
    if (!PendingJobs_.contains (id))
        return;
    PendingJob pj = PendingJobs_ [id];
    QFile file (pj.Filename_);
    if (!file.open (QIODevice::ReadOnly))
    {
        qWarning () << Q_FUNC_INFO << "could not open file for pj " << pj.Filename_;
        return;
    }
    QByteArray data = file.readAll ();
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        Feed feed = { QByteArray (), QDateTime::currentDateTime () };
        Feeds_ [pj.URL_] = feed;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent (data, true, &errorMsg, &errorLine, &errorColumn))
    {
        emit error (tr ("Parse error: %1, line %2, column %3, filename %4").arg (errorMsg).arg (errorLine).arg (errorColumn).arg (pj.Filename_));
        return;
    }
    Parser *parser = ParserFactory::Instance ().Return (doc);
    if (!parser)
    {
        emit error (tr ("Could not find parser to parse file %1").arg (pj.Filename_));
        return;
    }
    QList<Item> items = parser->Parse (Feeds_ [pj.URL_].Previous_, data);
    for (int i = 0; i < items.size (); ++i)
        qDebug () << items.at (i).Title_;

    Feeds_ [pj.URL_].Previous_ = data;
}

