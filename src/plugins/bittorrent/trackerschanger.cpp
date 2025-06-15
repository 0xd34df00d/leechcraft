/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trackerschanger.h"
#include <QMessageBox>
#include <QMainWindow>
#include <libtorrent/announce_entry.hpp>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/qtutil.h>
#include "singletrackerchanger.h"

namespace LC::BitTorrent
{
	namespace
	{
		QString HashVersionToString (libtorrent::protocol_version version)
		{
			switch (version)
			{
			case libtorrent::protocol_version::V1:
				return "SHA-1"_qs;
			case libtorrent::protocol_version::V2:
				return "SHA-256"_qs;
			default:
				return {};
			}
		}

		QString GetSources (uint8_t source)
		{
			using enum libtorrent::announce_entry::tracker_source;

			QStringList sources;
			if (source & source_torrent)
				sources << TrackersChanger::tr ("torrent");
			if (source & source_client)
				sources << TrackersChanger::tr ("user");
			if (source & source_magnet_link)
				sources << TrackersChanger::tr ("magnet");
			if (source & source_tex)
				sources << TrackersChanger::tr ("tracker exchange");

			return sources.join (", "_ql);
		}
	}

	TrackersChanger::TrackersChanger (const std::vector<libtorrent::announce_entry>& trackers, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		auto enableButtons = [this] (QTreeWidgetItem *item)
		{
			Ui_.ButtonModify_->setEnabled (item);
			Ui_.ButtonRemove_->setEnabled (item);
		};
		connect (Ui_.Trackers_,
				&QTreeWidget::currentItemChanged,
				enableButtons);
		enableButtons (nullptr);

		connect (Ui_.ButtonAdd_,
				&QPushButton::released,
				this,
				&TrackersChanger::AddTracker);
		connect (Ui_.ButtonModify_,
				&QPushButton::released,
				this,
				&TrackersChanger::ModifyTracker);
		connect (Ui_.ButtonRemove_,
				&QPushButton::released,
				this,
				&TrackersChanger::RemoveTracker);

		QList<QTreeWidgetItem*> items;
		for (const auto& tracker : trackers)
		{
			const auto showBool = [] (bool val) { return val ? tr ("yes") : tr ("no"); };

			const auto now = std::chrono::system_clock::now ();
			for (const auto& endpoint : tracker.endpoints)
				for (const auto hashVersion : { libtorrent::protocol_version::V1, libtorrent::protocol_version::V2 })
				{
					const auto& infohash = endpoint.info_hashes [hashVersion];
					if (infohash.next_announce < now)
						continue;

					items << new QTreeWidgetItem
					{
						{
							HashVersionToString (hashVersion),
							QString::fromStdString (tracker.url),
							QString::number (tracker.tier),
							tr ("%1 s").arg (libtorrent::total_seconds (infohash.next_announce - now)),
							tr ("%1 of %2").arg (infohash.fails).arg (tracker.fail_limit),
							showBool (tracker.verified),
							showBool (infohash.updating),
							showBool (infohash.start_sent),
							showBool (infohash.complete_sent),
							GetSources (tracker.source),
						}
					};
				}
		}
		Ui_.Trackers_->addTopLevelItems (items);
		Ui_.Trackers_->header ()->resizeSections (QHeaderView::ResizeToContents);
	}

	std::vector<libtorrent::announce_entry> TrackersChanger::GetTrackers () const
	{
		const int count = Ui_.Trackers_->topLevelItemCount ();
		std::vector<libtorrent::announce_entry> result;
		result.reserve (count);
		for (int i = 0; i < count; ++i)
		{
			auto item = Ui_.Trackers_->topLevelItem (i);
			auto& entry = result.emplace_back (item->text (0).toStdString ());
			entry.tier = item->text (1).toInt ();
		}
		return result;
	}

	void TrackersChanger::AddTracker ()
	{
		SingleTrackerChanger dia { this };
		if (dia.exec () != QDialog::Accepted)
			return;

		QStringList strings
		{
			dia.GetTracker (),
			QString::number (dia.GetTier ())
		};
		while (strings.size () < Ui_.Trackers_->columnCount ())
			strings << QString {};
		Ui_.Trackers_->addTopLevelItem (new QTreeWidgetItem { strings });
	}

	void TrackersChanger::ModifyTracker ()
	{
		const auto current = Ui_.Trackers_->currentItem ();
		if (!current)
			return;

		SingleTrackerChanger dia { this };
		dia.SetTracker (current->text (0));
		dia.SetTier (current->text (1).toInt ());
		if (dia.exec () != QDialog::Accepted)
			return;

		current->setText (0, dia.GetTracker ());
		current->setText (1, QString::number (dia.GetTier ()));
	}

	void TrackersChanger::RemoveTracker ()
	{
		const auto current = Ui_.Trackers_->currentItem ();
		if (!current)
			return;

		auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		if (QMessageBox::question (rootWM->GetPreferredWindow (),
					tr ("Confirm tracker removal"),
					tr ("Are you sure you want to remove the following tracker:<br />%1")
						.arg (current->text (0)),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			delete current;
	}
}
