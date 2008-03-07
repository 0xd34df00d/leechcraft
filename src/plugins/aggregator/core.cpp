#include <QtDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryFile>
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
    qDebug () << Q_FUNC_INFO;
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    if (!item || !TreeItem2Item_.contains (item))
        return;
    QString URL = TreeItem2Item_ [item].Link_;
    ItemUnread_ [TreeItem2Item_ [item]] = false;
    QDesktopServices::openUrl (QUrl (URL));
}

QString Core::GetDescription (const QModelIndex& index)
{
    qDebug () << Q_FUNC_INFO;
    TreeItem *item = static_cast<TreeItem*> (index.internalPointer ());
    if (!item || !TreeItem2Item_.contains (item))
        return QString ();
    ItemUnread_ [TreeItem2Item_ [item]] = false;
    return TreeItem2Item_ [item].Description_;
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
        qDebug () << ItemUnread_.size ();
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
    QFile file (pj.Filename_);
    if (!file.open (QIODevice::ReadOnly))
    {
        qWarning () << Q_FUNC_INFO << "could not open file for pj " << pj.Filename_;
        return;
    }
    QByteArray data = file.readAll ();
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        Feed feed = { pj.URL_, QByteArray (), QDateTime::currentDateTime (), QList<Item> () };
        Feeds_ [pj.URL_] = feed;
    }
    Feed& feed = Feeds_ [pj.URL_];
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

    QList<Item> items = parser->Parse (feed.Previous_, data);
    QList<Channel> channels;
    for (int i = 0; i < items.size (); ++i)
    {
        Channel chan = items.at (i).Parent_;
        if (channels.indexOf (chan) == -1)
            channels << chan;
    }
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        qDebug () << Q_FUNC_INFO << "inserting rows";
        beginInsertRows (QModelIndex (), rowCount (), rowCount () + channels.size () - 1);
        for (int i = 0; i < channels.size (); ++i)
        {
            QList<QVariant> data;
            data << channels.at (i).Title_ << feed.LastUpdate_;
            TreeItem *item = new TreeItem (data, RootItem_);
            RootItem_->AppendChild (item);
            Channel2TreeItem_ [channels.at (i)] = item;
        }
        endInsertRows ();
    }
    for (int i = 0; i < items.size (); ++i)
    {
        Item it = items.at (i);
        QList<QVariant> data;
        data << it.Title_ << it.PubDate_;

        TreeItem *channelParent = Channel2TreeItem_ [it.Parent_];
        QModelIndex channelIndex = index (channelParent->Row (), 0);

        beginInsertRows (channelIndex, 0, 0);
        TreeItem *item = new TreeItem (data, channelParent);
        channelParent->AppendChild (item);
        Item2TreeItem_ [it] = item;
        TreeItem2Item_ [item] = it;
        ItemUnread_ [it] = true;
        endInsertRows ();
    }

    feed.Previous_ = data;
}

