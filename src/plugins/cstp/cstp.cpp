/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cstp.h"
#include <QMenu>
#include <QTranslator>
#include <QTabWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QUrl>
#include <QTextCodec>
#include <QMainWindow>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/ijobholderrepresentationhandler.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace CSTP
{
	void CSTP::Init (ICoreProxy_ptr coreProxy)
	{
		Proxy_ = coreProxy;

		Core::Instance ().SetCoreProxy (coreProxy);

		Util::InstallTranslator ("cstp");

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"cstpsettings.xml");

		SetupToolbar ();

		Core::Instance ().SetToolbar (Toolbar_);

		connect (&Core::Instance (),
				SIGNAL (error (QString)),
				this,
				SLOT (handleError (QString)));
	}

	void CSTP::SecondInit ()
	{
	}

	void CSTP::Release ()
	{
		Core::Instance ().Release ();
		XmlSettingsManager::Instance ().Release ();
		XmlSettingsDialog_.reset ();
	}

	QByteArray CSTP::GetUniqueID () const
	{
		return "org.LeechCraft.CSTP";
	}

	QString CSTP::GetName () const
	{
		return "CSTP";
	}

	QString CSTP::GetInfo () const
	{
		return "Common Stream Transfer Protocols";
	}

	QStringList CSTP::Provides () const
	{
		return { "http", "https", "remoteable", "resume" };
	}

	QIcon CSTP::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	qint64 CSTP::GetDownloadSpeed () const
	{
		return Core::Instance ().GetTotalDownloadSpeed ();
	}

	qint64 CSTP::GetUploadSpeed () const
	{
		return 0;
	}

	EntityTestHandleResult CSTP::CouldDownload (const LC::Entity& e) const
	{
		return Core::Instance ().CouldDownload (e);
	}

	QFuture<IDownload::Result> CSTP::AddJob (LC::Entity e)
	{
		return Core::Instance ().AddTask (e);
	}

	QAbstractItemModel* CSTP::GetRepresentation () const
	{
		return Core::Instance ().GetRepresentationModel ();
	}

	IJobHolderRepresentationHandler_ptr CSTP::CreateRepresentationHandler ()
	{
		class Handler : public IJobHolderRepresentationHandler
		{
		public:
			void HandleCurrentRowChanged (const QModelIndex& index) override
			{
				Core::Instance ().ItemSelected (index);
			}
		};

		return std::make_shared<Handler> ();
	}

	Util::XmlSettingsDialog_ptr CSTP::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void CSTP::SetupToolbar ()
	{
		Toolbar_ = new QToolBar;
		Toolbar_->setWindowTitle ("CSTP");

		QAction *remove = Toolbar_->addAction (tr ("Remove"));
		connect (remove,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (removeTriggered ()));
		remove->setProperty ("ActionIcon", "list-remove");

		QAction *removeAll = Toolbar_->addAction (tr ("Remove all"));
		connect (removeAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (removeAllTriggered ()));
		removeAll->setProperty ("ActionIcon", "edit-clear-list");

		Toolbar_->addSeparator ();

		QAction *start = Toolbar_->addAction (tr ("Start"));
		connect (start,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (startTriggered ()));
		start->setProperty ("ActionIcon", "media-playback-start");

		QAction *stop = Toolbar_->addAction (tr ("Stop"));
		connect (stop,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (stopTriggered ()));
		stop->setProperty ("ActionIcon", "media-playback-stop");

		QAction *startAll = Toolbar_->addAction (tr ("Start all"));
		connect (startAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (startAllTriggered ()));
		startAll->setProperty ("ActionIcon", "media-seek-forward");

		QAction *stopAll = Toolbar_->addAction (tr ("Stop all"));
		connect (stopAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (stopAllTriggered ()));
		stopAll->setProperty ("ActionIcon", "media-record");
	}

	void CSTP::handleFileExists (Core::FileExistsBehaviour *remove)
	{
		auto rootWM = Core::Instance ().GetCoreProxy ()->GetRootWindowsManager ();
		auto userReply = QMessageBox::warning (rootWM->GetPreferredWindow (),
				tr ("File exists"),
				tr ("File %1 already exists, continue download?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (userReply == QMessageBox::Yes)
			*remove = Core::FileExistsBehaviour::Continue;
		else if (userReply == QMessageBox::No)
			*remove = Core::FileExistsBehaviour::Remove;
		else
			*remove = Core::FileExistsBehaviour::Abort;
	}

	void CSTP::handleError (const QString& error)
	{
		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("HTTP error", error, Priority::Critical));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_cstp, LC::CSTP::CSTP);
