/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "torrentplugin.h"
#include <QMessageBox>
#include <QUrl>
#include <QTemporaryFile>
#include <QtDebug>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QTranslator>
#include <QTextCodec>
#include <QTimer>
#include <QToolBar>
#include <QHeaderView>
#include <QInputDialog>
#include <libtorrent/session.hpp>
#include <libtorrent/version.hpp>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/tagscompletionmodel.h>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "xmlsettingsmanager.h"
#include "movetorrentfiles.h"
#include "trackerschanger.h"
#include "exportdialog.h"
#include "wizardgenerator.h"
#include "fastspeedcontrolwidget.h"
#include "ipfilterdialog.h"
#include "speedselectoraction.h"

#ifdef AddJob
#undef AddJob
#endif

using LeechCraft::ActionInfo;
using namespace LeechCraft::Util;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{

			void TorrentPlugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (InstallTranslator ("bittorrent"));
				Core::Instance ()->SetProxy (proxy);
				SetupCore ();
				SetupStuff ();

				setActionsEnabled ();
			}

			void TorrentPlugin::SecondInit ()
			{
			}

			TorrentPlugin::~TorrentPlugin ()
			{
			}

			QByteArray TorrentPlugin::GetUniqueID () const
			{
				return "org.LeechCraft.BitTorrent";
			}

			QString TorrentPlugin::GetName () const
			{
				return "BitTorrent";
			}

			QString TorrentPlugin::GetInfo () const
			{
				return tr ("Full-featured BitTorrent client.");
			}

			QStringList TorrentPlugin::Provides () const
			{
				return QStringList ("bittorrent") << "resume" << "remoteable";
			}

			QStringList TorrentPlugin::Needs () const
			{
				return QStringList ();
			}

			QStringList TorrentPlugin::Uses () const
			{
				return QStringList ();
			}

			void TorrentPlugin::SetProvider (QObject*, const QString&)
			{
			}

			void TorrentPlugin::Release ()
			{
				Core::Instance ()->Release ();
				XmlSettingsManager::Instance ()->Release ();
				XmlSettingsDialog_.reset ();
			}

			QIcon TorrentPlugin::GetIcon () const
			{
				return QIcon (":/resources/images/bittorrent.svg");
			}

			qint64 TorrentPlugin::GetDownloadSpeed () const
			{
				return Core::Instance ()->GetOverallStats ().download_rate;
			}

			qint64 TorrentPlugin::GetUploadSpeed () const
			{
				return Core::Instance ()->GetOverallStats ().upload_rate;
			}

			void TorrentPlugin::StartAll ()
			{
				int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
				for (int i = 0; i < numTorrents; ++i)
					Core::Instance ()->ResumeTorrent (i);
				setActionsEnabled ();
			}

			void TorrentPlugin::StopAll ()
			{
				int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
				for (int i = 0; i < numTorrents; ++i)
					Core::Instance ()->PauseTorrent (i);
			}

			EntityTestHandleResult TorrentPlugin::CouldDownload (const Entity& e) const
			{
				return Core::Instance ()->CouldDownload (e);
			}

			int TorrentPlugin::AddJob (Entity e)
			{
				QString suggestedFname;

				if (e.Entity_.canConvert<QUrl> ())
				{
					QUrl resource = e.Entity_.toUrl ();
					if (resource.scheme () == "magnet")
					{
						QString at = XmlSettingsManager::Instance ()->
							property ("AutomaticTags").toString ();
						QStringList tags = e.Additional_ [" Tags"].toStringList ();
						Q_FOREACH (QString tag, Core::Instance ()->GetProxy ()->
								GetTagsManager ()->Split (at))
							tags << Core::Instance ()->GetProxy ()->
								GetTagsManager ()->GetID (tag);

						QList<QPair<QString, QString>> queryItems = resource.queryItems ();
						for (QList<QPair<QString, QString>>::const_iterator i = queryItems.begin (),
								end = queryItems.end (); i != end; ++i)
							if (i->first == "kt")
							{
								QStringList humanReadable = i->second
									.split ('+', QString::SkipEmptyParts);
								Q_FOREACH (QString hr, humanReadable)
									tags += Core::Instance ()->GetProxy ()->
										GetTagsManager ()->GetID (hr);
							}

						return Core::Instance ()->AddMagnet (resource.toString (),
								e.Location_,
								tags,
								e.Parameters_);
					}
					else if (resource.scheme () == "file")
						suggestedFname = resource.toLocalFile ();
				}

				QByteArray entity = e.Entity_.toByteArray ();

				QFile file (suggestedFname);
				if ((!file.exists () ||
						!file.open (QIODevice::ReadOnly)) &&
						Core::Instance ()->IsValidTorrent (entity))
				{
					QTemporaryFile file ("lctemporarybittorrentfile.XXXXXX");
					if (!file.open  ())
						return -1;
					file.write (entity);
					suggestedFname = file.fileName ().toUtf8 ();
					file.setAutoRemove (false);
				}

				AddTorrentDialog_->Reinit ();
				AddTorrentDialog_->SetFilename (suggestedFname);
				if (!e.Location_.isEmpty ())
					AddTorrentDialog_->SetSavePath (e.Location_);

				QString path;
				QStringList tags = e.Additional_ [" Tags"].toStringList ();
				QVector<bool> files;
				QString fname;
				bool tryLive = e.Additional_ ["TryToStreamLive"].toBool ();
				if (e.Parameters_ & FromUserInitiated)
				{
					if (!tags.isEmpty ())
						AddTorrentDialog_->SetTags (tags);

					if (AddTorrentDialog_->exec () == QDialog::Rejected)
						return -1;

					fname = AddTorrentDialog_->GetFilename (),
					path = AddTorrentDialog_->GetSavePath ();
					tryLive = AddTorrentDialog_->GetTryLive ();
					files = AddTorrentDialog_->GetSelectedFiles ();
					tags = AddTorrentDialog_->GetTags ();
					if (AddTorrentDialog_->GetAddType () == Core::Started)
						e.Parameters_ &= ~NoAutostart;
					else
						e.Parameters_ |= NoAutostart;
				}
				else
				{
					fname = suggestedFname;
					path = e.Location_;
					QString at = XmlSettingsManager::Instance ()->
						property ("AutomaticTags").toString ();
					Q_FOREACH (QString tag, Core::Instance ()->GetProxy ()->
							GetTagsManager ()->Split (at))
						tags << Core::Instance ()->GetProxy ()->
							GetTagsManager ()->GetID (tag);
				}
				int result = Core::Instance ()->AddFile (fname,
						path,
						tags,
						tryLive,
						files,
						e.Parameters_);
				setActionsEnabled ();
				file.remove ();
				return result;
			}

			EntityTestHandleResult TorrentPlugin::CouldHandle (const LeechCraft::Entity& e) const
			{
				return Core::Instance ()->CouldHandle (e);
			}

			void TorrentPlugin::Handle (LeechCraft::Entity e)
			{
				Core::Instance ()->Handle (e);
			}

			void TorrentPlugin::KillTask (int id)
			{
				Core::Instance ()->KillTask (id);
			}

			QAbstractItemModel* TorrentPlugin::GetRepresentation () const
			{
				return Core::Instance ();
			}

			void TorrentPlugin::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& si, const QModelIndex&)
			{
				QModelIndex mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
				if (mapped.model () != GetRepresentation ())
					mapped = QModelIndex ();

				Core::Instance ()->SetCurrentTorrent (mapped.row ());
				if (mapped.isValid ())
					TabWidget_->InvalidateSelection ();

				setActionsEnabled ();
			}

			void TorrentPlugin::ImportSettings (const QByteArray& settings)
			{
				XmlSettingsDialog_->MergeXml (settings);
			}

			void TorrentPlugin::ImportData (const QByteArray&)
			{
			}

			QByteArray TorrentPlugin::ExportSettings () const
			{
				return XmlSettingsDialog_->GetXml ().toUtf8 ();
			}

			QByteArray TorrentPlugin::ExportData () const
			{
				return QByteArray ();
			}

			void TorrentPlugin::SetTags (int torrent, const QStringList& tags)
			{
				Core::Instance ()->UpdateTags (tags, torrent);
			}

			std::shared_ptr<XmlSettingsDialog> TorrentPlugin::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			void TorrentPlugin::SetShortcut (const QString& name,
					const QKeySequences_t& shortcuts)
			{
				ShortcutMgr_->SetShortcut (name, shortcuts);
			}

			QMap<QString, ActionInfo> TorrentPlugin::GetActionInfo () const
			{
				return ShortcutMgr_->GetActionInfo ();
			}

			QList<QWizardPage*> TorrentPlugin::GetWizardPages () const
			{
				std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
				return wg->GetPages ();
			}

			QList<QAction*> TorrentPlugin::GetActions (ActionsEmbedPlace place) const
			{
				QList<QAction*> result;

				switch (place)
				{
				case AEPCommonContextMenu:
					result += CreateTorrent_.get ();
					break;
				case AEPToolsMenu:
					result += OpenMultipleTorrents_.get ();
					result += IPFilter_.get ();
					break;
				default:
					break;
				}

				return result;
			}

			QString TorrentPlugin::GetDiagInfoString () const
			{
				return QString ("Built with rb_libtorrent %1 (%2).")
						.arg (LIBTORRENT_VERSION)
						.arg (LIBTORRENT_REVISION);
			}

			void TorrentPlugin::on_OpenTorrent__triggered ()
			{
				AddTorrentDialog_->Reinit ();
				if (AddTorrentDialog_->exec () == QDialog::Rejected)
					return;

				QString filename = AddTorrentDialog_->GetFilename (),
						path = AddTorrentDialog_->GetSavePath ();
				bool tryLive = AddTorrentDialog_->GetTryLive ();
				QVector<bool> files = AddTorrentDialog_->GetSelectedFiles ();
				QStringList tags = AddTorrentDialog_->GetTags ();
				TaskParameters tp = FromUserInitiated;
				if (AddTorrentDialog_->GetAddType () != Core::Started)
					tp |= NoAutostart;
				Core::Instance ()->AddFile (filename,
						path,
						tags,
						tryLive,
						files,
						tp);
				setActionsEnabled ();
			}

			void TorrentPlugin::on_OpenMultipleTorrents__triggered ()
			{
				AddMultipleTorrents dialog (Core::Instance ()->GetProxy ()->GetMainWindow ());
				std::unique_ptr<TagsCompleter> completer (new TagsCompleter (dialog.GetEdit (), this));
				dialog.GetEdit ()->AddSelector ();

				if (dialog.exec () == QDialog::Rejected)
					return;

				TaskParameters tp = FromUserInitiated;
				if (dialog.GetAddType () != Core::Started)
					tp |= NoAutostart;

				QString savePath = dialog.GetSaveDirectory (),
						openPath = dialog.GetOpenDirectory ();
				QDir dir (openPath);
				QStringList names = dir.entryList (QStringList ("*.torrent"));
				QStringList tags = dialog.GetTags ();
				for (int i = 0; i < names.size (); ++i)
				{
					QString name = openPath;
					if (!name.endsWith ('/'))
						name += '/';
					name += names.at (i);
					Core::Instance ()->AddFile (name, savePath, tags, false);
				}
				setActionsEnabled ();
			}

			void TorrentPlugin::on_IPFilter__triggered ()
			{
				IPFilterDialog dia;
				if (dia.exec () != QDialog::Accepted)
					return;

				Core::Instance ()->ClearFilter ();
				QList<QPair<Core::BanRange_t, bool>> filter = dia.GetFilter ();
				QPair<Core::BanRange_t, bool> pair;
				Q_FOREACH (pair, filter)
					Core::Instance ()->BanPeers (pair.first, pair.second);
			}

			void TorrentPlugin::on_CreateTorrent__triggered ()
			{
				std::auto_ptr<NewTorrentWizard> wizard (new NewTorrentWizard (Core::Instance ()->GetProxy ()->GetMainWindow ()));
				if (wizard->exec () == QDialog::Accepted)
					Core::Instance ()->MakeTorrent (wizard->GetParams ());
				setActionsEnabled ();
			}

			void TorrentPlugin::on_RemoveTorrent__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
					if (mapped.isValid () &&
							mapped.model () == Core::Instance ())
						rows << mapped.row ();
				}

				QMessageBox confirm (QMessageBox::Question,
						"LeechCraft BitTorrent",
						tr ("Do you really want to delete %n torrent(s)?", 0, rows.size ()),
						QMessageBox::Cancel,
						Core::Instance ()->GetProxy ()->GetMainWindow ());
				QPushButton *deleteTorrentsButton =
						confirm.addButton (tr ("&Delete"),
								QMessageBox::ActionRole);
				QPushButton *deleteTorrentsAndFilesButton =
						confirm.addButton (tr ("Delete with &files"),
								QMessageBox::ActionRole);
				confirm.setDefaultButton (QMessageBox::Cancel);

				confirm.exec ();

				int roptions = libtorrent::session::none;
				if (confirm.clickedButton () == deleteTorrentsAndFilesButton)
					roptions |= libtorrent::session::delete_files;
				else if (confirm.clickedButton () == deleteTorrentsButton)
					;// do nothing
				else return;

				std::sort (rows.begin (), rows.end (),
						std::greater<int> ());

				Q_FOREACH (int row, rows)
					Core::Instance ()->RemoveTorrent (row, roptions);
				TabWidget_->InvalidateSelection ();
				setActionsEnabled ();
			}

			void TorrentPlugin::on_Resume__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
					Core::Instance ()->ResumeTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
				setActionsEnabled ();
			}

			void TorrentPlugin::on_Stop__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
					Core::Instance ()->PauseTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
				setActionsEnabled ();
			}

			namespace
			{
				std::deque<int> GetSelections (QAbstractItemModel *model,
						QObject *sender)
				{
					QModelIndexList sis;
					try
					{
						sis = Util::GetSummarySelectedRows (sender);
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO
								<< e.what ();
						throw;
					}

					std::deque<int> selections;
					Q_FOREACH (QModelIndex si, sis)
					{
						QModelIndex mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
						if (mapped.model () != model)
							continue;
						selections.push_back (mapped.row ());
					}

					return selections;
				}
			};

			void TorrentPlugin::on_MoveUp__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				std::deque<int> selections;
				try
				{
					selections = GetSelections (GetRepresentation (), sender ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}

				Core::Instance ()->MoveUp (selections);

				QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
						property ("ItemSelectionModel").value<QObject*> ());

				if (sel)
					sel->clearSelection ();

				QItemSelection selection;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex sibling = si.sibling (si.row () - 1, si.column ());
					if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
						continue;

					selection.select (sibling, sibling);
				}

				if (sel)
					sel->select (selection, QItemSelectionModel::Rows |
							QItemSelectionModel::SelectCurrent);
			}

			void TorrentPlugin::on_MoveDown__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				std::deque<int> selections;
				try
				{
					selections = GetSelections (GetRepresentation (), sender ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}

				Core::Instance ()->MoveDown (selections);

				QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
						property ("ItemSelectionModel").value<QObject*> ());

				if (sel)
					sel->clearSelection ();

				QItemSelection selection;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex sibling = si.sibling (si.row () + 1, si.column ());
					if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
						continue;

					selection.select (sibling, sibling);
				}

				if (sel)
					sel->select (selection, QItemSelectionModel::Rows |
							QItemSelectionModel::SelectCurrent);
			}

			void TorrentPlugin::on_MoveToTop__triggered ()
			{
				try
				{
					Core::Instance ()->MoveToTop (GetSelections (GetRepresentation (),
							sender ()));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}
			}

			void TorrentPlugin::on_MoveToBottom__triggered ()
			{
				try
				{
					Core::Instance ()->MoveToBottom (GetSelections (GetRepresentation (),
							sender ()));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}
			}

			void TorrentPlugin::on_ForceReannounce__triggered ()
			{
				try
				{
					Q_FOREACH (int torrent, GetSelections (GetRepresentation (), sender ()))
						Core::Instance ()->ForceReannounce (torrent);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}
			}

			void TorrentPlugin::on_ForceRecheck__triggered ()
			{
				try
				{
					Q_FOREACH (int torrent, GetSelections (GetRepresentation (), sender ()))
						Core::Instance ()->ForceRecheck (torrent);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}
			}

			void TorrentPlugin::on_ChangeTrackers__triggered ()
			{
				QModelIndexList sis;
				try
				{
					sis = Util::GetSummarySelectedRows (sender ());
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}

				std::vector<libtorrent::announce_entry> allTrackers;
				Q_FOREACH (QModelIndex si, sis)
				{
					auto those = Core::Instance ()->GetTrackers (Core::Instance ()->
							GetProxy ()->MapToSource (si).row ());
					std::copy (those.begin (), those.end (),
							std::back_inserter (allTrackers));
				}

				if (allTrackers.empty ())
					allTrackers = Core::Instance ()->
							GetTrackers (Core::Instance ()->
									GetCurrentTorrent ());

				std::stable_sort (allTrackers.begin (), allTrackers.end (),
						[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
							{ return l.url < r.url; });

				auto newLast = std::unique (allTrackers.begin (), allTrackers.end (),
						[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
							{ return l.url == r.url; });

				allTrackers.erase (newLast, allTrackers.end ());

				if (allTrackers.empty ())
					return;

				TrackersChanger changer (Core::Instance ()->
						GetProxy ()->GetMainWindow ());
				changer.SetTrackers (allTrackers);

				if (changer.exec () == QDialog::Accepted)
					Q_FOREACH (QModelIndex si, sis)
						Core::Instance ()->SetTrackers (changer.GetTrackers (),
								Core::Instance ()->GetProxy ()->MapToSource (si).row ());
			}

			void TorrentPlugin::on_MoveFiles__triggered ()
			{
				QString oldDir = Core::Instance ()->GetTorrentDirectory ();
				MoveTorrentFiles mtf (oldDir);
				if (mtf.exec () == QDialog::Rejected)
					return;
				QString newDir = mtf.GetNewLocation ();
				if (oldDir == newDir)
					return;

				if (!Core::Instance ()->MoveTorrentFiles (newDir))
				{
					QString text = tr ("Failed to move torrent's files from %1 to %2")
							.arg (oldDir)
							.arg (newDir);
					emit gotEntity (Util::MakeNotification ("BitTorrent", text, PCritical_));
				}
			}

			void TorrentPlugin::on_MakeMagnetLink__triggered ()
			{
				QString magnet = Core::Instance ()->GetMagnetLink ();
				if (magnet.isEmpty ())
					return;

				QInputDialog *dia = new QInputDialog ();
				dia->setWindowTitle ("LeechCraft");
				dia->setLabelText (tr ("Magnet link:"));
				dia->setAttribute (Qt::WA_DeleteOnClose);
				dia->setInputMode (QInputDialog::TextInput);
				dia->setTextValue (magnet);
				dia->resize (700, dia->height ());
				dia->show ();
			}

			void TorrentPlugin::on_Import__triggered ()
			{
			}

			void TorrentPlugin::on_Export__triggered ()
			{
				ExportDialog dia;
				if (dia.exec () == QDialog::Rejected)
					return;

				bool settings = dia.GetSettings ();
				bool active = dia.GetActive ();
				QString where = dia.GetLocation ();

				Core::Instance ()->Export (where, settings, active);
			}

			void TorrentPlugin::handleFastSpeedComboboxes ()
			{
				int down = DownSelectorAction_->CurrentData ();
				TabWidget_->SetOverallDownloadRateController (down);

				int up = UpSelectorAction_->CurrentData ();
				TabWidget_->SetOverallUploadRateController (up);
			}

			void TorrentPlugin::setActionsEnabled ()
			{
				int torrent = Core::Instance ()->GetCurrentTorrent ();
				bool isValid = false;
				if (torrent != -1)
					isValid = Core::Instance ()->CheckValidity (torrent);
				RemoveTorrent_->setEnabled (isValid);
				Stop_->setEnabled (isValid);
				Resume_->setEnabled (isValid);
				ForceReannounce_->setEnabled (isValid);
			}

			void TorrentPlugin::showError (QString e)
			{
				emit gotEntity (Util::MakeNotification ("BitTorrent", e, PCritical_));
			}

			void TorrentPlugin::SetupCore ()
			{
				ShortcutMgr_ = new Util::ShortcutManager (Core::Instance ()->GetProxy (), this);

				XmlSettingsDialog_.reset (new XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"torrentsettings.xml");

				Core::Instance ()->DoDelayedInit ();

				SetupActions ();
				TabWidget_.reset (new TabWidget (ChangeTrackers_.get ()));
				TorrentSelectionChanged_ = true;
				LastPeersUpdate_.reset (new QTime);
				LastPeersUpdate_->start ();
				AddTorrentDialog_.reset (new AddTorrent (Core::Instance ()->GetProxy ()->GetMainWindow ()));
				connect (Core::Instance (),
						SIGNAL (error (QString)),
						this,
						SLOT (showError (QString)));
				connect (Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (Core::Instance (),
						SIGNAL (taskFinished (int)),
						this,
						SIGNAL (jobFinished (int)));
				connect (Core::Instance (),
						SIGNAL (taskRemoved (int)),
						this,
						SIGNAL (jobRemoved (int)));

				Core::Instance ()->SetWidgets (Toolbar_.get (), TabWidget_.get ());
			}

			void TorrentPlugin::SetupStuff ()
			{
				TagsAddDiaCompleter_.reset (new TagsCompleter (AddTorrentDialog_->GetEdit ()));
				AddTorrentDialog_->GetEdit ()->AddSelector ();

				OverallStatsUpdateTimer_.reset (new QTimer (this));
				connect (OverallStatsUpdateTimer_.get (),
						SIGNAL (timeout ()),
						TabWidget_.get (),
						SLOT (updateTorrentStats ()));
				connect (OverallStatsUpdateTimer_.get (),
						SIGNAL (timeout ()),
						Core::Instance (),
						SLOT (updateRows ()));
				OverallStatsUpdateTimer_->start (2000);

				FastSpeedControlWidget *fsc = new FastSpeedControlWidget ();
				XmlSettingsDialog_->SetCustomWidget ("FastSpeedControl", fsc);
				connect (fsc,
						SIGNAL (speedsChanged ()),
						DownSelectorAction_,
						SLOT (handleSpeedsChanged ()));
				connect (fsc,
						SIGNAL (speedsChanged ()),
						UpSelectorAction_,
						SLOT (handleSpeedsChanged ()));
				XmlSettingsManager::Instance ()->
					RegisterObject ("EnableFastSpeedControl",
						DownSelectorAction_, "handleSpeedsChanged");
				XmlSettingsManager::Instance ()->
					RegisterObject ("EnableFastSpeedControl",
						UpSelectorAction_, "handleSpeedsChanged");

#define _LC_MERGE(a) "Torrent"#a

#define _LC_SINGLE(a) \
				ShortcutMgr_->RegisterAction (_LC_MERGE(a), a.get ());

#define _LC_TRAVERSER(z,i,array) \
				_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
				BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names)

				_LC_EXPANDER ((OpenTorrent_)
						(ChangeTrackers_)
						(CreateTorrent_)
						(OpenMultipleTorrents_)
						(IPFilter_)
						(RemoveTorrent_)
						(Resume_)
						(Stop_)
						(MoveUp_)
						(MoveDown_)
						(MoveToTop_)
						(MoveToBottom_)
						(ForceReannounce_)
						(ForceRecheck_)
						(MoveFiles_)
						(Import_)
						(Export_)
						(MakeMagnetLink_));
			}

			void TorrentPlugin::SetupActions ()
			{
				Toolbar_.reset (new QToolBar ());
				Toolbar_->setWindowTitle ("BitTorrent");

				OpenTorrent_.reset (new QAction (tr ("Open torrent..."),
							Toolbar_.get ()));
				OpenTorrent_->setShortcut (Qt::Key_Insert);
				OpenTorrent_->setProperty ("ActionIcon", "document-open");
				connect (OpenTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_OpenTorrent__triggered ()));

				CreateTorrent_.reset (new QAction (tr ("Create torrent..."),
							Toolbar_.get ()));
				CreateTorrent_->setProperty ("ActionIcon", "document-new");
				connect (CreateTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_CreateTorrent__triggered ()));

				OpenMultipleTorrents_.reset (new QAction (tr ("Open multiple torrents..."),
						Toolbar_.get ()));
				OpenMultipleTorrents_->setProperty ("ActionIcon", "document-open-folder");
				connect (OpenMultipleTorrents_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_OpenMultipleTorrents__triggered ()));

				IPFilter_.reset (new QAction (tr ("IP filter..."),
							Toolbar_.get ()));
				IPFilter_->setProperty ("ActionIcon", "view-filter");
				connect (IPFilter_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_IPFilter__triggered ()));

				RemoveTorrent_.reset (new QAction (tr ("Remove"),
							Toolbar_.get ()));
				RemoveTorrent_->setShortcut (tr ("Del"));
				RemoveTorrent_->setProperty ("ActionIcon", "list-remove");
				connect (RemoveTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_RemoveTorrent__triggered ()));

				Resume_.reset (new QAction (tr ("Resume"),
							Toolbar_.get ()));
				Resume_->setShortcut (tr ("R"));
				Resume_->setProperty ("ActionIcon", "media-playback-start");
				connect (Resume_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Resume__triggered ()));

				Stop_.reset (new QAction (tr ("Pause"),
							Toolbar_.get ()));
				Stop_->setShortcut (tr ("S"));
				Stop_->setProperty ("ActionIcon", "media-playback-pause");
				connect (Stop_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Stop__triggered ()));

				MoveUp_.reset (new QAction (tr ("Move up"),
							Toolbar_.get ()));
				MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
				MoveUp_->setProperty ("ActionIcon", "go-up");
				connect (MoveUp_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveUp__triggered ()));

				MoveDown_.reset (new QAction (tr ("Move down"),
							Toolbar_.get ()));
				MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
				MoveDown_->setProperty ("ActionIcon", "go-down");
				connect (MoveDown_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveDown__triggered ()));

				MoveToTop_.reset (new QAction (tr ("Move to top"),
							Toolbar_.get ()));
				MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
				MoveToTop_->setProperty ("ActionIcon", "go-top");
				connect (MoveToTop_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveToTop__triggered ()));

				MoveToBottom_.reset (new QAction (tr ("Move to bottom"),
							Toolbar_.get ()));
				MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
				MoveToBottom_->setProperty ("ActionIcon", "go-bottom");
				connect (MoveToBottom_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveToBottom__triggered ()));

				ForceReannounce_.reset (new QAction (tr ("Reannounce"),
							Toolbar_.get ()));
				ForceReannounce_->setShortcut (tr ("F"));
				ForceReannounce_->setProperty ("ActionIcon", "network-wireless");
				connect (ForceReannounce_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ForceReannounce__triggered ()));

				ForceRecheck_.reset (new QAction (tr ("Recheck"),
						Toolbar_.get ()));
				ForceRecheck_->setProperty ("ActionIcon", "tools-check-spelling");
				connect (ForceRecheck_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ForceRecheck__triggered ()));

				MoveFiles_.reset (new QAction (tr ("Move files..."),
							Toolbar_.get ()));
				MoveFiles_->setShortcut (tr ("M"));
				MoveFiles_->setProperty ("ActionIcon", "transform-move");
				connect (MoveFiles_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveFiles__triggered ()));

				ChangeTrackers_.reset (new QAction (tr ("Change trackers..."),
							Toolbar_.get ()));
				ChangeTrackers_->setShortcut (tr ("C"));
				ChangeTrackers_->setProperty ("ActionIcon", "view-media-playlist");
				connect (ChangeTrackers_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ChangeTrackers__triggered ()));

				MakeMagnetLink_.reset (new QAction (tr ("Make magnet link..."),
							Toolbar_.get ()));
				MakeMagnetLink_->setProperty ("ActionIcon", "insert-link");
				connect (MakeMagnetLink_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MakeMagnetLink__triggered ()));

				Import_.reset (new QAction (tr ("Import..."),
							Toolbar_.get ()));
				connect (Import_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Import__triggered ()));
				Import_->setProperty ("ActionIcon", "document-import");

				Export_.reset (new QAction (tr ("Export..."),
							Toolbar_.get ()));
				connect (Export_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Export__triggered ()));
				Export_->setProperty ("ActionIcon", "document-export");

				Toolbar_->addAction (OpenTorrent_.get ());
				Toolbar_->addAction (RemoveTorrent_.get ());
				Toolbar_->addSeparator ();
				Toolbar_->addAction (Resume_.get ());
				Toolbar_->addAction (Stop_.get ());
				Toolbar_->addSeparator ();
				Toolbar_->addAction (MoveUp_.get ());
				Toolbar_->addAction (MoveDown_.get ());
				Toolbar_->addAction (MoveToTop_.get ());
				Toolbar_->addAction (MoveToBottom_.get ());
				Toolbar_->addSeparator ();
				Toolbar_->addAction (ForceReannounce_.get ());
				Toolbar_->addAction (ForceRecheck_.get ());
				Toolbar_->addAction (MoveFiles_.get ());
				Toolbar_->addAction (ChangeTrackers_.get ());
				Toolbar_->addAction (MakeMagnetLink_.get ());
				Toolbar_->addSeparator ();
				Toolbar_->addAction (Import_.get ());
				Toolbar_->addAction (Export_.get ());
				Toolbar_->addSeparator ();
				DownSelectorAction_ = new SpeedSelectorAction ("Down", this);
				DownSelectorAction_->handleSpeedsChanged ();
				Toolbar_->addAction (DownSelectorAction_);
				UpSelectorAction_ = new SpeedSelectorAction ("Up", this);
				UpSelectorAction_->handleSpeedsChanged ();
				Toolbar_->addAction (UpSelectorAction_);

				connect (DownSelectorAction_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (handleFastSpeedComboboxes ()));
				connect (UpSelectorAction_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (handleFastSpeedComboboxes ()));

				QMenu *contextMenu = new QMenu (tr ("Torrents actions"));
				contextMenu->addAction (RemoveTorrent_.get ());
				contextMenu->addSeparator ();
				contextMenu->addAction (MoveUp_.get ());
				contextMenu->addAction (MoveDown_.get ());
				contextMenu->addAction (MoveToTop_.get ());
				contextMenu->addAction (MoveToBottom_.get ());
				contextMenu->addSeparator ();
				contextMenu->addAction (ForceReannounce_.get ());
				contextMenu->addAction (ForceRecheck_.get ());
				contextMenu->addAction (MoveFiles_.get ());
				contextMenu->addAction (ChangeTrackers_.get ());
				contextMenu->addAction (MakeMagnetLink_.get ());
				Core::Instance ()->SetMenu (contextMenu);
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_bittorrent, LeechCraft::Plugins::BitTorrent::TorrentPlugin);

