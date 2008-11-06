#include <QtDebug>
#include <numeric>
#include <plugininterface/proxy.h>
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

int PeersModel::columnCount (const QModelIndex&) const
{
    return Headers_.size ();
}

QVariant PeersModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid () || (role != Qt::DisplayRole && role != SortRole))
        return QVariant ();

	libtorrent::bitfield localPieces = Core::Instance ()->GetLocalPieces ();

    int i = index.row ();

    switch (index.column ())
    {
        case 0:
            return Peers_.at (index.row ()).IP_;
        case 1:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).DSpeed_) + tr ("/s");
            else if (role == SortRole)
                return Peers_.at (i).DSpeed_;
            else
                return QVariant ();
        case 2:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).USpeed_) + tr ("/s");
            else if (role == SortRole)
                return Peers_.at (i).USpeed_;
            else
                return QVariant ();
        case 3:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).Downloaded_);
            else if (role == SortRole)
                return Peers_.at (i).Downloaded_;
            else
                return QVariant ();
        case 4:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (Peers_.at (i).Uploaded_);
            else if (role == SortRole)
                return Peers_.at (i).Uploaded_;
            else
                return QVariant ();
        case 5:
            return Peers_.at (i).Client_;
        case 6:
			{
				int remoteNum = std::accumulate (Peers_.at (i).Pieces_.begin (), Peers_.at (i).Pieces_.end (), 0),
					remoteHasWeDont = 0;
				for (size_t j = 0; j < localPieces.size (); ++j)
					remoteHasWeDont += (Peers_.at (i).Pieces_ [j] && !localPieces [j]);
				return tr ("%1, %2 we don't have").arg (remoteNum).arg (remoteHasWeDont);
			}
        case 7:
            return static_cast<quint64> (Peers_.at (i).LoadBalancing_);
        case 8:
            return Peers_.at (i).LastActive_.toString ("mm:ss.zzz");
        case 9:
            return Peers_.at (i).Hashfails_;
        case 10:
            return Peers_.at (i).Failcount_;
        case 11:
            if (Peers_.at (i).DownloadingPiece_ >= 0)
                return tr ("Piece %1, block %2, %3 of %4 bytes").
                        arg (Peers_.at (i).DownloadingPiece_).
                        arg (Peers_.at (i).DownloadingBlock_).
                        arg (Peers_.at (i).DownloadingProgress_).
                        arg (Peers_.at (i).DownloadingTotal_);
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

int PeersModel::rowCount (const QModelIndex&) const
{
    return Peers_.size ();
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
	if (torrent != CurrentTorrent_)
	{
		CurrentTorrent_ = torrent;
		Peers_.clear ();

		Update (peers, torrent);
	}
	else
	{
		QHash<QString, int> IP2position;
		for (int i = 0; i < Peers_.size (); ++i)
			IP2position [Peers_.at (i).IP_] = i;

		int psize = peers.size ();
		QList<PeerInfo> peers2insert;
		for (int i = 0; i < psize; ++i)
		{
			const PeerInfo& pi = peers.at (i);
			QHash<QString, int>::iterator pos = IP2position.find (pi.IP_);
			if (pos != IP2position.end ())
			{
				Peers_ [pos.value ()] = pi;
				IP2position.erase (pos);
			}
			else
				peers2insert << pi;
		}

		QList<int> values = IP2position.values ();
		for (int i = 0; i < values.size (); ++i)
			Peers_.removeAt (values.at (i));
		Peers_ += peers2insert;

		reset ();
	}
}

