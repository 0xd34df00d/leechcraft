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

#ifndef PLUGINS_BITTORRENT_TORRENTPLUIGN_H
#define PLUGINS_BITTORRENT_TORRENTPLUIGN_H
#define PLUGINS_BITTORRENT_WIN32_LEAN_AND_MEAN
#include <memory>
#include <deque>
#include <QMainWindow>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iimportexport.h>
#include <interfaces/itaggablejobs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavediaginfo.h>
#include <util/tagscompleter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "tabwidget.h"
#include "torrentinfo.h"

class QTimer;
class QToolBar;
class QComboBox;
class QTabWidget;
class QTranslator;

namespace LeechCraft
{
	namespace Util
	{
		class ShortcutManager;
	}

	namespace Plugins
	{
		namespace BitTorrent
		{
			class AddTorrent;
			class RepresentationModel;
			class SpeedSelectorAction;

			class TorrentPlugin : public QObject
								, public IInfo
								, public IDownload
								, public IEntityHandler
								, public IJobHolder
								, public IImportExport
								, public ITaggableJobs
								, public IHaveSettings
								, public IHaveShortcuts
								, public IStartupWizard
								, public IActionsExporter
								, public IHaveDiagInfo
			{
				Q_OBJECT

				Q_INTERFACES (IInfo
						IDownload
						IEntityHandler
						IJobHolder
						IImportExport
						ITaggableJobs
						IHaveSettings
						IHaveShortcuts
						IStartupWizard
						IActionsExporter
						IHaveDiagInfo);

				std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<AddTorrent> AddTorrentDialog_;
				std::auto_ptr<QTimer> OverallStatsUpdateTimer_;
				std::auto_ptr<QTime> LastPeersUpdate_;
				bool TorrentSelectionChanged_;
				std::unique_ptr<LeechCraft::Util::TagsCompleter> TagsAddDiaCompleter_;
				std::unique_ptr<TabWidget> TabWidget_;
				std::unique_ptr<QToolBar> Toolbar_;
				std::unique_ptr<QAction> OpenTorrent_,
					RemoveTorrent_,
					Resume_,
					Stop_,
					CreateTorrent_,
					MoveUp_,
					MoveDown_,
					MoveToTop_,
					MoveToBottom_,
					ForceReannounce_,
					ForceRecheck_,
					OpenMultipleTorrents_,
					IPFilter_,
					MoveFiles_,
					ChangeTrackers_,
					MakeMagnetLink_,
					Import_,
					Export_;
				std::unique_ptr<QTranslator> Translator_;

				SpeedSelectorAction *DownSelectorAction_,
						*UpSelectorAction_;

				Util::ShortcutManager *ShortcutMgr_;

				enum
				{
					EAOpenTorrent_,
					EAChangeTrackers_,
					EACreateTorrent_,
					EAOpenMultipleTorrents_,
					EARemoveTorrent_,
					EAResume_,
					EAStop_,
					EAMoveUp_,
					EAMoveDown_,
					EAMoveToTop_,
					EAMoveToBottom_,
					EAForceReannounce_,
					EAForceRecheck_,
					EAMoveFiles_,
					EAImport_,
					EAExport_,
					EAMakeMagnetLink_,
					EAIPFilter_
				};
			public:
				// IInfo
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				virtual ~TorrentPlugin ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				void Release ();
				QIcon GetIcon () const;

				// IDownload
				qint64 GetDownloadSpeed () const;
				qint64 GetUploadSpeed () const;
				void StartAll ();
				void StopAll ();
				EntityTestHandleResult CouldDownload (const LeechCraft::Entity&) const;
				int AddJob (LeechCraft::Entity);
				void KillTask (int);

				// IEntityHandler
				EntityTestHandleResult CouldHandle (const LeechCraft::Entity&) const;
				void Handle (LeechCraft::Entity);

				// IJobHolder
				QAbstractItemModel* GetRepresentation () const;

				// IImportExport
				void ImportSettings (const QByteArray&);
				void ImportData (const QByteArray&);
				QByteArray ExportSettings () const;
				QByteArray ExportData () const;

				// ITaggableJobs
				void SetTags (int, const QStringList&);

				// IHaveSettings
				std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				// IHaveShortcuts
				void SetShortcut (const QString&, const QKeySequences_t&);
				QMap<QString, ActionInfo> GetActionInfo () const;

				// IStartupWizard
				QList<QWizardPage*> GetWizardPages () const;

				// IToolBarEmbedder
				QList<QAction*> GetActions (ActionsEmbedPlace) const;

				// IHaveDiagInfo
				QString GetDiagInfoString () const;
			private slots:
				void on_OpenTorrent__triggered ();
				void on_OpenMultipleTorrents__triggered ();
				void on_IPFilter__triggered ();
				void on_CreateTorrent__triggered ();
				void on_RemoveTorrent__triggered ();
				void on_Resume__triggered ();
				void on_Stop__triggered ();
				void on_MoveUp__triggered ();
				void on_MoveDown__triggered ();
				void on_MoveToTop__triggered ();
				void on_MoveToBottom__triggered ();
				void on_ForceReannounce__triggered ();
				void on_ForceRecheck__triggered ();
				void on_ChangeTrackers__triggered ();
				void on_MoveFiles__triggered ();
				void on_MakeMagnetLink__triggered ();
				void on_Import__triggered ();
				void on_Export__triggered ();
				void handleFastSpeedComboboxes ();
				void setActionsEnabled ();
				void showError (QString);
				void handleTasksTreeSelectionCurrentRowChanged (const QModelIndex&, const QModelIndex&);
			private:
				void SetupCore ();
				void SetupStuff ();
				void SetupActions ();
			signals:
				void gotEntity (const LeechCraft::Entity&);
				void jobFinished (int);
				void jobRemoved (int);

				void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
			};
		};
	};
};

#endif

