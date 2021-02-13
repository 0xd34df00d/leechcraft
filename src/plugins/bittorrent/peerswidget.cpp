/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "peerswidget.h"
#include <QAction>
#include <QSortFilterProxyModel>
#include <QTime>
#include <libtorrent/peer_info.hpp>
#include <util/util.h>
#include "peerinfo.h"
#include "peersmodel.h"
#include "addpeerdialog.h"
#include "banpeersdialog.h"
#include "sessionholder.h"
#include "ltutils.h"
#include "types.h"

namespace LC::BitTorrent
{
	PeersWidget::PeersWidget (QWidget *parent)
	: QWidget { parent }
	, PeersSorter_ { new QSortFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		PeersSorter_->setDynamicSortFilter (true);
		PeersSorter_->setSortRole (PeersModel::SortRole);
		Ui_.PeersView_->setModel (PeersSorter_);

		const auto addPeer = new QAction (tr ("Add peer..."), Ui_.PeersView_);
		addPeer->setProperty ("ActionIcon", "list-add-user");
		connect (addPeer,
				&QAction::triggered,
				this,
				[this]
				{
					AddPeerDialog peer;
					if (peer.exec () == QDialog::Accepted)
						AddPeer (GetTorrentHandle (TorrentIdx_), peer.GetIP (), peer.GetPort ());
				});
		Ui_.PeersView_->addAction (addPeer);

		const auto banPeer = new QAction (tr ("Ban peer..."), Ui_.PeersView_);
		banPeer->setProperty ("ActionIcon", "im-ban-user");
		banPeer->setEnabled (false);
		connect (banPeer,
				&QAction::triggered,
				this,
				[this]
				{
					const auto& idx = Ui_.PeersView_->currentIndex ();

					BanPeersDialog ban;
					ban.SetIP (idx.sibling (idx.row (), 0).data ().toString ());
					if (ban.exec () == QDialog::Accepted)
					{
						BanPeers (Holder_->GetSession (), qMakePair (ban.GetStart (), ban.GetEnd ()));
						SaveFilter (Holder_->GetSession ());
					}
				});
		Ui_.PeersView_->addAction (banPeer);

		connect (Ui_.PeersView_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				[this, banPeer] (const QModelIndex& idx)
				{
					UpdateDetails ();
					banPeer->setEnabled (idx.isValid ());
				});
	}

	PeersWidget::~PeersWidget () = default;

	void PeersWidget::SetSessionHolder (const SessionHolder& holder)
	{
		Holder_ = &holder;
	}

	void PeersWidget::SetSelectedTorrent (const QModelIndex& torrent)
	{
		TorrentIdx_ = torrent;

		auto newModel = std::make_unique<PeersModel> (torrent);
		PeersSorter_->setSourceModel (newModel.get ());
		CurrentModel_ = std::move (newModel);
	}

	void PeersWidget::Update ()
	{
		if (CurrentModel_)
			CurrentModel_->Update ();

		UpdateDetails ();
	}

	void PeersWidget::UpdateDetails ()
	{
		const auto& current = Ui_.PeersView_->currentIndex ();
		const auto& p = current.data (PeersModel::PeerInfoRole).value<PeerInfo> ();

		if (!current.isValid () || !p.PI_)
		{
			Ui_.PeerInfo_->clear ();
			Ui_.PeerType_->clear ();
			Ui_.PeerSpeed_->clear ();
			Ui_.PeerPayloadSpeed_->clear ();
			Ui_.PeerPeakSpeeds_->clear ();
			Ui_.PeerDownloaded_->clear ();
			Ui_.PeerRemoteDLSpeed_->clear ();
			Ui_.PeerProgress_->clear ();
			Ui_.PeerIsSeed_->clear ();
			Ui_.PeerLastRequest_->clear ();
			Ui_.PeerLastActive_->clear ();
			Ui_.PeerSendBuffer_->clear ();
			Ui_.PeerReceiveBuffer_->clear ();
			Ui_.PeerNumHashfails_->clear ();
			Ui_.PeerQueues_->clear ();
			Ui_.PeerFailcount_->clear ();
			Ui_.PeerPendingDisk_->clear ();
			Ui_.PeerRTT_->clear ();
			return;
		}

		QStringList sources;
		const auto maxSourcesCount = 5;
		sources.reserve (maxSourcesCount);
		if (p.PI_->source & libtorrent::peer_info::tracker)
			sources << QStringLiteral ("tracker");
		if (p.PI_->source & libtorrent::peer_info::dht)
			sources << QStringLiteral ("DHT");
		if (p.PI_->source & libtorrent::peer_info::pex)
			sources << QStringLiteral ("PEX");
		if (p.PI_->source & libtorrent::peer_info::lsd)
			sources << QStringLiteral ("LSD");
		if (p.PI_->source & libtorrent::peer_info::resume_data)
			sources << QStringLiteral ("resume");

		Ui_.PeerInfo_->setText (tr ("%1 %2 from %3")
				.arg (p.IP_, p.Client_, sources.join (' ')));

		Ui_.PeerType_->setText (p.PI_->connection_type ==
				libtorrent::peer_info::standard_bittorrent ?
				tr ("Standard BitTorrent peer") :
				tr ("Web seed"));

		Ui_.PeerPieces_->SetPieceMap (p.PI_->pieces);

		Ui_.PeerSpeed_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->down_speed),
					  Util::MakePrettySize (p.PI_->up_speed)));

		Ui_.PeerPayloadSpeed_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->payload_down_speed),
					  Util::MakePrettySize (p.PI_->payload_up_speed)));

		Ui_.PeerPeakSpeeds_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->download_rate_peak),
					  Util::MakePrettySize (p.PI_->upload_rate_peak)));

		Ui_.PeerRemoteDLSpeed_->setText (tr ("%1/s")
				.arg (Util::MakePrettySize (p.PI_->remote_dl_rate)));

		Ui_.PeerDownloaded_->setText (tr ("%1 | %2")
				.arg (Util::MakePrettySize (p.PI_->total_download),
					  Util::MakePrettySize (p.PI_->total_upload)));

		if (p.PI_->downloading_piece_index >= 0)
			Ui_.PeerProgress_->setText (tr ("%1 (piece %2, block %3, %4/%5)")
					.arg (p.PI_->progress)
					.arg (p.PI_->downloading_piece_index)
					.arg (p.PI_->downloading_block_index)
					.arg (p.PI_->downloading_progress)
					.arg (p.PI_->downloading_total));
		else
			Ui_.PeerProgress_->setText (QString::number (p.PI_->progress));

		Ui_.PeerIsSeed_->setText (p.PI_->seed ?
				tr ("yes") : tr ("no"));

		const auto& lastRequest = QTime {}.addMSecs (libtorrent::total_milliseconds (p.PI_->last_request));
		Ui_.PeerLastRequest_->setText (tr ("%1 (%n second(s) remaining)",
				"", p.PI_->request_timeout)
				.arg (lastRequest.toString ()));

		const auto& lastActive = QTime {}.addMSecs (libtorrent::total_milliseconds (p.PI_->last_active));
		Ui_.PeerLastActive_->setText (lastActive.toString ());

		if (int sendBuf = p.PI_->send_buffer_size)
			Ui_.PeerSendBuffer_->setText (tr ("%1% of %2")
					.arg (100 * p.PI_->used_send_buffer / sendBuf)
					.arg (sendBuf));
		else
			Ui_.PeerSendBuffer_->setText (tr ("No send buffer"));

		if (int recBuf = p.PI_->receive_buffer_size)
			Ui_.PeerReceiveBuffer_->setText (tr ("%1% of %2")
					.arg (100 * p.PI_->used_receive_buffer / recBuf)
					.arg (recBuf));
		else
			Ui_.PeerReceiveBuffer_->setText (tr ("No receive buffer"));

		Ui_.PeerNumHashfails_->setText (QString::number (p.PI_->num_hashfails));

		Ui_.PeerQueues_->setText (tr ("%1 | %2")
				.arg (p.PI_->download_queue_length)
				.arg (p.PI_->upload_queue_length));

		Ui_.PeerFailcount_->setText (QString::number (p.PI_->failcount));

		Ui_.PeerPendingDisk_->setText (Util::MakePrettySize (p.PI_->pending_disk_bytes));

		Ui_.PeerRTT_->setText (QString::number (p.PI_->rtt));
	}
}
