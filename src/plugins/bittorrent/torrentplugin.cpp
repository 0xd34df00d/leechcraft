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
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "xmlsettingsmanager.h"
#include "movetorrentfiles.h"
#include "representationmodel.h"
#include "trackerschanger.h"
#include "exportdialog.h"
#include "wizardgenerator.h"
#include "fastspeedcontrolwidget.h"
#include "ipfilterdialog.h"

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
				SetupTorrentView ();
				SetupStuff ();

				connect (proxy->GetTreeViewReemitter (),
						SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&,
								QTreeView*)),
						this,
						SLOT (handleItemSelected (const QModelIndex&)));
			
				setActionsEnabled ();
			}

			void TorrentPlugin::SecondInit ()
			{
			}
			
			TorrentPlugin::~TorrentPlugin ()
			{
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
			
			bool TorrentPlugin::CouldDownload (const DownloadEntity& e) const
			{
				return Core::Instance ()->CouldDownload (e);
			}
			
			int TorrentPlugin::AddJob (DownloadEntity e)
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
				
						QList<QPair<QString, QString> > queryItems = resource.queryItems ();
						for (QList<QPair<QString, QString> >::const_iterator i = queryItems.begin (),
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

			void TorrentPlugin::KillTask (int id)
			{
				Core::Instance ()->KillTask (id);
			}
			
			QAbstractItemModel* TorrentPlugin::GetRepresentation () const
			{
				return FilterModel_.get ();
			}
			
			void TorrentPlugin::handleItemSelected (const QModelIndex& si)
			{
				QModelIndex item = Core::Instance ()->GetProxy ()->MapToSource (si);
				if (item.model () != GetRepresentation ())
					item = QModelIndex ();

				QModelIndex mapped = item.isValid () ?
					FilterModel_->mapToSource (item) : QModelIndex ();
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
			
			boost::shared_ptr<XmlSettingsDialog> TorrentPlugin::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
			
#define _LC_MERGE(a) EA##a
			
#define _LC_SINGLE(a) \
				case _LC_MERGE(a): \
					a->setShortcut (shortcut); \
					break;
			
#define _LC_TRAVERSER(z,i,array) \
				_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))
			
#define _LC_EXPANDER(Names) \
				switch (name) \
				{ \
					BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names) \
				}
			void TorrentPlugin::SetShortcut (int name,
					const QKeySequence& shortcut)
			{
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
			
#define _L(a) result [EA##a] = ActionInfo (a->text (), \
					a->shortcut (), a->icon ())
			QMap<int, ActionInfo> TorrentPlugin::GetActionInfo () const
			{
				QMap<int, ActionInfo> result;
				_L (OpenTorrent_);
				_L (ChangeTrackers_);
				_L (CreateTorrent_);
				_L (OpenMultipleTorrents_);
				_L (IPFilter_);
				_L (RemoveTorrent_);
				_L (Resume_);
				_L (Stop_);
				_L (MoveUp_);
				_L (MoveDown_);
				_L (MoveToTop_);
				_L (MoveToBottom_);
				_L (ForceReannounce_);
				_L (ForceRecheck_);
				_L (MoveFiles_);
				_L (Import_);
				_L (Export_);
				_L (MakeMagnetLink_);
				return result;
			}
#undef _L

			QList<QWizardPage*> TorrentPlugin::GetWizardPages () const
			{
				std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
				return wg->GetPages ();
			}

			QList<QAction*> TorrentPlugin::GetActions () const
			{
				QList<QAction*> result;
				result += CreateTorrent_.get ();
				result += OpenMultipleTorrents_.get ();
				result += IPFilter_.get ();
				return result;
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
				AddMultipleTorrents dialog;
				std::auto_ptr<TagsCompleter> completer (new TagsCompleter (dialog.GetEdit (), this));
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
				QList<QPair<Core::BanRange_t, bool> > filter = dia.GetFilter ();
				QPair<Core::BanRange_t, bool> pair;
				Q_FOREACH (pair, filter)
					Core::Instance ()->BanPeers (pair.first, pair.second);
			}
			
			void TorrentPlugin::on_CreateTorrent__triggered ()
			{
				std::auto_ptr<NewTorrentWizard> wizard (new NewTorrentWizard ());
				if (wizard->exec () == QDialog::Accepted)
					Core::Instance ()->MakeTorrent (wizard->GetParams ());
				setActionsEnabled ();
			}
			
			void TorrentPlugin::on_RemoveTorrent__triggered ()
			{
				QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
					if (mapped.isValid () &&
							mapped.model () == FilterModel_.get ())
						rows << mapped.row ();
				}

				if (QMessageBox::question (0,
							tr ("LeechCraft"),
							tr ("Do you really want to delete %n torrents?", 0, rows.size ()),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					return;

				std::sort (rows.begin (), rows.end (),
						std::greater<int> ());
			
				Q_FOREACH (int row, rows)
					Core::Instance ()->RemoveTorrent (row);
				TabWidget_->InvalidateSelection ();
				setActionsEnabled ();
			}

			void TorrentPlugin::on_Resume__triggered ()
			{
				QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
					Core::Instance ()->ResumeTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
				setActionsEnabled ();
			}
			
			void TorrentPlugin::on_Stop__triggered ()
			{
				QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				QList<int> rows;
				Q_FOREACH (QModelIndex si, sis)
					Core::Instance ()->PauseTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
				setActionsEnabled ();
			}
			
			namespace
			{
				std::deque<int> GetSelections (QAbstractItemModel *model)
				{
					QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
					if (!tree)
						throw std::runtime_error ("No current view");

					QItemSelectionModel *sel = tree->selectionModel ();
					if (!sel)
						throw std::runtime_error ("No selection model");

					QModelIndexList sis = sel->selectedRows ();
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
				QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				std::deque<int> selections;
				try
				{
					selections = GetSelections (GetRepresentation ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}

				Core::Instance ()->MoveUp (selections);

				sel->clearSelection ();
				QItemSelection selection;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex sibling = si.sibling (si.row () - 1, si.column ());
					if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
						continue;
					
					selection.select (sibling, sibling);
				}
				sel->select (selection, QItemSelectionModel::Rows |
						QItemSelectionModel::SelectCurrent);
			}
			
			void TorrentPlugin::on_MoveDown__triggered ()
			{
				QTreeView *tree = Core::Instance ()->GetProxy ()->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				std::deque<int> selections;
				try
				{
					selections = GetSelections (GetRepresentation ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return;
				}

				Core::Instance ()->MoveDown (selections);

				sel->clearSelection ();
				QItemSelection selection;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex sibling = si.sibling (si.row () + 1, si.column ());
					if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
						continue;

					selection.select (sibling, sibling);
				}
				sel->select (selection, QItemSelectionModel::Rows |
						QItemSelectionModel::SelectCurrent);
			}
			
			void TorrentPlugin::on_MoveToTop__triggered ()
			{
				try
				{
					Core::Instance ()->MoveToTop (GetSelections (GetRepresentation ()));
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
					Core::Instance ()->MoveToBottom (GetSelections (GetRepresentation ()));
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
					Q_FOREACH (int torrent, GetSelections (GetRepresentation ()))
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
					Q_FOREACH (int torrent, GetSelections (GetRepresentation ()))
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
				QStringList trackers = Core::Instance ()->GetTrackers ();
				TrackersChanger changer;
				changer.SetTrackers (trackers);
				if (changer.exec () == QDialog::Accepted)
					Core::Instance ()->SetTrackers (changer.GetTrackers ());
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
					QMessageBox::warning (0,
							tr ("LeechCraft"),
							tr ("Failed to move torrent's files from %1 to %2")
							.arg (oldDir)
							.arg (newDir));
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

			void TorrentPlugin::handleSpeedsChanged ()
			{
				DownSelector_->clear ();
				UpSelector_->clear ();

				if (!XmlSettingsManager::Instance ()->
						property ("EnableFastSpeedControl").toBool ())
				{
					DownSelectorAction_->setVisible (false);
					UpSelectorAction_->setVisible (false);
					return;
				}

				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Torrent");
				settings.beginGroup ("FastSpeedControl");
				int num = settings.beginReadArray ("Values");
				for (int i = 0; i < num; ++i)
				{
					settings.setArrayIndex (i);
					int dv = settings.value ("DownValue").toInt ();
					int uv = settings.value ("UpValue").toInt ();
					DownSelector_->addItem (tr ("%1 KiB/s").arg (dv), dv);
					UpSelector_->addItem (tr ("%1 KiB/s").arg (uv), uv);
				}
				settings.endArray ();
				settings.endGroup ();

				DownSelector_->addItem (QString::fromUtf8 ("\u221E"), 0);
				UpSelector_->addItem (QString::fromUtf8 ("\u221E"), 0);

				DownSelectorAction_->setVisible (true);
				UpSelectorAction_->setVisible (true);

				DownSelector_->setCurrentIndex (DownSelector_->count () - 1);
				UpSelector_->setCurrentIndex (UpSelector_->count () - 1);
			}

			void TorrentPlugin::handleFastSpeedComboboxes ()
			{
				int down = DownSelector_->itemData (DownSelector_->currentIndex ()).toInt ();
				TabWidget_->SetOverallDownloadRateController (down);

				int up = UpSelector_->itemData (UpSelector_->currentIndex ()).toInt ();
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
				qWarning () << e;
				QMessageBox::warning (0,
						tr ("LeechCraft"),
						e);
			}
			
			void TorrentPlugin::doLogMessage (const QString& msg)
			{
				emit log (msg);
			}
			
			void TorrentPlugin::SetupCore ()
			{
				XmlSettingsDialog_.reset (new XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"torrentsettings.xml");

				Core::Instance ()->DoDelayedInit ();

				TabWidget_.reset (new TabWidget ());
				SetupActions ();
				TorrentSelectionChanged_ = true;
				LastPeersUpdate_.reset (new QTime);
				LastPeersUpdate_->start ();
				AddTorrentDialog_.reset (new AddTorrent ());
				connect (Core::Instance (),
						SIGNAL (error (QString)),
						this,
						SLOT (showError (QString)));
				connect (Core::Instance (),
						SIGNAL (logMessage (const QString&)),
						this,
						SLOT (doLogMessage (const QString&)));
				connect (Core::Instance (),
						SIGNAL (torrentFinished (const QString&)),
						this,
						SIGNAL (downloadFinished (const QString&)));
				connect (Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
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
			
			void TorrentPlugin::SetupTorrentView ()
			{
				FilterModel_.reset (new RepresentationModel);
				FilterModel_->setSourceModel (Core::Instance ());
				connect (Core::Instance (),
						SIGNAL (dataChanged (const QModelIndex&,
								const QModelIndex&)),
						FilterModel_.get (),
						SLOT (invalidate ()));
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
						FilterModel_.get (),
						SLOT (invalidate ()));
				OverallStatsUpdateTimer_->start (500);

				FastSpeedControlWidget *fsc = new FastSpeedControlWidget ();
				XmlSettingsDialog_->SetCustomWidget ("FastSpeedControl", fsc);
				connect (fsc,
						SIGNAL (speedsChanged ()),
						this,
						SLOT (handleSpeedsChanged ()));
				XmlSettingsManager::Instance ()->
					RegisterObject ("EnableFastSpeedControl",
						this, "handleSpeedsChanged");
				handleSpeedsChanged ();
			}
			
			void TorrentPlugin::SetupActions ()
			{
				Toolbar_.reset (new QToolBar ());
				Toolbar_->setWindowTitle (tr ("BitTorrent"));
			
				OpenTorrent_.reset (new QAction (tr ("Open torrent..."),
							Toolbar_.get ()));
				OpenTorrent_->setShortcut (Qt::Key_Insert);
				OpenTorrent_->setProperty ("ActionIcon", "torrent_addjob");
				connect (OpenTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_OpenTorrent__triggered ()));
			
				CreateTorrent_.reset (new QAction (tr ("Create torrent..."),
							Toolbar_.get ()));
				CreateTorrent_->setShortcut (tr ("N"));
				CreateTorrent_->setProperty ("ActionIcon", "torrent_create");
				connect (CreateTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_CreateTorrent__triggered ()));
			
				OpenMultipleTorrents_.reset (new QAction (tr ("Open multiple torrents..."),
						Toolbar_.get ()));
				OpenMultipleTorrents_->setProperty ("ActionIcon", "torrent_addmulti");
				connect (OpenMultipleTorrents_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_OpenMultipleTorrents__triggered ()));

				IPFilter_.reset (new QAction (tr ("IP filter..."),
							Toolbar_.get ()));
				IPFilter_->setProperty ("ActionIcon", "torrent_ipfilter");
				connect (IPFilter_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_IPFilter__triggered ()));
			
				RemoveTorrent_.reset (new QAction (tr ("Remove"),
							Toolbar_.get ()));
				RemoveTorrent_->setShortcut (tr ("Del"));
				RemoveTorrent_->setProperty ("ActionIcon", "torrent_deletejob");
				connect (RemoveTorrent_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_RemoveTorrent__triggered ()));
				
				Resume_.reset (new QAction (tr ("Resume"),
							Toolbar_.get ()));
				Resume_->setShortcut (tr ("R"));
				Resume_->setProperty ("ActionIcon", "torrent_startjob");
				connect (Resume_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Resume__triggered ()));
			
				Stop_.reset (new QAction (tr ("Pause"),
							Toolbar_.get ()));
				Stop_->setShortcut (tr ("S"));
				Stop_->setProperty ("ActionIcon", "torrent_stopjob");
				connect (Stop_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Stop__triggered ()));
			
				MoveUp_.reset (new QAction (tr ("Move up"),
							Toolbar_.get ()));
				MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
				MoveUp_->setProperty ("ActionIcon", "torrent_moveup");
				connect (MoveUp_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveUp__triggered ()));
			
				MoveDown_.reset (new QAction (tr ("Move down"),
							Toolbar_.get ()));
				MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
				MoveDown_->setProperty ("ActionIcon", "torrent_movedown");
				connect (MoveDown_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveDown__triggered ()));
			
				MoveToTop_.reset (new QAction (tr ("Move to top"),
							Toolbar_.get ()));
				MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
				MoveToTop_->setProperty ("ActionIcon", "torrent_movetop");
				connect (MoveToTop_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveToTop__triggered ()));
			
				MoveToBottom_.reset (new QAction (tr ("Move to bottom"),
							Toolbar_.get ()));
				MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
				MoveToBottom_->setProperty ("ActionIcon", "torrent_movebottom");
				connect (MoveToBottom_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveToBottom__triggered ()));
			
				ForceReannounce_.reset (new QAction (tr ("Reannounce"),
							Toolbar_.get ()));
				ForceReannounce_->setShortcut (tr ("F"));
				ForceReannounce_->setProperty ("ActionIcon", "torrent_forcereannounce");
				connect (ForceReannounce_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ForceReannounce__triggered ()));
				
				ForceRecheck_.reset (new QAction (tr ("Recheck"),
						Toolbar_.get ()));
				ForceRecheck_->setProperty ("ActionIcon", "torrent_forcerecheck");
				connect (ForceRecheck_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ForceRecheck__triggered ()));
			
				MoveFiles_.reset (new QAction (tr ("Move files..."),
							Toolbar_.get ()));
				MoveFiles_->setShortcut (tr ("M"));
				MoveFiles_->setProperty ("ActionIcon", "torrent_movefiles");
				connect (MoveFiles_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_MoveFiles__triggered ()));
			
				ChangeTrackers_.reset (new QAction (tr ("Change trackers..."),
							Toolbar_.get ()));
				ChangeTrackers_->setShortcut (tr ("C"));
				ChangeTrackers_->setProperty ("ActionIcon", "torrent_changetrackers");
				connect (ChangeTrackers_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_ChangeTrackers__triggered ()));

				MakeMagnetLink_.reset (new QAction (tr ("Make magnet link..."),
							Toolbar_.get ()));
				MakeMagnetLink_->setProperty ("ActionIcon", "torrent_insertmagnetlink");
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
				Import_->setProperty ("ActionIcon", "torrent_import");
			
				Export_.reset (new QAction (tr ("Export..."),
							Toolbar_.get ()));
				connect (Export_.get (),
						SIGNAL (triggered ()),
						this,
						SLOT (on_Export__triggered ()));
				Export_->setProperty ("ActionIcon", "torrent_export");

				DownSelector_ = new QComboBox (Toolbar_.get ());
				UpSelector_ = new QComboBox (Toolbar_.get ());
				connect (DownSelector_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (handleFastSpeedComboboxes ()));
				connect (UpSelector_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (handleFastSpeedComboboxes ()));
			
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
				DownSelectorAction_ = Toolbar_->addWidget (DownSelector_);
				UpSelectorAction_ = Toolbar_->addWidget (UpSelector_);

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

Q_EXPORT_PLUGIN2 (leechcraft_bittorrent, LeechCraft::Plugins::BitTorrent::TorrentPlugin);

