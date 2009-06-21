#include <QtDebug>
#include <numeric>
#include <plugininterface/proxy.h>
#include "core.h"
#include "peersmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			using LeechCraft::Util::Proxy;
			
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
					<< tr ("Available pieces");
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
				if (!index.isValid () ||
						(role != Qt::DisplayRole && role != SortRole))
					return QVariant ();
			
				int i = index.row ();
				PeerInfo pi = Peers_.at (i);
			
				switch (index.column ())
				{
					case 0:
						return pi.IP_;
					case 1:
						if (role == Qt::DisplayRole)
							return Proxy::Instance ()->
								MakePrettySize (pi.PI_->payload_down_speed) + tr ("/s");
						else if (role == SortRole)
							return pi.PI_->payload_down_speed;
						else
							return QVariant ();
					case 2:
						if (role == Qt::DisplayRole)
							return Proxy::Instance ()->
								MakePrettySize (pi.PI_->payload_up_speed) + tr ("/s");
						else if (role == SortRole)
							return pi.PI_->payload_up_speed;
						else
							return QVariant ();
					case 3:
						if (role == Qt::DisplayRole)
							return Proxy::Instance ()->
								MakePrettySize (pi.PI_->total_download);
						else if (role == SortRole)
							return static_cast<qulonglong> (pi.PI_->total_download);
						else
							return QVariant ();
					case 4:
						if (role == Qt::DisplayRole)
							return Proxy::Instance ()->
								MakePrettySize (pi.PI_->total_upload);
						else if (role == SortRole)
							return static_cast<qulonglong> (pi.PI_->total_upload);
						else
							return QVariant ();
					case 5:
						return pi.Client_;
					case 6:
						return tr ("%1/%2")
							.arg (pi.RemoteHas_)
							.arg (pi.PI_->num_pieces);
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

			const PeerInfo& PeersModel::GetPeerInfo (const QModelIndex& index) const
			{
				if (index.row () >= Peers_.size ())
					throw std::runtime_error ("Index too large");
				return Peers_.at (index.row ());
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
					reset ();
			
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
							QModelIndex chin1 = index (pos.value (), 0);
							QModelIndex chin2 = index (pos.value (), columnCount () - 1);
							emit dataChanged (chin1, chin2);
							IP2position.erase (pos);
						}
						else
							peers2insert << pi;
					}
			
					QList<int> values = IP2position.values ();
					std::sort (values.begin (), values.end (),
							std::greater<int> ());
					for (int i = 0; i < values.size (); ++i)
					{
						int val = values.at (i);
						beginRemoveRows (QModelIndex (), val, val);
						Peers_.removeAt (val);
						endRemoveRows ();
					}

					if (peers2insert.size ())
					{
						beginInsertRows (QModelIndex (),
								Peers_.size (),
								Peers_.size () + peers2insert.size () - 1);
						Peers_ += peers2insert;
						endInsertRows ();
					}
				}
			}
			
		};
	};
};

