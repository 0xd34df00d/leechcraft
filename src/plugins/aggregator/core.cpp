#include <QtDebug>
#include <QImage>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryFile>
#include <QTimer>
#include <stdexcept>
#include <plugininterface/proxy.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "parserfactory.h"
#include "rss20parser.h"
#include "atom10parser.h"
#include "treeitem.h"
#include "channelsmodel.h"

Core::Core ()
{
    SaveScheduled_ = false;
    ParserFactory::Instance ().Register (&RSS20Parser::Instance ());
    ParserFactory::Instance ().Register (&Atom10Parser::Instance ());
    ItemHeaders_ << tr ("Name") << tr ("Date");

    qRegisterMetaTypeStreamOperators<Feed> ("Feed");
    qRegisterMetaTypeStreamOperators<Item> ("Item");

    ChannelsModel_ = new ChannelsModel (this);
    connect (ChannelsModel_, SIGNAL (channelDataUpdated ()), this, SIGNAL (channelDataUpdated ()));

    TagsCompletionModel_ = new TagsCompletionModel (this);

    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Aggregator");
    int numFeeds = settings.beginReadArray ("Feeds");
    for (int i = 0; i < numFeeds; ++i)
    {
        settings.setArrayIndex (i);
        Feed feed = settings.value ("Feed").value<Feed> ();
        Feeds_ [feed.URL_] = feed;
        ChannelsModel_->AddFeed (feed);
    }
    settings.endArray ();
    TagsCompletionModel_->UpdateTags (settings.value ("GlobalTags", QStringList ("untagged")).toStringList ());

    ActivatedChannel_ = 0;
}

Core& Core::Instance ()
{
    static Core core;
    return core;
}

void Core::Release ()
{
    saveSettings ();
    XmlSettingsManager::Instance ()->Release ();
}

void Core::DoDelayedInit ()
{
    UpdateTimer_ = new QTimer (this);
    UpdateTimer_->start (XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60 * 1000);
    connect (UpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateFeeds ()));
    if (XmlSettingsManager::Instance ()->property ("UpdateOnStartup").toBool ())
        QTimer::singleShot (2000, this, SLOT (updateFeeds ()));

    QTimer *saveTimer = new QTimer (this);
    saveTimer->start (60 * 1000);
    connect (saveTimer, SIGNAL (timeout ()), this, SLOT (scheduleSave ()));
    
    XmlSettingsManager::Instance ()->RegisterObject ("UpdateInterval", this, "updateIntervalChanged");
    UpdateUnreadItemsNumber ();
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
}

void Core::AddFeed (const QString& url, const QStringList& tags)
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
    if (!idd->CouldDownload (url, false))
    {
        emit error (tr ("Could not handle URL %1").arg (url));
        return;
    }
    {
        QTemporaryFile file;
        file.open ();
        DirectDownloadParams params = { url, file.fileName (), true, false };
        PendingJob pj = { PendingJob::RFeedAdded, url, file.fileName (), tags };
        int id = idd->AddJob (params);
        PendingJobs_ [id] = pj;
        file.close ();
    }
}

void Core::RemoveFeed (const QModelIndex& index)
{
    if (!index.isValid ())
        return;
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (index);
    bool shouldChangeChannel = (channel.get () == ActivatedChannel_);
    if (shouldChangeChannel)
        beginRemoveRows (QModelIndex (), 0, ActivatedChannel_->Items_.size () - 1);

    std::vector<boost::shared_ptr<Channel> > channelsToRemove;
    for (QMap<QString, Feed>::iterator i = Feeds_.begin (); i != Feeds_.end (); ++i)
    {
        bool thisOne = false;
        for (int j = 0; j < i.value ().Channels_.size (); ++j)
            if (*i.value ().Channels_ [j] == *channel)
            {
                thisOne = true;
                break;
            }

        if (!thisOne)
            continue;

        for (int j = 0; j < i.value ().Channels_.size (); ++j)
        {
            channelsToRemove.push_back (i.value ().Channels_ [j]);
        }

        Feeds_.erase (i);
        break;
    }
    for (int i = 0; i < channelsToRemove.size (); ++i)
        ChannelsModel_->RemoveChannel (channelsToRemove [i]);

    if (shouldChangeChannel)
        endRemoveRows ();
    
    UpdateUnreadItemsNumber ();
}

void Core::Activated (const QModelIndex& index)
{
    if (!ActivatedChannel_ || ActivatedChannel_->Items_.size () <= index.row ())
        return;

    boost::shared_ptr<Item> item = ActivatedChannel_->Items_ [index.row ()];

    QString URL = item->Link_;
    item->Unread_ = false;
    ChannelsModel_->UpdateChannelData (ActivatedChannel_);
    UpdateUnreadItemsNumber ();
    QDesktopServices::openUrl (QUrl (URL));
}

void Core::FeedActivated (const QModelIndex& index)
{
    QDesktopServices::openUrl (QUrl (ChannelsModel_->GetChannelForIndex (index)->Link_));
}

QString Core::GetDescription (const QModelIndex& index)
{
    if (!ActivatedChannel_ || ActivatedChannel_->Items_.size () <= index.row ())
        return QString ();

    boost::shared_ptr<Item> item = ActivatedChannel_->Items_ [index.row ()];

    item->Unread_ = false;
    ChannelsModel_->UpdateChannelData (ActivatedChannel_);
    UpdateUnreadItemsNumber ();
    return item->Description_;
}

QAbstractItemModel* Core::GetChannelsModel ()
{
    return ChannelsModel_;
}

TagsCompletionModel* Core::GetTagsCompletionModel ()
{
    return TagsCompletionModel_;
}

void Core::UpdateTags (const QStringList& tags)
{
    TagsCompletionModel_->UpdateTags (tags);
}

void Core::MarkItemAsUnread (const QModelIndex& i)
{
    if (!ActivatedChannel_ || !i.isValid ())
        return;

    ActivatedChannel_->Items_ [i.row ()]->Unread_ = true;
    ChannelsModel_->UpdateChannelData (ActivatedChannel_);
    emit dataChanged (index (i.row (), 0), index (i.row (), 1));
    UpdateUnreadItemsNumber ();
}

void Core::MarkChannelAsRead (const QModelIndex& i)
{
    if (!ActivatedChannel_ || !i.isValid ())
        return;

    ChannelsModel_->MarkChannelAsRead (i);
    if (ChannelsModel_->GetChannelForIndex (i).get () == ActivatedChannel_)
        emit dataChanged (index (0, 0), index (ActivatedChannel_->Items_.size () - 1, 1));
    UpdateUnreadItemsNumber ();
}

void Core::MarkChannelAsUnread (const QModelIndex& i)
{
    if (!ActivatedChannel_ || !i.isValid ())
        return;

    ChannelsModel_->MarkChannelAsUnread (i);
    if (ChannelsModel_->GetChannelForIndex (i).get () == ActivatedChannel_)
        emit dataChanged (index (0, 0), index (ActivatedChannel_->Items_.size () - 1, 1));
    UpdateUnreadItemsNumber ();
}

QStringList Core::GetTagsForIndex (int i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (ChannelsModel_->index (i, 0));
    if (channel)
        return channel->Tags_;
    else
        return QStringList ();
}

QString Core::GetChannelLink (const QModelIndex& i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (i);
    if (channel)
        return channel->Link_;
    else
        return tr ("<empty>");
}

QString Core::GetChannelDescription (const QModelIndex& i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (i);
    if (channel)
        return channel->Description_;
    else
        return tr ("<empty>");
}

QString Core::GetChannelAuthor (const QModelIndex& i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (i);
    if (channel)
        return channel->Author_;
    else
        return tr ("<empty>");
}

QString Core::GetChannelLanguage (const QModelIndex& i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (i);
    if (channel)
        return channel->Language_;
    else
        return tr ("<empty>");
}

QPixmap Core::GetChannelPixmap (const QModelIndex& i) const
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (i);
    if (channel)
        return channel->Pixmap_;
    else
        return QPixmap ();
}

void Core::SetTagsForIndex (const QString& tags, const QModelIndex& index)
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (index);
    channel->Tags_ = tags.split (' ');
}

void Core::UpdateFeed (const QModelIndex& index)
{
    boost::shared_ptr<Channel> channel = ChannelsModel_->GetChannelForIndex (index);
    QString url = FindFeedForChannel (channel);
    if (url.isEmpty ())
    {
        qWarning () << Q_FUNC_INFO << "could not found feed for index" << index;
        return;
    }

    QObject *provider = Providers_ ["http"];
    IDirectDownload *idd = qobject_cast<IDirectDownload*> (provider);
    if (!provider || !idd)
    {
        emit error (tr ("Strange, but no suitable provider found"));
        return;
    }
    if (!idd->CouldDownload (url, false))
    {
        emit error (tr ("Could not handle URL %1").arg (url));
        return;
    }

    QTemporaryFile file;
    file.open ();
    DirectDownloadParams params = { url, file.fileName (), true, false };
    PendingJob pj = { PendingJob :: RFeedUpdated, url, file.fileName () };
    int id = idd->AddJob (params);
    PendingJobs_ [id] = pj;
    file.close ();
}

int Core::columnCount (const QModelIndex& parent) const
{
    return ItemHeaders_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || !ActivatedChannel_ || index.row () >= rowCount ())
        return QVariant ();

    if (role == Qt::DisplayRole)
    {
        switch (index.column ())
        {
            case 0:
                return ActivatedChannel_->Items_ [index.row ()]->Title_;
            case 1:
                return ActivatedChannel_->Items_ [index.row ()]->PubDate_;
            default:
                return QVariant ();
        }
    }
    else if (role == Qt::ForegroundRole)
        return ActivatedChannel_->Items_ [index.row ()]->Unread_ ? Qt::red : Qt::black;
    else
        return QVariant ();
}

Qt::ItemFlags Core::flags (const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool Core::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return ItemHeaders_.at (column);
    else
        return QVariant ();
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex& index) const
{
    return QModelIndex ();
}

int Core::rowCount (const QModelIndex& parent) const
{
    if (ActivatedChannel_)
        return ActivatedChannel_->Items_.size ();
    else
        return 0;
}

void Core::currentChannelChanged (const QModelIndex& index)
{
    boost::shared_ptr<Channel> ch = ChannelsModel_->GetChannelForIndex (index);
    if (!ch)
        return;
    ActivatedChannel_ = ch.get ();
    reset ();
}

void Core::scheduleSave ()
{
    if (SaveScheduled_)
        return;
    QTimer::singleShot (500, this, SLOT (saveSettings ()));
}

void Core::handleJobFinished (int id)
{
    bool silent = XmlSettingsManager::Instance ()->property ("BeSilent").toBool ();

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
        if (silent)
            qWarning () << "Downloaded file has null size!";
        else
            emit error (tr ("Downloaded file has null size!"));
        return;
    }
    if (pj.Role_ == PendingJob::RFeedUpdated && !Feeds_.contains (pj.URL_))
    {
        if (silent)
            qWarning () << "Feed with url %1 not found.";
        else
            emit error (tr ("Feed with url %1 not found.").arg (pj.URL_));
        return;
    }
    std::vector<boost::shared_ptr<Channel> > channels;
    if (pj.Role_ != PendingJob::RFeedExternalData)
    {
        QByteArray data = file.readAll ();
        QDomDocument doc;
        QString errorMsg;
        int errorLine, errorColumn;
        if (!doc.setContent (data, true, &errorMsg, &errorLine, &errorColumn))
        {
            if (silent)
                qWarning () << tr ("XML file parse error: %1, line %2, column %3, filename %4").arg (errorMsg).arg (errorLine).arg (errorColumn).arg (pj.Filename_);
            else
                emit error (tr ("XML file parse error: %1, line %2, column %3, filename %4").arg (errorMsg).arg (errorLine).arg (errorColumn).arg (pj.Filename_));
            return;
        }
        if (pj.Role_ == PendingJob::RFeedAdded)
        {
            Feed feed;
            feed.URL_ = pj.URL_;
            Feeds_ [pj.URL_] = feed;
        }

        Parser *parser = ParserFactory::Instance ().Return (doc);
        if (!parser)
        {
            emit error (tr ("Could not find parser to parse file %1").arg (pj.Filename_));
            return;
        }
        file.close ();
        file.remove ();

        channels = parser->Parse (Feeds_ [pj.URL_].Channels_, data);
    }
    QString emitString;
    if (pj.Role_ == PendingJob::RFeedAdded)
    {
        Feeds_ [pj.URL_].Channels_ = channels;
        for (int i = 0; i < channels.size (); ++i)
        {
            channels [i]->Tags_ = pj.Tags_;
            if (QUrl (channels [i]->PixmapURL_).isValid () && !QUrl (channels [i]->PixmapURL_).isRelative ())
            {
                ExternalData data;
                data.Type_ = ExternalData::TImage;
                data.RelatedChannel_ = channels [i];
                QTemporaryFile file;
                file.open ();
                try
                {
                    fetchExternalFile (channels [i]->PixmapURL_, file.fileName ());
                }
                catch (const std::runtime_error& e)
                {
                    qWarning () << Q_FUNC_INFO << e.what ();
                    continue;
                }
                PendingJob2ExternalData_ [channels [i]->PixmapURL_] = data;
                file.close ();
            }
        }
        ChannelsModel_->AddFeed (Feeds_ [pj.URL_]);
        TagsCompletionModel_->UpdateTags (pj.Tags_);
    }
    else if (pj.Role_ == PendingJob::RFeedUpdated)
    {
        ChannelsModel_->Update (channels);
        for (int i = 0; i < channels.size (); ++i)
        {
            int position = -1;
            for (int j = 0; j < Feeds_ [pj.URL_].Channels_.size (); ++j)
                if (*Feeds_ [pj.URL_].Channels_ [j] == *channels [i])
                {
                    position = j;
                    break;
                }

            if (position == -1)
            {
                Feeds_ [pj.URL_].Channels_.push_back (channels [i]);
                emitString += tr ("Added channel \"%1\" (has %2 items)\r\n").arg (channels [i]->Title_).arg (channels [i]->Items_.size ());
            }
            else
            {
                if (channels [i]->Items_.size () - Feeds_ [pj.URL_].Channels_ [position]->Items_.size ())
                    emitString += tr ("Updated channel \"%1\" (%2 new items)\r\n")
                        .arg (channels [i]->Title_)
                        .arg (channels [i]->Items_.size () - Feeds_ [pj.URL_].Channels_ [position]->Items_.size ());

                bool insertedRows = false;
                if (ActivatedChannel_ == Feeds_ [pj.URL_].Channels_ [position].get () && channels [i]->Items_.size () - Feeds_ [pj.URL_].Channels_ [position]->Items_.size ())
                {
                    insertedRows = true;
                    beginInsertRows (QModelIndex (), 0, channels [i]->Items_.size () - Feeds_ [pj.URL_].Channels_ [position]->Items_.size ());
                }
                Feeds_ [pj.URL_].Channels_ [position]->Items_ = channels.at (i)->Items_;
                if (channels.at (i)->LastBuild_.isValid ())
                    Feeds_ [pj.URL_].Channels_.at (position)->LastBuild_ = channels.at (i)->LastBuild_;
                else
                    Feeds_ [pj.URL_].Channels_.at (position)->LastBuild_ = Feeds_ [pj.URL_].Channels_.at (position)->Items_ [0]->PubDate_;
                ChannelsModel_->UpdateChannelData (Feeds_ [pj.URL_].Channels_ [position]);
                if (insertedRows)
                    endInsertRows ();
                emit dataChanged (index (0, 0), index (channels [i]->Items_.size () - 1, 1));


                int ipc = XmlSettingsManager::Instance ()->property ("ItemsPerChannel").toInt ();
                if (Feeds_ [pj.URL_].Channels_ [position]->Items_.size () > ipc)
                {
                    if (ActivatedChannel_ == Feeds_ [pj.URL_].Channels_ [position].get ())
                        beginRemoveRows (QModelIndex (), ipc, ActivatedChannel_->Items_.size ());
                    Feeds_ [pj.URL_].Channels_ [position]->Items_.erase (Feeds_ [pj.URL_].Channels_ [position]->Items_.begin () + ipc,
                            Feeds_ [pj.URL_].Channels_ [position]->Items_.end ());
                    if (ActivatedChannel_ == Feeds_ [pj.URL_].Channels_ [position].get ())
                        endRemoveRows ();

                    ChannelsModel_->UpdateChannelData (Feeds_ [pj.URL_].Channels_ [position]);
                }

                int days = XmlSettingsManager::Instance ()->property ("ItemsMaxAge").toInt ();
                QDateTime current = QDateTime::currentDateTime ();
                int removeFrom = -1;
                for (int j = 0; j < Feeds_ [pj.URL_].Channels_ [position]->Items_.size (); ++j)
                {
                    if (Feeds_ [pj.URL_].Channels_ [position]->Items_ [j]->PubDate_.daysTo (current) > days)
                    {
                        removeFrom = j;
                        break;
                    }
                }
                if (removeFrom == 0)
                    removeFrom = 1;
                if (removeFrom > 0)
                {
                    if (ActivatedChannel_ == Feeds_ [pj.URL_].Channels_ [position].get ())
                        beginRemoveRows (QModelIndex (), removeFrom, ActivatedChannel_->Items_.size ());
                    Feeds_ [pj.URL_].Channels_ [position]->Items_.erase (Feeds_ [pj.URL_].Channels_ [position]->Items_.begin () + removeFrom,
                            Feeds_ [pj.URL_].Channels_ [position]->Items_.end ());
                    if (ActivatedChannel_ == Feeds_ [pj.URL_].Channels_ [position].get ())
                        endRemoveRows ();

                    ChannelsModel_->UpdateChannelData (Feeds_ [pj.URL_].Channels_ [position]);
                }
            }
        }
    }
    else if (pj.Role_ == PendingJob::RFeedExternalData)
    {
        ExternalData data = PendingJob2ExternalData_ [pj.URL_];
        PendingJob2ExternalData_.remove (pj.URL_);
        if (data.RelatedChannel_)
        {
            switch (data.Type_)
            {
                case ExternalData::TImage:
                    data.RelatedChannel_->Pixmap_ = QPixmap::fromImage (QImage (file.fileName ()));
                    ChannelsModel_->UpdateChannelData (data.RelatedChannel_);
                    break;
            }
        }
        else if (data.RelatedFeed_)
        {
        }
    }
    UpdateUnreadItemsNumber ();
    if (!emitString.isEmpty ())
    {
        emitString.prepend ("Aggregator updated:\r\n");
        emit showDownloadMessage (emitString);
    }
    scheduleSave ();
}

void Core::handleJobRemoved (int id)
{
    if (!PendingJobs_.contains (id))
        return;
    PendingJobs_.remove (id);
}

void Core::handleJobError (int id, IDirectDownload::Error)
{
    if (!PendingJobs_.contains (id))
        return;
    PendingJobs_.remove (id);
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
        if (!idd->CouldDownload (urls.at (i), false))
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

void Core::fetchExternalFile (const QString& url, const QString& where)
{
    QObject *provider = Providers_ ["http"];
    IDirectDownload *idd = qobject_cast<IDirectDownload*> (provider);
    if (!provider || !idd)
    {
        emit error (tr ("Strange, but no suitable provider found"));
        throw std::runtime_error ("no suitable provider");
    }
    if (!idd->CouldDownload (url, false))
    {
        emit error (tr ("Could not handle URL %1").arg (url));
        throw std::runtime_error ("could not handle URL");
    }

    DirectDownloadParams params = { url, where, true, false };
    PendingJob pj = { PendingJob :: RFeedExternalData, url, where };
    int id = idd->AddJob (params);
    PendingJobs_ [id] = pj;
}

void Core::saveSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Aggregator");
    settings.beginWriteArray ("Feeds");
    settings.remove ("");
    QList<Feed> feeds = Feeds_.values ();
    for (int i = 0; i < feeds.size (); ++i)
    {
        settings.setArrayIndex (i);
        settings.setValue ("Feed", qVariantFromValue<Feed> (feeds.at (i)));
    }
    settings.endArray ();
    settings.setValue ("GlobalTags", TagsCompletionModel_->GetTags ());
    SaveScheduled_ = false;
}

void Core::updateIntervalChanged ()
{
    UpdateTimer_->setInterval (XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60 * 1000);
}

QString Core::FindFeedForChannel (const boost::shared_ptr<Channel>& channel) const
{
    for (QMap<QString, Feed>::const_iterator i = Feeds_.begin (); i != Feeds_.end (); ++i)
    {
        std::vector<boost::shared_ptr<Channel> >::const_iterator j = std::find (i.value ().Channels_.begin (), i.value ().Channels_.end (), channel);
        if (j != i.value ().Channels_.end ())
            return i.key ();
    }
    return QString ();
}

void Core::UpdateUnreadItemsNumber () const
{
    int result = 0;
    for (QMap<QString, Feed>::const_iterator i = Feeds_.begin (); i != Feeds_.end (); ++i)
        for (int j = 0; j < i.value ().Channels_.size (); ++j)
            for (int k = 0; k < i.value ().Channels_ [j]->Items_.size (); ++k)
                result += i.value ().Channels_ [j]->Items_ [k]->Unread_;
    emit unreadNumberChanged (result);
}

