/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "peersmodel.h"
#include <numeric>
#include <QIcon>
#include <QTimer>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/sll/unreachable.h>
#include "geoip.h"
#include "sessionholder.h"

namespace LC::BitTorrent
{
	std::shared_ptr<GeoIP> PeersModel::GeoIP_ {};

	PeersModel::PeersModel (const SessionHolder& holder, int idx, QObject *parent)
	: QAbstractItemModel { parent }
	, FlagsPath_ { Util::GetSysPath (Util::SysPath::Share, QStringLiteral ("global_icons/flags"), {}) }
	, Headers_
	{
		tr ("IP"),
		tr ("Download rate"),
		tr ("Upload rate"),
		tr ("Downloaded"),
		tr ("Uploaded"),
		tr ("Client"),
		tr ("Available pieces"),
	}
	, Holder_ { holder }
	, Index_ { idx }
	{
		if (!GeoIP_)
			GeoIP_ = std::make_shared<GeoIP> ();

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (update ()));
		timer->start (2000);
		QTimer::singleShot (0,
				this,
				SLOT (update ()));
	}

	int PeersModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant PeersModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		const int i = index.row ();
		const auto& pi = Peers_.at (i);

		if (role == PeerInfoRole)
			return QVariant::fromValue (pi);

		if (index.column () == 0)
		{
			const auto& code = Peers_.at (i).CountryCode_;
			switch (role)
			{
			case Qt::DecorationRole:
				return QIcon (FlagsPath_ + code + ".png");
			case Qt::DisplayRole:
			case SortRole:
				return pi.IP_;
			case Qt::ToolTipRole:
				return code.isEmpty () || code == "--" || code == "00" ?
						QString () :
						code;
			default:
				return {};
			}
		}

		if (role != Qt::DisplayRole && role != SortRole)
			return {};

		const auto isDisplayRole = role == Qt::DisplayRole;

		switch (index.column ())
		{
		case 1:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->payload_down_speed) + tr ("/s");
			else
				return pi.PI_->payload_down_speed;
		case 2:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->payload_up_speed) + tr ("/s");
			else
				return pi.PI_->payload_up_speed;
		case 3:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->total_download);
			else
				return static_cast<qulonglong> (pi.PI_->total_download);
		case 4:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->total_upload);
			else
				return static_cast<qulonglong> (pi.PI_->total_upload);
		case 5:
			return pi.Client_;
		case 6:
			return tr ("%1/%2")
				.arg (pi.RemoteHas_)
				.arg (pi.PI_->num_pieces);
		default:
			return "Unhandled column";
		}

		Util::Unreachable ();
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

	void PeersModel::update ()
	{
		QList<PeerInfo> result;
		std::vector<libtorrent::peer_info> peerInfos;
		const auto& handle = Holder_ [Index_];
		handle.get_peer_info (peerInfos);

		const auto& localPieces = handle.status (libtorrent::torrent_handle::query_pieces).pieces;

		QList<int> ourMissing;
		for (int i = 0, size = localPieces.size (); i < size; ++i)
			if (!localPieces [i])
				ourMissing << i;

		for (size_t i = 0; i < peerInfos.size (); ++i)
		{
			const libtorrent::peer_info& pi = peerInfos [i];

			const int interesting = std::count_if (ourMissing.begin (), ourMissing.end (),
					[&pi] (int idx) { return idx < pi.pieces.size () && pi.pieces [idx]; });

			PeerInfo ppi
			{
				QString::fromStdString (pi.ip.address ().to_string ()),
				QString::fromUtf8 (pi.client.c_str ()),
				interesting,
				GeoIP_->GetCountry (pi.ip.address ()).value_or (QString {}),
				std::make_shared<libtorrent::peer_info> (pi)
			};
			result << ppi;
		}

		Update (result);
	}

	void PeersModel::Clear ()
	{
		if (Peers_.isEmpty ())
			return;

		beginRemoveRows (QModelIndex (), 0, Peers_.size () - 1);
		Peers_.clear ();
		endRemoveRows ();
	}

	void PeersModel::Update (const QList<PeerInfo>& peers)
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
		std::sort (values.begin (), values.end (), std::greater<int> ());
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
