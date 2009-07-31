#ifndef PLUGINS_BITTORRENT_TORRENTPLUIGN_H
#define PLUGINS_BITTORRENT_TORRENTPLUIGN_H
#define PLUGINS_BITTORRENT_WIN32_LEAN_AND_MEAN
#include <memory>
#include <deque>
#include <QMainWindow>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iimportexport.h>
#include <interfaces/itaggablejobs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/itoolbarembedder.h>
#include <plugininterface/tagscompleter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "tabwidget.h"
#include "torrentinfo.h"

class QTimer;
class QToolBar;
class QTabWidget;
class QTranslator;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class AddTorrent;
			class RepresentationModel;

			class TorrentPlugin : public QObject
								, public IInfo
								, public IDownload
								, public IJobHolder
								, public IImportExport
								, public ITaggableJobs
								, public IHaveSettings
								, public IHaveShortcuts
								, public IToolBarEmbedder
			{
				Q_OBJECT

				Q_INTERFACES (IInfo IDownload IJobHolder IImportExport ITaggableJobs IHaveSettings IHaveShortcuts IToolBarEmbedder);

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<AddTorrent> AddTorrentDialog_;
				std::auto_ptr<QTimer> OverallStatsUpdateTimer_;
				std::auto_ptr<QTime> LastPeersUpdate_;
				std::auto_ptr<RepresentationModel> FilterModel_;
				bool TorrentSelectionChanged_;
				std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsAddDiaCompleter_;
				std::auto_ptr<TabWidget> TabWidget_;
				std::auto_ptr<QToolBar> Toolbar_;
				std::auto_ptr<QAction> OpenTorrent_,
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
					MoveFiles_,
					ChangeTrackers_,
					Import_,
					Export_;
				std::auto_ptr<QTranslator> Translator_;

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
					EAExport_
				};
			public:
				// IInfo
				void Init (ICoreProxy_ptr);
				virtual ~TorrentPlugin ();
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
				bool CouldDownload (const LeechCraft::DownloadEntity&) const;
				int AddJob (LeechCraft::DownloadEntity);

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
				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				// IHaveShortcuts
				void SetShortcut (int, const QKeySequence&);
				QMap<int, LeechCraft::ActionInfo> GetActionInfo () const;

				// IToolBarEmbedder
				QList<QAction*> GetActions () const;
			private slots:
				void on_OpenTorrent__triggered ();
				void on_OpenMultipleTorrents__triggered ();
				void on_CreateTorrent__triggered ();
				void on_RemoveTorrent__triggered (int);
				void on_Resume__triggered (int);
				void on_Stop__triggered (int);
				void on_MoveUp__triggered (const std::deque<int>&);
				void on_MoveDown__triggered (const std::deque<int>&);
				void on_MoveToTop__triggered (const std::deque<int>&);
				void on_MoveToBottom__triggered (const std::deque<int>&);
				void on_ForceReannounce__triggered (int);
				void on_ForceRecheck__triggered (int);
				void on_ChangeTrackers__triggered ();
				void on_MoveFiles__triggered (int = 0);
				void on_Import__triggered ();
				void on_Export__triggered ();
				void setActionsEnabled ();
				void showError (QString);
				void doLogMessage (const QString&);
				void handleItemSelected (const QModelIndex&);
			private:
				void SetupCore ();
				void SetupTorrentView ();
				void SetupStuff ();
				void SetupActions ();
			signals:
				void downloadFinished (const QString&);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void jobFinished (int);
				void jobRemoved (int);
				void log (const QString&) const;
			};
		};
	};
};

#endif

