/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "trackerschanger.h"
#include <QMessageBox>
#include <QMainWindow>
#include "core.h"
#include "singletrackerchanger.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			TrackersChanger::TrackersChanger (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				connect (Ui_.Trackers_,
						SIGNAL (currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)),
						this,
						SLOT (currentItemChanged (QTreeWidgetItem*)));
				currentItemChanged (0);
			}
			
			void TrackersChanger::SetTrackers (const std::vector<libtorrent::announce_entry>& trackers)
			{
				Ui_.Trackers_->clear ();
				Q_FOREACH (libtorrent::announce_entry tracker, trackers)
				{
					QStringList strings;
					bool torrent, client, magnet, tex;
					torrent = tracker.source & libtorrent::announce_entry::source_torrent;
					client = tracker.source & libtorrent::announce_entry::source_client;
					magnet = tracker.source & libtorrent::announce_entry::source_magnet_link;
					tex = tracker.source & libtorrent::announce_entry::source_tex;
					strings << QString::fromUtf8 (tracker.url.c_str ())
						<< QString::number (tracker.tier)
						<< tr ("%1 s").arg (tracker.next_announce_in ())
						<< QString::number (tracker.fails)
						<< QString::number (tracker.fail_limit)
						<< (tracker.verified ? tr ("true") : tr ("false"))
						<< (tracker.updating ? tr ("true") : tr ("false"))
						<< (tracker.start_sent ? tr ("true") : tr ("false"))
						<< (tracker.complete_sent ? tr ("true") : tr ("false"))
						<< (torrent ? tr ("true") : tr ("false"))
						<< (client ? tr ("true") : tr ("false"))
						<< (magnet ? tr ("true") : tr ("false"))
						<< (tex ? tr ("true") : tr ("false"));
					Ui_.Trackers_->addTopLevelItem (new QTreeWidgetItem (strings));
				}
				for (int i = 0; i < Ui_.Trackers_->columnCount (); ++i)
					Ui_.Trackers_->resizeColumnToContents (i);
			}
			
			std::vector<libtorrent::announce_entry> TrackersChanger::GetTrackers () const
			{
				std::vector<libtorrent::announce_entry> result;
				for (int i = 0; i < Ui_.Trackers_->topLevelItemCount (); ++i)
				{
					QTreeWidgetItem *item = Ui_.Trackers_->topLevelItem (i);
					libtorrent::announce_entry entry (item->text (0).toStdString ());
					entry.tier = item->text (1).toInt ();
					result.push_back (entry);
				}
				return result;
			}

			void TrackersChanger::currentItemChanged (QTreeWidgetItem *current)
			{
				Ui_.ButtonModify_->setEnabled (current);
				Ui_.ButtonRemove_->setEnabled (current);
			}

			void TrackersChanger::on_ButtonAdd__released ()
			{
				SingleTrackerChanger dia (this);
				if (dia.exec () != QDialog::Accepted)
					return;

				QStringList strings;
				strings << dia.GetTracker ()
					<< QString::number (dia.GetTier ());
				while (strings.size () < Ui_.Trackers_->columnCount ())
					strings << QString ();
				Ui_.Trackers_->addTopLevelItem (new QTreeWidgetItem (strings));
			}

			void TrackersChanger::on_ButtonModify__released ()
			{
				QTreeWidgetItem *current = Ui_.Trackers_->currentItem ();
				if (!current)
					return;

				SingleTrackerChanger dia (this);
				dia.SetTracker (current->text (0));
				dia.SetTier (current->text (1).toInt ());
				if (dia.exec () != QDialog::Accepted)
					return;

				QStringList strings;
				strings << dia.GetTracker ()
					<< QString::number (dia.GetTier ());
				while (strings.size () < Ui_.Trackers_->columnCount ())
					strings << QString ();

				int idx = Ui_.Trackers_->indexOfTopLevelItem (current);
				Ui_.Trackers_->insertTopLevelItem (idx, new QTreeWidgetItem (strings));
				delete Ui_.Trackers_->takeTopLevelItem (idx);
			}

			void TrackersChanger::on_ButtonRemove__released ()
			{
				QTreeWidgetItem *current = Ui_.Trackers_->currentItem ();
				if (!current)
					return;

				if (QMessageBox::question (Core::Instance ()->
								GetProxy ()->GetMainWindow (),
							tr ("Confirm tracker removal"),
							tr ("Are you sure you want to remove the "
								"following tracker:<br />%1")
								.arg (current->text (0)),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
					delete current;
			}
		};
	};
};

