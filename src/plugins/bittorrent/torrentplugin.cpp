/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrentplugin.h"
#include <QUrl>
#include <QTemporaryFile>
#include <QtDebug>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QUrlQuery>
#include <QFileInfo>
#include <QMainWindow>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/ijobholderrepresentationhandler.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include "core.h"
#include "addtorrent.h"
#include "xmlsettingsmanager.h"
#include "wizardgenerator.h"
#include "fastspeedcontrolwidget.h"
#include "speedselectoraction.h"
#include "torrenttab.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "types.h"
#include "listactions.h"
#include "ltutils.h"

using LC::ActionInfo;
using namespace LC::Util;

namespace LC::BitTorrent
{
	namespace
	{
		class ReprProxy final : public QSortFilterProxyModel
		{
		public:
			explicit ReprProxy (QAbstractItemModel *model)
			: QSortFilterProxyModel (model)
			{
				setDynamicSortFilter (true);
				setSourceModel (model);
			}

			QVariant data (const QModelIndex& unmapped, int role) const override
			{
				const auto& index = mapToSource (unmapped);
				if (index.column () == Columns::ColumnProgress && role == Qt::DisplayRole)
					return sourceModel ()->data (index, Roles::FullProgressText);

				return QSortFilterProxyModel::data (unmapped, role);
			}
		protected:
			bool filterAcceptsColumn (int sourceColumn, const QModelIndex&) const override
			{
				return sourceColumn >= Columns::ColumnName &&
						sourceColumn <= Columns::ColumnProgress;
			}
		};
	}

	void TorrentPlugin::Init (ICoreProxy_ptr proxy)
	{
		InstallTranslator (QStringLiteral ("bittorrent"));
		Core::Instance ()->SetProxy (proxy);

		TabTC_ =
		{
			GetUniqueID () + "_TorrentTab",
			tr ("BitTorrent tab"),
			tr ("Full BitTorrent downloads tab."),
			GetIcon (),
			10,
			TFSingle | TFOpenableByRequest | TFSuggestOpening
		};

		Actions_ = new ListActions
		{
			{
				.Session_ = Core::Instance ()->GetSession (),
				.StatusKeeper_ = *Core::Instance ()->GetStatusKeeper (),
				.GetPreferredParent_ = [] { return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow (); }
			}
		};

		SetupCore ();
		SetupStuff ();

		TorrentTab_ = new TorrentTab
		{
			{
				.Session_ = Core::Instance ()->GetSession (),
				.Model_ = *Core::Instance (),
				.Dispatcher_ = Core::Instance ()->GetAlertDispatcher (),
				.SSM_ = *Core::Instance ()->GetSessionSettingsManager (),
				.StatusKeeper_ = *Core::Instance ()->GetStatusKeeper (),
			},
			TabTC_,
			this
		};

		ReprProxy_ = new ReprProxy (Core::Instance ());
	}

	void TorrentPlugin::SecondInit ()
	{
	}

	QByteArray TorrentPlugin::GetUniqueID () const
	{
		return "org.LeechCraft.BitTorrent";
	}

	QString TorrentPlugin::GetName () const
	{
		return QStringLiteral ("BitTorrent");
	}

	QString TorrentPlugin::GetInfo () const
	{
		return tr ("Full-featured BitTorrent client.");
	}

	QStringList TorrentPlugin::Provides () const
	{
		return { QStringLiteral ("bittorrent"), QStringLiteral ("resume"), QStringLiteral ("remoteable") };
	}

	void TorrentPlugin::Release ()
	{
		delete TorrentTab_;
		Core::Instance ()->Release ();
		XmlSettingsManager::Instance ().Release ();
		XmlSettingsDialog_.reset ();
	}

	QIcon TorrentPlugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	qint64 TorrentPlugin::GetDownloadSpeed () const
	{
		return GetSessionStats (Core::Instance ()->GetSession ()).Rate_.Down_;
	}

	qint64 TorrentPlugin::GetUploadSpeed () const
	{
		return GetSessionStats (Core::Instance ()->GetSession ()).Rate_.Up_;
	}

	namespace
	{
		EntityTestHandleResult CouldDownloadUrl (const QUrl& url)
		{
			if (url.scheme () == "magnet"_ql)
			{
				const auto& items = QUrlQuery { url }.queryItems ();
				const bool hasMagnet = std::any_of (items.begin (), items.end (),
						[] (const auto& item) { return item.first == "xt" && item.second.startsWith ("urn:btih:"); });
				return hasMagnet ?
						EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
						EntityTestHandleResult {};
			}

			if (url.scheme () == "file"_ql)
			{
				const auto& str = url.toLocalFile ();
				QFile file { str };
				if (!file.exists () ||
						!file.open (QIODevice::ReadOnly))
					return {};

				auto& xsm = XmlSettingsManager::Instance ();
				if (file.size () > xsm.property ("MaxAutoTorrentSize").toInt () * 1024 * 1024)
				{
					if (str.endsWith (".torrent"_ql, Qt::CaseInsensitive))
					{
						auto msg = TorrentPlugin::tr ("Rejecting file %1 because it's bigger than current auto limit.")
								.arg (str);
						const auto& entity = Util::MakeNotification (QStringLiteral ("BitTorrent"),
								msg, Priority::Warning);
						GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
					}
					return EntityTestHandleResult ();
				}

				return IsValidTorrent (file.readAll ()) ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}

			return {};
		}
	}

	EntityTestHandleResult TorrentPlugin::CouldDownload (const Entity& e) const
	{
		if (e.Entity_.canConvert<QUrl> ())
			return CouldDownloadUrl (e.Entity_.toUrl ());

		if (e.Entity_.canConvert<QByteArray> ())
			return IsValidTorrent (e.Entity_.toByteArray ()) ?
					EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
					EntityTestHandleResult ();

		return {};
	}

	namespace
	{
		void ExecDialog (QDialog& dia)
		{
			dia.show ();
			QEventLoop dialogGuard;
			QObject::connect (&dia,
					&QDialog::finished,
					&dialogGuard,
					&QEventLoop::quit);
			dialogGuard.exec ();
		}
	}

	QFuture<IDownload::Result> TorrentPlugin::AddJob (Entity e)
	{
		QString suggestedFname;

		const auto tagsMgr = GetProxyHolder ()->GetTagsManager ();

		const auto& suggestedTags = e.Additional_ [QStringLiteral (" Tags")].toStringList ();
		const auto& autoTags = XmlSettingsManager::Instance ().property ("AutomaticTags").toString ();
		auto tagsIds = tagsMgr->SplitToIDs (autoTags) + tagsMgr->GetIDs (suggestedTags);

		if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl resource = e.Entity_.toUrl ();
			if (resource.scheme () == "magnet"_ql)
			{
				for (const auto& [key, value] : QUrlQuery { resource }.queryItems ())
					if (key == QStringLiteral ("kt"))
						tagsIds += tagsMgr->GetIDs (value.split ('+', Qt::SkipEmptyParts));

				return Core::Instance ()->AddMagnet (resource.toString (),
						e.Location_,
						tagsIds,
						e.Parameters_);
			}

			if (resource.scheme () == "file"_ql)
				suggestedFname = resource.toLocalFile ();
		}
		else if (IsValidTorrent (e.Entity_.toByteArray ()))
		{
			QTemporaryFile tmpFile { QStringLiteral ("lctemporarybittorrentfile.XXXXXX") };
			tmpFile.write (e.Entity_.toByteArray ());
			suggestedFname = tmpFile.fileName ().toUtf8 ();
			tmpFile.setAutoRemove (false);
		}

		QString path;
		QVector<bool> files;
		QString fname;
		bool tryLive = e.Additional_ [QStringLiteral ("TryToStreamLive")].toBool ();
		if (e.Parameters_ & FromUserInitiated)
		{
			AddTorrent dia;
			dia.SetFilename (suggestedFname);
			dia.SetTags (suggestedTags);
			if (!e.Location_.isEmpty ())
				dia.SetSavePath (e.Location_);
			else if (e.Parameters_ & IsDownloaded && !suggestedFname.isEmpty ())
				dia.SetSavePath (QFileInfo (suggestedFname).absolutePath ());

			ExecDialog (dia);

			if (dia.result () == QDialog::Rejected)
				return MakeReadyFuture (Result { AsLeft, { Error::Type::UserCanceled, {} } });

			fname = dia.GetFilename (),
			path = dia.GetSavePath ();
			tryLive = dia.GetTryLive ();
			files = dia.GetSelectedFiles ();
			tagsIds = dia.GetTags ();
			if (dia.GetAddType () == AddState::Started)
				e.Parameters_ &= ~NoAutostart;
			else
				e.Parameters_ |= NoAutostart;
		}
		else
		{
			fname = suggestedFname;
			path = e.Location_;
		}
		auto result = Core::Instance ()->AddFile (fname,
				path,
				tagsIds,
				tryLive,
				files,
				e.Parameters_);
		QFile::remove (suggestedFname);
		return result;
	}

	QAbstractItemModel* TorrentPlugin::GetRepresentation () const
	{
		return ReprProxy_;
	}

	IJobHolderRepresentationHandler_ptr TorrentPlugin::CreateRepresentationHandler ()
	{
		class Handler : public IJobHolderRepresentationHandler
		{
			TorrentPlugin * const Plugin_;
		public:
			explicit Handler (TorrentPlugin *plugin)
			: Plugin_ { plugin }
			{
			}

			void HandleCurrentRowChanged (const QModelIndex& srcIdx) override
			{
				const auto& index = Plugin_->ReprProxy_->mapToSource (srcIdx);
				Plugin_->Actions_->SetCurrentIndex (index);
				Plugin_->TabWidget_->SetCurrentTorrent (index);
			}

			void HandleSelectedRowsChanged (const QModelIndexList& srcIdxs) override
			{
				const auto& indexes = Util::Map (srcIdxs,
						[this] (const auto& idx) { return Plugin_->ReprProxy_->mapToSource (idx); });
				Plugin_->Actions_->SetCurrentSelection (indexes);
			}
		};

		return std::make_shared<Handler> (this);
	}

	void TorrentPlugin::SetTags (int torrent, const QStringList& tags)
	{
		auto& model = *Core::Instance ();
		model.setData (model.index (torrent, 0), tags, Roles::TorrentTags);
	}

	XmlSettingsDialog_ptr TorrentPlugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void TorrentPlugin::SetShortcut (const QByteArray& name, const QKeySequences_t& shortcuts)
	{
		Core::Instance ()->GetShortcutManager ()->SetShortcut (name, shortcuts);
	}

	QMap<QByteArray, ActionInfo> TorrentPlugin::GetActionInfo () const
	{
		return Core::Instance ()->GetShortcutManager ()->GetActionInfo ();
	}

	TabClasses_t TorrentPlugin::GetTabClasses () const
	{
		return { TabTC_ };
	}

	void TorrentPlugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == TabTC_.TabClass_)
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (GetName (), TorrentTab_);
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	QList<QWizardPage*> TorrentPlugin::GetWizardPages () const
	{
		return CreateStartupWizard (Core::Instance ()->GetSessionSettingsManager ());
	}

	QList<QAction*> TorrentPlugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QString TorrentPlugin::GetDiagInfoString () const
	{
		return QStringLiteral ("Built with rb_libtorrent %1 (%2).")
				.arg (LIBTORRENT_VERSION, LIBTORRENT_REVISION);
	}

	void TorrentPlugin::SetupCore ()
	{
		XmlSettingsDialog_ = std::make_shared<XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), QStringLiteral ("torrentsettings.xml"));

		Core::Instance ()->DoDelayedInit ();

		SetupActions ();
		TabWidget_ = std::make_unique<TabWidget> (*Core::Instance (),
				Core::Instance ()->GetSession (),
				*Core::Instance ()->GetSessionSettingsManager ());

		Core::Instance ()->SetWidgets (Actions_->GetToolbar (), TabWidget_.get ());
	}

	void TorrentPlugin::SetupStuff ()
	{
		auto statsUpdateTimer = new QTimer { this };
		statsUpdateTimer->callOnTimeout (TabWidget_.get (), &TabWidget::UpdateTorrentStats);
		statsUpdateTimer->start (2000);

		const auto selectorsUpdater = [this]
		{
			DownSelectorAction_->HandleSpeedsChanged ();
			UpSelectorAction_->HandleSpeedsChanged ();
		};

		const auto fsc = new FastSpeedControlWidget ();
		XmlSettingsDialog_->SetCustomWidget (QStringLiteral ("FastSpeedControl"), fsc);
		connect (fsc,
				&FastSpeedControlWidget::speedsChanged,
				this,
				selectorsUpdater);
		XmlSettingsManager::Instance ().RegisterObject ("EnableFastSpeedControl",
				this, [=] (auto) { selectorsUpdater (); });
	}

	void TorrentPlugin::SetupActions ()
	{
		auto toolbar = Actions_->GetToolbar ();
		auto openInTorrentTab = toolbar->addAction (tr ("Open in torrent tab"), this,
				[this]
				{
					const auto torrent = TabWidget_->GetCurrentTorrent ();
					if (!torrent.isValid ())
						return;

					TorrentTab_->SetCurrentTorrent (torrent);
					TabOpenRequested (TabTC_.TabClass_);
				});
		openInTorrentTab->setIcon (TabTC_.Icon_);

		toolbar->addSeparator ();
		toolbar->addAction (openInTorrentTab);
		toolbar->addSeparator ();

		const auto ssm = Core::Instance ()->GetSessionSettingsManager ();

		DownSelectorAction_ = new SpeedSelectorAction
		{
			ssm,
			&SessionSettingsManager::SetOverallDownloadRate,
			QStringLiteral ("Down"),
			this
		};
		toolbar->addAction (DownSelectorAction_);
		UpSelectorAction_ = new SpeedSelectorAction
		{
			ssm,
			&SessionSettingsManager::SetOverallUploadRate,
			QStringLiteral ("Up"),
			this
		};
		toolbar->addAction (UpSelectorAction_);

		auto contextMenu = Actions_->MakeContextMenu ();
		contextMenu->addSeparator ();
		contextMenu->addAction (openInTorrentTab);
		Core::Instance ()->SetMenu (contextMenu);
	}
}

LC_EXPORT_PLUGIN (leechcraft_bittorrent, LC::BitTorrent::TorrentPlugin);
