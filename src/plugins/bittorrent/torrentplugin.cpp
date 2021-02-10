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
#include <QTabWidget>
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
#include <util/threads/futures.h>
#include <util/shortcuts/shortcutmanager.h>
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

using LC::ActionInfo;
using namespace LC::Util;

namespace LC
{
namespace BitTorrent
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
				const int normCol = index.column ();

				if (normCol == Core::ColumnProgress && role == Qt::DisplayRole)
					return sourceModel ()->data (index, Roles::FullLengthText);
				else
					return QSortFilterProxyModel::data (unmapped, role);
			}
		protected:
			bool filterAcceptsColumn (int sourceColumn, const QModelIndex&) const override
			{
				return sourceColumn >= Core::Columns::ColumnName &&
						sourceColumn <= Core::Columns::ColumnProgress;
			}
		};
	}

	void TorrentPlugin::Init (ICoreProxy_ptr proxy)
	{
		InstallTranslator ("bittorrent");
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
				.Holder_ = Core::Instance ()->GetSessionHolder (),
				.GetPreferredParent_ = [] { return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow (); }
			}
		};

		SetupCore ();
		SetupStuff ();

		TorrentTab_ = new TorrentTab (Core::Instance ()->GetSessionHolder (), TabTC_, this);
		connect (TorrentTab_,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

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

	void TorrentPlugin::Release ()
	{
		delete TorrentTab_;
		Core::Instance ()->Release ();
		XmlSettingsManager::Instance ()->Release ();
		XmlSettingsDialog_.reset ();
	}

	QIcon TorrentPlugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	qint64 TorrentPlugin::GetDownloadSpeed () const
	{
		return Core::Instance ()->GetSessionStats ().Rate_.Down_;
	}

	qint64 TorrentPlugin::GetUploadSpeed () const
	{
		return Core::Instance ()->GetSessionStats ().Rate_.Up_;
	}

	EntityTestHandleResult TorrentPlugin::CouldDownload (const Entity& e) const
	{
		return Core::Instance ()->CouldDownload (e);
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

		const auto& suggestedTags = e.Additional_ [" Tags"].toStringList ();
		const auto& autoTags = XmlSettingsManager::Instance ()->property ("AutomaticTags").toString ();
		auto tagsIds = tagsMgr->SplitToIDs (autoTags) + tagsMgr->GetIDs (suggestedTags);

		if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl resource = e.Entity_.toUrl ();
			if (resource.scheme () == "magnet")
			{
				for (const auto& [key, value] : QUrlQuery { resource }.queryItems ())
					if (key == QStringLiteral ("kt"))
						tagsIds += tagsMgr->GetIDs (value.split ('+', Qt::SkipEmptyParts));

				return Core::Instance ()->AddMagnet (resource.toString (),
						e.Location_,
						tagsIds,
						e.Parameters_);
			}
			else if (resource.scheme () == "file")
				suggestedFname = resource.toLocalFile ();
		}
		else if (Core::Instance ()->IsValidTorrent (e.Entity_.toByteArray ()))
		{
			QTemporaryFile tmpFile ("lctemporarybittorrentfile.XXXXXX");
			tmpFile.write (e.Entity_.toByteArray ());
			suggestedFname = tmpFile.fileName ().toUtf8 ();
			tmpFile.setAutoRemove (false);
		}

		QFile file { suggestedFname };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< suggestedFname
					<< file.errorString ();
			return Util::MakeReadyFuture (Result::Left ({ Error::Type::LocalError, "Unable to open file" }));
		}

		QString path;
		QVector<bool> files;
		QString fname;
		bool tryLive = e.Additional_ ["TryToStreamLive"].toBool ();
		if (e.Parameters_ & FromUserInitiated)
		{
			AddTorrent dia;
			dia.SetFilename (suggestedFname);
			if (!e.Location_.isEmpty ())
				dia.SetSavePath (e.Location_);
			else if (e.Parameters_ & IsDownloaded && !suggestedFname.isEmpty ())
				dia.SetSavePath (QFileInfo (suggestedFname).absolutePath ());

			if (!suggestedTags.isEmpty ())
				dia.SetTags (suggestedTags);

			ExecDialog (dia);

			if (dia.result () == QDialog::Rejected)
				return Util::MakeReadyFuture (Result::Left ({ Error::Type::UserCanceled, {} }));

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
		file.remove ();
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

			void HandleCurrentRowChanged (const QModelIndex& index) override
			{
				Plugin_->Actions_->SetCurrentIndex (index);
				Plugin_->TabWidget_->SetCurrentTorrent (index.row ());
			}

			void HandleSelectedRowsChanged (const QModelIndexList& indexes) override
			{
				Plugin_->Actions_->SetCurrentSelection (indexes);
			}
		};

		return std::make_shared<Handler> (this);
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

	XmlSettingsDialog_ptr TorrentPlugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void TorrentPlugin::SetShortcut (const QString& name,
			const QKeySequences_t& shortcuts)
	{
		Core::Instance ()->GetShortcutManager ()->SetShortcut (name, shortcuts);
	}

	QMap<QString, ActionInfo> TorrentPlugin::GetActionInfo () const
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
		{
			emit addNewTab ("BitTorrent", TorrentTab_);
			emit raiseTab (TorrentTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	QList<QWizardPage*> TorrentPlugin::GetWizardPages () const
	{
		return WizardGenerator {}.GetPages (Core::Instance ()->GetSessionSettingsManager ());
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
		XmlSettingsDialog_.reset (new XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"torrentsettings.xml");

		Core::Instance ()->DoDelayedInit ();

		SetupActions ();
		TabWidget_.reset (new TabWidget
			{
				Core::Instance ()->GetSessionHolder (),
				*Core::Instance ()->GetSessionSettingsManager (),
			});

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
		XmlSettingsDialog_->SetCustomWidget ("FastSpeedControl", fsc);
		connect (fsc,
				&FastSpeedControlWidget::speedsChanged,
				this,
				selectorsUpdater);
		XmlSettingsManager::Instance ()->RegisterObject ("EnableFastSpeedControl",
				this, [=] (auto) { selectorsUpdater (); });
	}

	void TorrentPlugin::SetupActions ()
	{
		auto toolbar = Actions_->GetToolbar ();
		auto openInTorrentTab = toolbar->addAction (tr ("Open in torrent tab"), this,
				[this]
				{
					const auto torrent = TabWidget_->GetCurrentTorrent ();
					if (torrent == -1)
						return;

					TorrentTab_->SetCurrentTorrent (torrent);
					TabOpenRequested (TabTC_.TabClass_);
				});
		openInTorrentTab->setIcon (TabTC_.Icon_);

		toolbar->addSeparator ();
		toolbar->addAction (openInTorrentTab);
		toolbar->addSeparator ();

		const auto ssm = Core::Instance ()->GetSessionSettingsManager ();

		DownSelectorAction_ = new SpeedSelectorAction { ssm, &SessionSettingsManager::SetOverallDownloadRate, "Down", this };
		toolbar->addAction (DownSelectorAction_);
		UpSelectorAction_ = new SpeedSelectorAction { ssm, &SessionSettingsManager::SetOverallUploadRate, "Up", this };
		toolbar->addAction (UpSelectorAction_);

		auto contextMenu = Actions_->MakeContextMenu ();
		contextMenu->addSeparator ();
		contextMenu->addAction (openInTorrentTab);
		Core::Instance ()->SetMenu (contextMenu);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_bittorrent, LC::BitTorrent::TorrentPlugin);
