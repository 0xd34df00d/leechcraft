/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "listactions.h"
#include <QAction>
#include <QDir>
#include <QFile>
#include <QFuture>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <libtorrent/magnet_uri.hpp>
#include <util/sll/prelude.h>
#include <util/sll/views.h>
#include "addmagnetdialog.h"
#include "addmultipletorrents.h"
#include "addtorrent.h"
#include "core.h"
#include "ltutils.h"
#include "movetorrentfiles.h"
#include "newtorrentparams.h"
#include "newtorrentwizard.h"
#include "trackerschanger.h"
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	namespace
	{
		bool CheckExists (const QString& torrentPath, const QDir& saveDir)
		{
			QFile torrentFile { torrentPath };
			if (!torrentFile.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< torrentPath
						<< torrentFile.errorString ();
				return false;
			}

			const auto& torrentData = torrentFile.readAll ();
			try
			{

				libtorrent::torrent_info torrent { torrentData.constData (), static_cast<int> (torrentData.size ()) };
				const auto& files = torrent.files ();
				switch (files.num_files ())
				{
				case 0:
					return false;
				case 1:
					return saveDir.exists (QString::fromStdString (files.file_name (0).to_string ()));
				default:
				{
					const auto& dirName = QString::fromStdString (files.name ());
					return !dirName.isEmpty () && saveDir.exists (dirName);
				}
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse"
						<< torrentPath
						<< e.what ();
				return false;
			}
		}

		template<typename Dia, typename ParentF, typename Handler>
		auto RunDialog (const ParentF& parentF, Handler&& f)
		{
			return [&parentF, f]
			{
				const auto dia = new Dia { parentF () };
				dia->setAttribute (Qt::WA_DeleteOnClose);
				dia->show ();
				QObject::connect (dia,
						&QDialog::accepted,
						[dia, f] { f (dia); });
			};
		}
	}

	ListActions::ListActions (const Dependencies& deps, QWidget *parent)
	: QObject { parent }
	, D_ { deps }
	, Toolbar_ { new QToolBar { "BitTorrent" } }
	{
		OpenTorrent_ = Toolbar_->addAction (tr ("Open torrent..."), this,
				RunDialog<AddTorrent> (D_.GetPreferredParent_,
						[this] (AddTorrent *dia)
						{
							TaskParameters tp = FromUserInitiated;
							if (dia->GetAddType () != AddState::Started)
								tp |= NoAutostart;
							Core::Instance ()->AddFile (dia->GetFilename (),
									dia->GetSavePath (),
									dia->GetTags (),
									dia->GetTryLive (),
									dia->GetSelectedFiles (),
									tp);

							SetActionsEnabled ();
						}));
		OpenTorrent_->setShortcut (Qt::Key_Insert);
		OpenTorrent_->setProperty ("ActionIcon", "document-open");

		AddMagnet_ = Toolbar_->addAction (tr ("Add magnet link..."), this,
				RunDialog<AddMagnetDialog> (D_.GetPreferredParent_,
						[this] (AddMagnetDialog *dia)
						{
							Core::Instance ()->AddMagnet (dia->GetLink (),
									dia->GetPath (),
									dia->GetTags ());
							SetActionsEnabled ();
						}));
		AddMagnet_->setProperty ("ActionIcon", "document-open-remote");

		OpenMultipleTorrents_ = Toolbar_->addAction (tr ("Open multiple torrents..."), this,
				RunDialog<AddMultipleTorrents> (D_.GetPreferredParent_,
						[this] (AddMultipleTorrents *dia)
						{
							TaskParameters tp = FromUserInitiated;
							if (!dia->ShouldAddAsStarted ())
								tp |= NoAutostart;

							const auto& savePath = dia->GetSaveDirectory ();
							const auto& openPath = dia->GetOpenDirectory ();
							const auto& tags = dia->GetTags ();
							const QDir saveDir { savePath };
							for (const auto& torrentName : QDir { openPath }.entryList ({ "*.torrent" }))
							{
								auto torrentPath = openPath;
								if (!torrentPath.endsWith ('/'))
									torrentPath += '/';
								torrentPath += torrentName;

								if (dia->OnlyIfExists ())
								{
									bool torrentExists = CheckExists (torrentPath, saveDir);
									qDebug () << torrentName << torrentExists;
									if (!torrentExists)
										continue;
								}

								Core::Instance ()->AddFile (torrentPath, savePath, tags, false);
							}

							SetActionsEnabled ();
						}));
		OpenMultipleTorrents_->setProperty ("ActionIcon", "document-open-folder");

		RemoveTorrent_ = Toolbar_->addAction (tr ("Remove"), this,
				[this]
				{
					QMessageBox confirm (QMessageBox::Question,
							QStringLiteral ("BitTorrent"),
							tr ("Do you really want to delete %n torrent(s)?", nullptr, CurSelection_.size ()),
							QMessageBox::Cancel);
					confirm.addButton (tr ("&Delete"), QMessageBox::DestructiveRole);
					auto withFilesButton = confirm.addButton (tr ("Delete with &files"), QMessageBox::DestructiveRole);
					confirm.setDefaultButton (QMessageBox::Cancel);

					confirm.exec ();

					if (confirm.clickedButton () == confirm.button (QMessageBox::Cancel))
						return;

					auto rows = GetSelectedHandlesIndices ();

					bool withFiles = confirm.clickedButton () == withFilesButton;
					std::sort (rows.begin (), rows.end (), std::greater<> ());
					for (int row : rows)
						Core::Instance ()->RemoveTorrent (row, withFiles);

					SetActionsEnabled ();
				});
		RemoveTorrent_->setShortcut (tr ("Del"));
		RemoveTorrent_->setProperty ("ActionIcon", "list-remove");

		Toolbar_->addSeparator ();

		CreateTorrent_ = Toolbar_->addAction (tr ("Create torrent..."), this,
				RunDialog<NewTorrentWizard> (D_.GetPreferredParent_,
						[] (NewTorrentWizard *dia) { Core::Instance ()->MakeTorrent (dia->GetParams ()); }));
		CreateTorrent_->setProperty ("ActionIcon", "document-new");

		Toolbar_->addSeparator ();

		Resume_ = Toolbar_->addAction (tr ("Resume"), this,
				[this]
				{
					for (int row : GetSelectedHandlesIndices ())
						Core::Instance ()->ResumeTorrent (row);
				});
		Resume_->setShortcut (tr ("R"));
		Resume_->setProperty ("ActionIcon", "media-playback-start");

		Stop_ = Toolbar_->addAction (tr ("Pause"), this,
				[this]
				{
					for (int row : GetSelectedHandlesIndices ())
						Core::Instance ()->PauseTorrent (row);
				});
		Stop_->setShortcut (tr ("S"));
		Stop_->setProperty ("ActionIcon", "media-playback-pause");

		Toolbar_->addSeparator ();

		MoveUp_ = Toolbar_->addAction (tr ("Move up"), this,
				[this] { Core::Instance ()->MoveUp (GetSelectedHandlesIndices ()); });
		MoveUp_->setShortcut (Qt::CTRL | Qt::Key_Up);
		MoveUp_->setProperty ("ActionIcon", "go-up");

		MoveDown_ = Toolbar_->addAction (tr ("Move down"), this,
				[this] { Core::Instance ()->MoveDown (GetSelectedHandlesIndices ()); });
		MoveDown_->setShortcut (Qt::CTRL | Qt::Key_Down);
		MoveDown_->setProperty ("ActionIcon", "go-down");

		MoveToTop_ = Toolbar_->addAction (tr ("Move to top"), this,
				[this] { Core::Instance ()->MoveToTop (GetSelectedHandlesIndices ()); });
		MoveToTop_->setShortcut (Qt::CTRL | Qt::SHIFT | Qt::Key_Up);
		MoveToTop_->setProperty ("ActionIcon", "go-top");

		MoveToBottom_ = Toolbar_->addAction (tr ("Move to bottom"), this,
				[this] { Core::Instance ()->MoveToBottom (GetSelectedHandlesIndices ()); });
		MoveToBottom_->setShortcut (Qt::CTRL | Qt::SHIFT | Qt::Key_Down);
		MoveToBottom_->setProperty ("ActionIcon", "go-bottom");

		Toolbar_->addSeparator ();

		ForceReannounce_ = Toolbar_->addAction (tr ("Reannounce"), this,
				[this]
				{
					for (int torrent : GetSelectedHandlesIndices ())
						Core::Instance ()->ForceReannounce (torrent);
				});
		ForceReannounce_->setShortcut (tr ("F"));
		ForceReannounce_->setProperty ("ActionIcon", "network-wireless");

		ForceRecheck_ = Toolbar_->addAction (tr ("Recheck"), this,
				[this]
				{
					for (int torrent : GetSelectedHandlesIndices ())
						Core::Instance ()->ForceRecheck (torrent);
				});
		ForceRecheck_->setProperty ("ActionIcon", "tools-check-spelling");

		MoveFiles_ = Toolbar_->addAction (tr ("Move files..."), this,
				[this]
				{
					const auto currentRows = GetSelectedHandlesIndices ();
					if (currentRows.empty() )
						return;

					const auto oldDirs = Util::Map (currentRows,
							[] (const int row) { return Core::Instance ()->GetTorrentDirectory (row); });

					const auto mtf = new MoveTorrentFiles { oldDirs, D_.GetPreferredParent_ () };
					mtf->setAttribute (Qt::WA_DeleteOnClose);
					mtf->show ();

					connect (mtf,
							&QDialog::accepted,
							this,
							[=, this]
							{
								const auto newDir = mtf->GetNewLocation ();

								for (auto it : Util::Views::Zip (currentRows, oldDirs))
								{
									if (it.second == newDir)
										continue;

									if (!Core::Instance ()->MoveTorrentFiles (newDir, it.first))
										QMessageBox::critical (D_.GetPreferredParent_ (),
												QStringLiteral ("BitTorrent"),
												tr ("Failed to move torrent's files from %1 to %2.")
														.arg (it.second, newDir));
								}
							});
				});
		MoveFiles_->setShortcut (tr ("M"));
		MoveFiles_->setProperty ("ActionIcon", "transform-move");

		ChangeTrackers_ = Toolbar_->addAction (tr ("Change trackers..."), this,
				[this]
				{
					std::vector<libtorrent::announce_entry> allTrackers;
					for (const auto& index : CurSelection_)
					{
						const auto& handle = GetTorrentHandle (index);
						auto those = handle.trackers ();
						std::move (those.begin (), those.end (), std::back_inserter (allTrackers));
					}

					std::stable_sort (allTrackers.begin (), allTrackers.end (),
							Util::ComparingBy (&libtorrent::announce_entry::url));

					auto newLast = std::unique (allTrackers.begin (), allTrackers.end (),
							[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
								{ return l.url == r.url; });

					allTrackers.erase (newLast, allTrackers.end ());

					if (allTrackers.empty ())
						return;

					auto changer = new TrackersChanger { allTrackers, D_.GetPreferredParent_ () };
					changer->setAttribute (Qt::WA_DeleteOnClose);
					changer->show ();
					connect (changer,
							&QDialog::accepted,
							this,
							[=, this]
							{
								const auto& trackers = changer->GetTrackers ();
								for (const auto& index : CurSelection_)
								{
									const auto& handle = GetTorrentHandle (index);
									handle.replace_trackers (trackers);
									handle.force_reannounce ();
								}
							});
				});
		ChangeTrackers_->setShortcut (tr ("C"));
		ChangeTrackers_->setProperty ("ActionIcon", "view-media-playlist");
		// TODO
		// Ui_.Tabs_->SetChangeTrackersAction (ChangeTrackers_);

		MakeMagnetLink_ = Toolbar_->addAction (tr ("Make magnet link..."), this,
				[this]
				{
					const auto& handle = GetTorrentHandle (CurIdx_);
					const auto magnet = QString::fromStdString (libtorrent::make_magnet_uri (handle));
					if (magnet.isEmpty ())
						return;

					const auto dia = new QInputDialog {};
					dia->setWindowTitle (QStringLiteral ("BitTorrent"));
					dia->setLabelText (tr ("Magnet link:"));
					dia->setAttribute (Qt::WA_DeleteOnClose);
					dia->setInputMode (QInputDialog::TextInput);
					dia->setTextValue (magnet);
					dia->resize (700, dia->height ());
					dia->show ();
				});
		MakeMagnetLink_->setProperty ("ActionIcon", "insert-link");

		Toolbar_->addSeparator ();

		IPFilter_ = Toolbar_->addAction (tr ("IP filter..."), this, [this] { RunIPFilterDialog (D_.Session_); });
		IPFilter_->setProperty ("ActionIcon", "view-filter");

		SetActionsEnabled ();
	}

	QToolBar* ListActions::GetToolbar () const
	{
		return Toolbar_;
	}

	void ListActions::SetCurrentIndex (const QModelIndex& index)
	{
		if (CurIdx_ == index)
			return;

		CurIdx_ = index;
		SetActionsEnabled ();
	}

	void ListActions::SetCurrentSelection (const QModelIndexList& selection)
	{
		CurSelection_ = selection;
	}

	QMenu* ListActions::MakeContextMenu () const
	{
		auto menu = new QMenu;

		menu->addActions ({ Resume_, Stop_, MakeMagnetLink_, RemoveTorrent_ });
		menu->addSeparator ();
		menu->addActions ({ MoveToTop_, MoveUp_, MoveDown_, MoveToBottom_ });
		menu->addSeparator ();
		menu->addActions ({ ForceReannounce_, ForceRecheck_, MoveFiles_, ChangeTrackers_ });

		menu->setAttribute (Qt::WA_DeleteOnClose);

		return menu;
	}

	void ListActions::SetActionsEnabled ()
	{
		const auto& actions =
		{
			Resume_, Stop_, MakeMagnetLink_, RemoveTorrent_,
			MoveUp_, MoveDown_, MoveToTop_, MoveToBottom_,
			ForceReannounce_, ForceRecheck_, MoveFiles_, ChangeTrackers_
		};

		const bool enable = CurIdx_.isValid ();
		for (auto action : actions)
			action->setEnabled (enable);
	}

	QList<libtorrent::torrent_handle> ListActions::GetSelectedHandles () const
	{
		return Util::Map (CurSelection_, &GetTorrentHandle);
	}

	QList<int> ListActions::GetSelectedHandlesIndices () const
	{
		return Util::Map (CurSelection_, [] (const QModelIndex& idx) { return idx.data (Roles::HandleIndex).toInt (); });
	}
}
