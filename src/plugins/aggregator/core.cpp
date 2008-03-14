#include <QtDebug>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryFile>
#include <QTimer>
#include <plugininterface/proxy.h>
#include "core.h"
#include "parserfactory.h"
#include "rss20parser.h"
#include "treeitem.h"

Core::Core ()
{
    ParserFactory::Instance ().Register (&RSS20Parser::Instance ());
    QList<QVariant> rootData;
    rootData << tr ("Name") << tr ("Date");
    RootItem_ = new TreeItem (rootData);

    qRegisterMetaTypeStreamOperators<Feed> ("Feed");

    QTimer *updateTimer = new QTimer (this);
    updateTimer->start (30 * 1000);
    connect (updateTimer, SIGNAL (timeout ()), this, SLOT (updateFeeds ()));
}

Core& Core::Instance ()
{
    static Core core;
    return core;
}

void Core::Release ()
{
    QList<Feed> feeds = Feeds_.values ();
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("Aggregator");
    settings.beginWriteArray ("Feeds");
    for (int i = 0; i < feeds.size (); ++i)
    {
        settings.setArrayIndex (i);
        Feed feed = feeds.at (i);

    }
    settings.endArray ();
    settings.endGroup ();
}

void Core::SetProvider (QObject *provider, const QString& feature)
{
    Providers_ [feature] = provider;
    if (feature == "http")
    {
        connect (provider, SIGNAL (jobFinished (int)), this, SLOT (handleJobFinished (int)));
        connect (provider, SIGNAL (jobRemoved (int)), this, SLOT (handleJobRemoved (int)));
        connect (provider, SIGNAL (jobError (int, IDirectDownload::Error)), this, SLOT (handleJobError (int, IDirectDownload::Error)));
    }
    else if (feature == "cron")
        connect (provider, SIGNAL (shot (int)), this, SLOT (handleShot (int)));
}

void Core::AddFeed (const QString& url)
{
    if (Feeds_.contains (url))
    {
        emit error (tr ("This feed is already added"));
        return;
    }

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

void Core::Activated (const QModelIndex& index)
{
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    if (!item || !TreeItem2Item_.contains (item))
        return;
    QString URL = TreeItem2Item_ [item]->Link_;
    ItemUnread_ [TreeItem2Item_ [item]] = false;
    QDesktopServices::openUrl (QUrl (URL));
}

QString Core::GetDescription (const QModelIndex& index)
{
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    if (!item || !TreeItem2Item_.contains (item))
        return QString ();
    ItemUnread_ [TreeItem2Item_ [item]] = false;
    return TreeItem2Item_ [item]->Description_;
}

int Core::columnCount (const QModelIndex& parent) const
{
    if (parent.isValid ())
        return static_cast<TreeItem*> (parent.internalPointer ())->ColumnCount ();
    else
        return RootItem_->ColumnCount ();
}

QVariant Core::data (const QModelIndex& parent, int role) const
{
    if (!parent.isValid ())
        return QVariant ();

    TreeItem *item = static_cast<TreeItem*> (parent.internalPointer ());
    if (role == Qt::DisplayRole)
        return item->Data (parent.column ());
    else if (role == Qt::ForegroundRole)
    {
        if (TreeItem2Item_.contains (item))
            return ItemUnread_ [TreeItem2Item_ [item]] ? Qt::red : Qt::black;
        else
            return QVariant ();
    }
    else
        return QVariant ();
}

Qt::ItemFlags Core::flags (const QModelIndex& index) const
{
    if (!index.isValid ())
        return 0;
    else
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return RootItem_->Data (column);
    else
        return QVariant ();
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    TreeItem *parentItem;
    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    TreeItem *childItem = parentItem->Child (row);
    if (childItem)
        return createIndex (row, column, childItem);
    else
        return QModelIndex ();
}

QModelIndex Core::parent (const QModelIndex& index) const
{
    if (!index.isValid ())
        return QModelIndex ();

    TreeItem *child = static_cast<TreeItem*> (index.internalPointer ()),
             *parent = child->Parent ();
    if (parent == RootItem_)
        return QModelIndex ();
    return createIndex (parent->Row (), 0, parent);
}

int Core::rowCount (const QModelIndex& parent) const
{
    TreeItem *parentItem;
    if (parent.column () > 0)
        return 0;

    if (!parent.isValid ())
        parentItem = RootItem_;
    else
        parentItem = static_cast<TreeItem*> (parent.internalPointer ());

    return parentItem->ChildCount ();
}

void Core::handleJobFinished (int id)
{
    if (!PendingJobs_.contains (id))
        return;
    PendingJob pj = PendingJobs_ [id];
    PendingJobs_.remove (id);
    QFile file (pj.Filename_);
    if (!file.open (QIODevice::ReadOnly))
    {
        qWarning () << Q_FUNC_INFO << "could not open file for pj " << pj.Filename_;
        return;
    }
    if (!file.size ())
    {
        emit error (tr ("Downloaded file has null size!"));
        return;
    }
    QByteArray data = file.readAll ();
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        Feed feed = { pj.URL_, QDateTime::currentDateTime (), QList<Channel*> () };
        Feeds_ [pj.URL_] = feed;
    }
    if (!Feeds_.contains (pj.URL_))
    {
        emit error (tr ("Feed with url %1 not found.").arg (pj.URL_));
        return;
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
    file.remove ();

    QList<Channel*> channels = parser->Parse (Feeds_ [pj.URL_].Channels_, data);
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
        for (int i = 0; i < channels.size (); ++i)
        {
            QList<QVariant> data;
            Channel *current = channels.at (i);
            data << current->Title_ << (current->LastBuild_.isValid () ? current->LastBuild_ : Feeds_ [pj.URL_].LastUpdate_);
            TreeItem *channelItem = new TreeItem (data, RootItem_);
            RootItem_->AppendChild (channelItem);
            Channel2TreeItem_ [channels.at (i)] = channelItem;
            for (int j = 0; j < current->Items_.size (); ++j)
            {
                Item *it = current->Items_.at (j);
                QList<QVariant> data;
                data << it->Title_ << it->PubDate_;
                TreeItem *item = new TreeItem (data, channelItem);
                channelItem->AppendChild (item);
                Item2TreeItem_ [it] = item;
                TreeItem2Item_ [item] = it;
                ItemUnread_ [it] = true;
            }
        }
        endInsertRows ();
        Feeds_ [pj.URL_].Channels_ = channels;
    }
    else if (pj.Role_ == PendingJob::RFeedUpdated)
    {
        for (int i = 0; i < channels.size (); ++i)
        {
            Channel *current = channels.at (i);
            if (current->Items_.size () <= 0)
                continue;

            int position = -1;
            for (int j = 0; j < Feeds_ [pj.URL_].Channels_.size (); ++j)
                if (*Feeds_ [pj.URL_].Channels_.at (j) == *current)
                {
                    position = j;
                    break;
                }

            if (position == -1)
            {
                beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
                Feeds_ [pj.URL_].Channels_.append (current);
                QList<QVariant> data;
                Channel *current = channels.at (i);
                data << current->Title_ << (current->LastBuild_.isValid () ? current->LastBuild_ : Feeds_ [pj.URL_].LastUpdate_);
                TreeItem *channelItem = new TreeItem (data, RootItem_);
                RootItem_->AppendChild (channelItem);
                Channel2TreeItem_ [channels.at (i)] = channelItem;
                for (int j = 0; j < current->Items_.size (); ++j)
                {
                    Item *it = current->Items_.at (j);
                    QList<QVariant> data;
                    data << it->Title_ << it->PubDate_;
                    TreeItem *item = new TreeItem (data, channelItem);
                    channelItem->AppendChild (item);
                    Item2TreeItem_ [it] = item;
                    TreeItem2Item_ [item] = it;
                    ItemUnread_ [it] = true;
                }
                endInsertRows ();
            }
            else
            {
                TreeItem *channelItem = Channel2TreeItem_ [Feeds_ [pj.URL_].Channels_.at (i)];
                QModelIndex channelIndex = index (channelItem->Row (), 0);

                beginInsertRows (channelIndex, 0, current->Items_.size () - 1);
                for (int j = current->Items_.size () - 1; j >= 0; --j)
                {
                    qDebug () << current->Items_.size ();
                    Item *it = current->Items_.at (j);
                    QList<QVariant> data;
                    data << it->Title_ << it->PubDate_;
                    TreeItem *item = new TreeItem (data, channelItem);
                    channelItem->PrependChild (item);
                    Item2TreeItem_ [it] = item;
                    TreeItem2Item_ [item] = it;
                    ItemUnread_ [it] = true;
                }
                Feeds_ [pj.URL_].Channels_.at (position)->Items_ = current->Items_ + Feeds_ [pj.URL_].Channels_.at (position)->Items_;
                endInsertRows ();
            }
        }
    }
}

void Core::updateFeeds ()
{
    QObject *provider = Providers_ ["http"];
    IDirectDownload *idd = qobject_cast<IDirectDownload*> (provider);
    if (!provider || !idd)
    {
        emit error (tr ("Strange, but no suitable provider found"));
        return;
    }
    QList<QString> urls = Feeds_.keys ();
    for (int i = 0; i < urls.size (); ++i)
    {
        if (!idd->CouldDownload (urls.at (i)))
        {
            emit error (tr ("Could not handle URL %1").arg (urls.at (i)));
            continue;
        }

        QTemporaryFile file;
        file.open ();
        DirectDownloadParams params = { urls.at (i), file.fileName (), true, false };
        PendingJob pj = { PendingJob :: RFeedUpdated, urls.at (i), file.fileName () };
        int id = idd->AddJob (params);
        PendingJobs_ [id] = pj;
        file.close ();
    }
}

