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
#include <QAction>
#include <QTimer>
#include <QUrlQuery>
#include <QFileInfo>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
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
#include "ltutils.h"
#include "representationhandler.h"

using namespace LC::Util;

namespace LC::BitTorrent
{
	void TorrentPlugin::Init (ICoreProxy_ptr proxy)
	{
		InstallTranslator ("bittorrent"_qs);
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

		XmlSettingsDialog_ = std::make_shared<XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), "torrentsettings.xml"_qs);

		Core::Instance ()->DoDelayedInit ();

		FastSpeedControlWidget_ = new FastSpeedControlWidget ();
		XmlSettingsDialog_->SetCustomWidget ("FastSpeedControl"_qs, FastSpeedControlWidget_);

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
		return "BitTorrent"_qs;
	}

	QString TorrentPlugin::GetInfo () const
	{
		return tr ("Full-featured BitTorrent client.");
	}

	QStringList TorrentPlugin::Provides () const
	{
		return { "bittorrent"_qs, "resume"_qs, "remoteable"_qs };
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
						const auto& entity = Util::MakeNotification ("BitTorrent"_qs,
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

		const auto& suggestedTags = e.Additional_ [" Tags"_qs].toStringList ();
		const auto& autoTags = XmlSettingsManager::Instance ().property ("AutomaticTags").toString ();
		auto tagsIds = tagsMgr->SplitToIDs (autoTags) + tagsMgr->GetIDs (suggestedTags);

		if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl resource = e.Entity_.toUrl ();
			if (resource.scheme () == "magnet"_ql)
			{
				for (const auto& [key, value] : QUrlQuery { resource }.queryItems ())
					if (key == "kt"_qs)
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
			QTemporaryFile tmpFile { "lctemporarybittorrentfile.XXXXXX"_qs };
			tmpFile.write (e.Entity_.toByteArray ());
			suggestedFname = tmpFile.fileName ().toUtf8 ();
			tmpFile.setAutoRemove (false);
		}

		QString path;
		QVector<bool> files;
		QString fname;
		bool tryLive = e.Additional_ ["TryToStreamLive"_qs].toBool ();
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

	IJobHolderRepresentationHandler_ptr TorrentPlugin::CreateRepresentationHandler ()
	{
		auto handler = std::make_unique<RepresentationHandler> ();

		connect (FastSpeedControlWidget_,
				&FastSpeedControlWidget::speedsChanged,
				&*handler,
				&RepresentationHandler::UpdateSpeedControllerOptions);
		XmlSettingsManager::Instance ().RegisterObject ("EnableFastSpeedControl",
				&*handler, [handler = &*handler] (auto) { handler->UpdateSpeedControllerOptions (); });

		connect (&*handler,
				&RepresentationHandler::torrentTabFocusRequested,
				this,
				[this] (const QModelIndex& torrent)
				{
					TorrentTab_->SetCurrentTorrent (torrent);
					TabOpenRequested (TabTC_.TabClass_);
				});

		return handler;
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
		return "Built with rb_libtorrent %1 (%2)."_qs.arg (LIBTORRENT_VERSION, LIBTORRENT_REVISION);
	}
}

LC_EXPORT_PLUGIN (leechcraft_bittorrent, LC::BitTorrent::TorrentPlugin);
