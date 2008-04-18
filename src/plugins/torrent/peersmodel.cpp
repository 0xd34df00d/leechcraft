#include <QtDebug>
#include <numeric>
#include <plugininterface/treeitem.h>
#include "core.h"
#include "peersmodel.h"

PeersModel::PeersModel (QObject *parent)
: QAbstractItemModel (parent)
, CurrentTorrent_ (-1)
{
    Headers_ << tr ("IP")
        << tr ("Drate")
        << tr ("Urate")
        << tr ("Downloaded")
        << tr ("Uploaded")
        << tr ("Client")
        << tr ("Available pieces")
        << tr ("LB")
        << tr ("Last active")
        << tr ("Hashfails")
        << tr ("Failcount")
        << tr ("Progress");
}

PeersModel::~PeersModel ()
{
}

int PeersModel::columnCount (const QModelIndex& index) const
{
    return Headers_.size ();
}

QVariant PeersModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || role != Qt::DisplayRole)
        return QVariant ();

    const std::vector<bool>* localPieces = Core::Instance ()->GetLocalPieces (CurrentTorrent_);
    switch (index.column ())
    {
        case 0:
            return Peers_.at (index.row ()).IP_;
        case 1:
            return Peers_.at (index.row ()).DSpeed_;
        case 2:
            return Peers_.at (index.row ()).USpeed_;
        case 3:
            return Peers_.at (index.row ()).Downloaded_;
        case 4:
            return Peers_.at (index.row ()).Uploaded_;
        case 5:
            return Peers_.at (index.row ()).Client_;
        case 6:
            if (localPieces)
            {
                int remoteNum = std::accumulate (Peers_.at (index.row ()).Pieces_.begin (), Peers_.at (index.row ()).Pieces_.end (), 0),
                    remoteHasWeDont = 0;
                for (int j = 0; j < localPieces->size (); ++j)
                    remoteHasWeDont += (Peers_.at (index.row ()).Pieces_ [j] && !(*localPieces) [j]);
                return tr ("%1, %2 we don't have").arg (remoteNum).arg (remoteHasWeDont);
            }
            else
                return "";
        case 7:
            return static_cast<quint64> (Peers_.at (index.row ()).LoadBalancing_);
        case 8:
            return Peers_.at (index.row ()).LastActive_.toString ("mm:ss.zzz");
        case 9:
            return Peers_.at (index.row ()).Hashfails_;
        case 10:
            return Peers_.at (index.row ()).Failcount_;
        case 11:
            if (Peers_.at (index.row ()).DownloadingPiece_ >= 0)
                return tr ("Piece %1, block %2, %3 of %4 bytes").
                        arg (Peers_.at (index.row ()).DownloadingPiece_).
                        arg (Peers_.at (index.row ()).DownloadingBlock_).
                        arg (Peers_.at (index.row ()).DownloadingProgress_).
                        arg (Peers_.at (index.row ()).DownloadingTotal_);
            else
                return tr ("Not downloading");
        default:
            return "Unhandled column";
    }
}

Qt::ItemFlags PeersModel::flags (const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool PeersModel::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QModelIndex PeersModel::index (int row, int column, const QModelIndex&) const
{
    if (!hasIndex (row, column))
        return QModelIndex ();

    return createIndex (row, column);
} 

QVariant PeersModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (role != Qt::DisplayRole || orient != Qt::Horizontal)
        return QVariant ();

    return Headers_.at (column);
}

QModelIndex PeersModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int PeersModel::rowCount (const QModelIndex& index) const
{
    Peers_.size ();
}

void PeersModel::Clear ()
{
    if (!Peers_.size ())
        return;

    beginRemoveRows (QModelIndex (), 0, Peers_.size () - 1);
    Peers_.clear ();
    endRemoveRows ();
}

void PeersModel::Update (const QList<PeerInfo>& peers, int torrent)
{
    CurrentTorrent_ = torrent;
    QList<PeerInfo> peers2insert;
    QMap<QString, int> IP2position;

    for (int i = 0; i < Peers_.size (); ++i)
        IP2position [Peers_.at (i).IP_] = i;

    for (int i = 0; i < peers.size (); ++i)
    {
        PeerInfo pi = peers.at (i);

        int found = false;
        for (int j = 0; j < Peers_.size (); ++j)
            if (Peers_.at (j).IP_ == pi.IP_)
            {
                found = true;
                Peers_ [j] = pi;
                IP2position.remove (pi.IP_);
                emit dataChanged (index (j, 1), index (j, 11));
                break;
            }

        if (found)
            continue;

        peers2insert << pi;
    }

    for (QMap<QString, int>::const_iterator i = IP2position.begin (); i != IP2position.end (); ++i)
    {
        beginRemoveRows (QModelIndex (), i.value (), i.value ());
        Peers_.removeAt (i.value ());
        endRemoveRows ();
    }

    if (peers2insert.size ())
    {
        beginInsertRows (QModelIndex (), Peers_.size (), Peers_.size () + peers2insert.size () - 1);
        Peers_ += peers2insert;
        endInsertRows ();
    }
}

