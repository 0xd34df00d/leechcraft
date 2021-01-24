/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "peerstablinker.h"
#include <QTimer>
#include <QSortFilterProxyModel>
#include <util/util.h>
#include "ui_torrenttabwidget.h"
#include "core.h"
#include "peersmodel.h"

namespace LC
{
namespace BitTorrent
{
	PeersTabLinker::PeersTabLinker (Ui::TorrentTabWidget *ui,
			QSortFilterProxyModel *proxy,
			QObject *parent)
	: QObject (parent)
	, Ui_ (ui)
	, ProxyModel_ (proxy)
	{
		connect (Ui_->PeersView_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (handleNewRow (const QModelIndex&)));
	}

	void PeersTabLinker::handleNewRow (const QModelIndex& pi)
	{
		if (!pi.isValid ())
			return;

		Current_ = ProxyModel_->mapToSource (pi);
		update ();
	}

	void PeersTabLinker::update ()
	{
		if (!Current_.isValid ())
		{
			Ui_->PeerInfo_->clear ();
			Ui_->PeerType_->clear ();
			Ui_->PeerSpeed_->clear ();
			Ui_->PeerPayloadSpeed_->clear ();
			Ui_->PeerPeakSpeeds_->clear ();
			Ui_->PeerDownloaded_->clear ();
			Ui_->PeerRemoteDLSpeed_->clear ();
			Ui_->PeerProgress_->clear ();
			Ui_->PeerIsSeed_->clear ();
			Ui_->PeerLastRequest_->clear ();
			Ui_->PeerLastActive_->clear ();
			Ui_->PeerSendBuffer_->clear ();
			Ui_->PeerReceiveBuffer_->clear ();
			Ui_->PeerNumHashfails_->clear ();
			Ui_->PeerQueues_->clear ();
			Ui_->PeerFailcount_->clear ();
			Ui_->PeerPendingDisk_->clear ();
			Ui_->PeerRTT_->clear ();
			return;
		}

		PeerInfo p;
		try
		{
			auto model = static_cast<const PeersModel*> (Current_.model ());
			p = model->GetPeerInfo (Current_);
		}
		catch (...)
		{
			Current_ = QModelIndex ();
			update ();
			return;
		}

		QTimer::singleShot (1000, this, SLOT (update ()));

		QString source;
		if (p.PI_->source & libtorrent::peer_info::tracker)
			source += "tracker ";
		if (p.PI_->source & libtorrent::peer_info::dht)
			source += "DHT ";
		if (p.PI_->source & libtorrent::peer_info::pex)
			source += "PEX ";
		if (p.PI_->source & libtorrent::peer_info::lsd)
			source += "LSD ";
		if (p.PI_->source & libtorrent::peer_info::resume_data)
			source += "resume ";
		Ui_->PeerInfo_->setText (tr ("%1 %2 from %3")
				.arg (p.IP_)
				.arg (p.Client_)
				.arg (source));

		Ui_->PeerType_->setText (p.PI_->connection_type ==
					libtorrent::peer_info::standard_bittorrent ?
				tr ("Standard BitTorrent peer") :
				tr ("Web seed"));

		Ui_->PeerPieces_->setPieceMap (p.PI_->pieces);

		Ui_->PeerSpeed_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->down_speed))
				.arg (Util::MakePrettySize (p.PI_->up_speed)));

		Ui_->PeerPayloadSpeed_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->payload_down_speed))
				.arg (Util::MakePrettySize (p.PI_->payload_up_speed)));

		Ui_->PeerPeakSpeeds_->setText (tr ("%1/s | %2/s")
				.arg (Util::MakePrettySize (p.PI_->download_rate_peak))
				.arg (Util::MakePrettySize (p.PI_->upload_rate_peak)));

		Ui_->PeerRemoteDLSpeed_->setText (tr ("%1/s")
				.arg (Util::MakePrettySize (p.PI_->remote_dl_rate)));

		Ui_->PeerDownloaded_->setText (tr ("%1 | %2")
				.arg (Util::MakePrettySize (p.PI_->total_download))
				.arg (Util::MakePrettySize (p.PI_->total_upload)));

		if (p.PI_->downloading_piece_index >= 0)
			Ui_->PeerProgress_->setText (tr ("%1 (piece %2, block %3, %4/%5)")
					.arg (p.PI_->progress)
					.arg (p.PI_->downloading_piece_index)
					.arg (p.PI_->downloading_block_index)
					.arg (p.PI_->downloading_progress)
					.arg (p.PI_->downloading_total));
		else
			Ui_->PeerProgress_->setText (QString ("%1")
					.arg (p.PI_->progress));

		Ui_->PeerIsSeed_->setText (p.PI_->seed ?
				tr ("yes") : tr ("no"));

		const auto& lastRequest = QTime {}.addMSecs (libtorrent::total_milliseconds (p.PI_->last_request));
		Ui_->PeerLastRequest_->setText (tr ("%1 (%n second(s) remaining)",
					"", p.PI_->request_timeout)
				.arg (lastRequest.toString ()));

		const auto& lastActive = QTime {}.addMSecs (libtorrent::total_milliseconds (p.PI_->last_active));
		Ui_->PeerLastActive_->setText (lastActive.toString ());

		int sendBuf = p.PI_->send_buffer_size;
		if (sendBuf)
			Ui_->PeerSendBuffer_->setText (tr ("%1% of %2")
					.arg (100 * p.PI_->used_send_buffer / sendBuf)
					.arg (sendBuf));
		else
			Ui_->PeerSendBuffer_->setText (tr ("No send buffer"));

		int recBuf = p.PI_->receive_buffer_size;
		if (recBuf)
			Ui_->PeerReceiveBuffer_->setText (tr ("%1% of %2")
					.arg (100 * p.PI_->used_receive_buffer / recBuf)
					.arg (recBuf));
		else
			Ui_->PeerReceiveBuffer_->setText (tr ("No receive buffer"));

		Ui_->PeerNumHashfails_->setText (QString::number (p.PI_->num_hashfails));

		Ui_->PeerQueues_->setText (tr ("%1 | %2")
				.arg (p.PI_->download_queue_length)
				.arg (p.PI_->upload_queue_length));

		Ui_->PeerFailcount_->setText (QString::number (p.PI_->failcount));

		Ui_->PeerPendingDisk_->setText (Util::MakePrettySize (p.PI_->pending_disk_bytes));

		Ui_->PeerRTT_->setText (QString::number (p.PI_->rtt));
	}
}
}
