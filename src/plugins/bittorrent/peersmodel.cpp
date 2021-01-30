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
#include <libtorrent/peer_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/sll/unreachable.h>
#include <util/sll/qtutil.h>
#include "geoip.h"
#include "sessionholder.h"

namespace LC::BitTorrent
{
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
	}

	int PeersModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant PeersModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const int i = index.row ();
		const auto& pi = Peers_.at (i);

		if (role == PeerInfoRole)
			return QVariant::fromValue (pi);

		if (index.column () == ColumnIP)
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
				return code.isEmpty () || code == "--"_ql || code == "00"_ql ?
						QString {} :
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
		case ColumnDownloadRate:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->payload_down_speed) + tr ("/s");
			else
				return pi.PI_->payload_down_speed;
		case ColumnUploadRate:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->payload_up_speed) + tr ("/s");
			else
				return pi.PI_->payload_up_speed;
		case ColumnDownloaded:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->total_download);
			else
				return static_cast<qulonglong> (pi.PI_->total_download);
		case ColumnUploaded:
			if (isDisplayRole)
				return Util::MakePrettySize (pi.PI_->total_upload);
			else
				return static_cast<qulonglong> (pi.PI_->total_upload);
		case ColumnClient:
			return pi.Client_;
		case ColumnPieces:
			return tr ("%1/%2")
				.arg (pi.RemoteHas_)
				.arg (pi.PI_->num_pieces);
		default:
			return {};
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
			return {};

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

	void PeersModel::Update ()
	{
		std::vector<libtorrent::peer_info> peerInfos;
		const auto& handle = Holder_ [Index_];
		if (!handle.is_valid ())
			return;

		handle.get_peer_info (peerInfos);

		const auto& localPieces = handle.status (libtorrent::torrent_handle::query_pieces).pieces;

		QList<int> ourMissing;
		for (int i = 0, size = localPieces.size (); i < size; ++i)
			if (!localPieces [i])
				ourMissing << i;

		auto& geoIP = GeoIP::Instance ();

		QList<PeerInfo> peers;
		peers.reserve (peerInfos.size ());
		for (const auto& pi : peerInfos)
		{
			const int interesting = std::count_if (ourMissing.begin (), ourMissing.end (),
					[&pi] (int idx) { return idx < pi.pieces.size () && pi.pieces [idx]; });

			peers.push_back ({
					QString::fromStdString (pi.ip.address ().to_string ()),
					QString::fromUtf8 (pi.client.c_str ()),
					interesting,
					geoIP.GetCountry (pi.ip.address ()).value_or (QString {}),
					std::make_shared<libtorrent::peer_info> (pi)
				});
		}

		Update (peers);
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
		QHash<QString, int> ip2position;
		ip2position.reserve (Peers_.size ());
		for (int i = 0; i < Peers_.size (); ++i)
			ip2position [Peers_.at (i).IP_] = i;

		QList<PeerInfo> peers2insert;
		for (const auto& pi : peers)
		{
			auto pos = ip2position.find (pi.IP_);
			if (pos != ip2position.end ())
			{
				Peers_ [pos.value ()] = pi;
				auto chin1 = index (pos.value (), 0);
				auto chin2 = index (pos.value (), columnCount () - 1);
				emit dataChanged (chin1, chin2);
				ip2position.erase (pos);
			}
			else
				peers2insert << pi;
		}

		auto values = ip2position.values ();
		std::sort (values.begin (), values.end (), std::greater<> ());
		for (int val : values)
		{
			beginRemoveRows (QModelIndex (), val, val);
			Peers_.removeAt (val);
			endRemoveRows ();
		}

		if (!peers2insert.isEmpty ())
		{
			beginInsertRows ({},
					Peers_.size (),
					Peers_.size () + peers2insert.size () - 1);
			Peers_ += peers2insert;
			endInsertRows ();
		}
	}
}
