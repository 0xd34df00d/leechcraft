/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionstatskeeper.h"
#include <chrono>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_stats.hpp>
#include "alertdispatcher.h"

namespace LC::BitTorrent
{
	namespace
	{
		struct MetricIndices
		{
			int RecvBytes_ = libtorrent::find_metric_idx ("net.recv_bytes");
			int SentBytes_ = libtorrent::find_metric_idx ("net.sent_bytes");
			int RecvIPOverheadBytes_ = libtorrent::find_metric_idx ("net.recv_ip_overhead_bytes");
			int SentIPOverheadBytes_ = libtorrent::find_metric_idx ("net.sent_ip_overhead_bytes");
			int RecvPayloadBytes_ = libtorrent::find_metric_idx ("net.recv_payload_bytes");
			int SentPayloadBytes_ = libtorrent::find_metric_idx ("net.sent_payload_bytes");
			int RecvTrackerBytes_ = libtorrent::find_metric_idx ("net.recv_tracker_bytes");
			int SentTrackerBytes_ = libtorrent::find_metric_idx ("net.sent_tracker_bytes");
			int RecvFailedBytes_ = libtorrent::find_metric_idx ("net.recv_failed_bytes");
			int RecvRedundantBytes_ = libtorrent::find_metric_idx ("net.recv_redundant_bytes");
			int DHTBytesIn_ = libtorrent::find_metric_idx ("dht.dht_bytes_in");
			int DHTBytesOut_ = libtorrent::find_metric_idx ("dht.dht_bytes_out");
			int DHTNodes_ = libtorrent::find_metric_idx ("dht.dht_nodes");
			int DHTTorrents_ = libtorrent::find_metric_idx ("dht.dht_torrents");
			int NumPeers_ = libtorrent::find_metric_idx ("peer.num_peers_connected");
		};

		const MetricIndices& GetMetricIndices ()
		{
			static const MetricIndices indices;
			return indices;
		}

		struct CounterReader
		{
			libtorrent::span<const std::int64_t> Counters_;

			std::int64_t operator() (int idx) const
			{
				return idx >= 0 && idx < Counters_.size () ? Counters_ [idx] : 0;
			}
		};

		SessionStats::Symmetric<int> Rate (const SessionStats::Symmetric<int64_t>& now,
				const SessionStats::Symmetric<int64_t>& before,
				double seconds)
		{
			return
			{
				static_cast<int> ((now.Down_ - before.Down_) / seconds),
				static_cast<int> ((now.Up_ - before.Up_) / seconds),
			};
		}
	}

	SessionStatsKeeper::SessionStatsKeeper (const libtorrent::session& session, AlertDispatcher& dispatcher)
	: Session_ { session }
	{
		dispatcher.RegisterHandler ([this] (const libtorrent::session_stats_alert& a)
				{
					HandleStats (a);
					return false;
				});
		dispatcher.Swallow (libtorrent::session_stats_header_alert::alert_type, false);
	}

	const SessionStats& SessionStatsKeeper::GetStats () const
	{
		return Stats_;
	}

	void SessionStatsKeeper::HandleStats (const libtorrent::session_stats_alert& a)
	{
		const auto& idx = GetMetricIndices ();
		const CounterReader get { a.counters () };

		const Sample sample
		{
			.Time_ = a.timestamp (),
			.Total_ = { get (idx.RecvBytes_) + get (idx.RecvIPOverheadBytes_), get (idx.SentBytes_) + get (idx.SentIPOverheadBytes_) },
			.IPOverheadTotal_ = { get (idx.RecvIPOverheadBytes_), get (idx.SentIPOverheadBytes_) },
			.DHTTotal_ = { get (idx.DHTBytesIn_), get (idx.DHTBytesOut_) },
			.TrackerTotal_ = { get (idx.RecvTrackerBytes_), get (idx.SentTrackerBytes_) },
		};

		const auto seconds = Prev_ ?
				std::chrono::duration<double> (sample.Time_ - Prev_->Time_).count () :
				0.0;
		const auto rate = [&] (const SessionStats::Symmetric<int64_t> Sample::*field)
		{
			return seconds > 0 ?
					Rate (sample.*field, (*Prev_).*field, seconds) :
					SessionStats::Symmetric<int> {};
		};

		const auto dhtRunning = Session_.is_dht_running ();
		Stats_ =
		{
			.Rate_ = rate (&Sample::Total_),
			.IPOverheadRate_ = rate (&Sample::IPOverheadTotal_),
			.DHTRate_ = rate (&Sample::DHTTotal_),
			.TrackerRate_ = rate (&Sample::TrackerTotal_),

			.Total_ = sample.Total_,
			.IPOverheadTotal_ = sample.IPOverheadTotal_,
			.DHTTotal_ = sample.DHTTotal_,
			.TrackerTotal_ = sample.TrackerTotal_,
			.PayloadTotal_ = { get (idx.RecvPayloadBytes_), get (idx.SentPayloadBytes_) },

			.NumPeers_ = static_cast<int> (get (idx.NumPeers_)),
			.DHTNodes_ = dhtRunning ? static_cast<int> (get (idx.DHTNodes_)) : 0,
			.DHTTorrents_ = dhtRunning ? static_cast<int> (get (idx.DHTTorrents_)) : 0,

			.TotalFailedBytes_ = get (idx.RecvFailedBytes_),
			.TotalRedundantBytes_ = get (idx.RecvRedundantBytes_),
		};

		Prev_ = sample;
	}
}
