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

    int i = index.row ();
	PeerInfo pi = Peers_.at (i);

    switch (index.column ())
    {
        case 0:
            return pi.IP_;
        case 1:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (pi.DSpeed_) + tr ("/s");
            else if (role == SortRole)
                return pi.DSpeed_;
            else
                return QVariant ();
        case 2:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (pi.USpeed_) + tr ("/s");
            else if (role == SortRole)
                return Peers_.at (i).USpeed_;
            else
                return QVariant ();
        case 3:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (pi.Downloaded_);
            else if (role == SortRole)
                return pi.Downloaded_;
            else
                return QVariant ();
        case 4:
            if (role == Qt::DisplayRole)
                return Proxy::Instance ()->MakePrettySize (pi.Uploaded_);
            else if (role == SortRole)
                return pi.Uploaded_;
            else
                return QVariant ();
        case 5:
            return Peers_.at (i).Client_;
        case 6:
			return tr ("%1, %2 interesting").arg (pi.NumPieces_).arg (pi.RemoteHas_);
        case 7:
            return pi.LastActive_.toString ("mm:ss.zzz");
        case 8:
            return pi.Hashfails_;
        case 9:
            return pi.Failcount_;
        case 10:
            if (pi.DownloadingPiece_ >= 0)
                return tr ("Piece %1, block %2, %3 of %4 bytes").
                        arg (pi.DownloadingPiece_).
                        arg (pi.DownloadingBlock_).
                        arg (pi.DownloadingProgress_).
                        arg (pi.DownloadingTotal_);
            else
                return tr ("Idle");
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

