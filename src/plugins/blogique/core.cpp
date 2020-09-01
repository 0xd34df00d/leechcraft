/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QtDebug>
#include <QTimer>
#include <QMessageBox>
#include <QMainWindow>
#include <interfaces/iplugin2.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/sll/prelude.h>
#include "interfaces/blogique/ibloggingplatformplugin.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "commentsmanager.h"
#include "exportwizard.h"
#include "pluginproxy.h"
#include "storagemanager.h"
#include "blogiquewidget.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
	Core::Core ()
	: PluginProxy_ (std::make_shared<PluginProxy> ())
	, StorageManager_ (new StorageManager ("org.LeechCraft.Blogique", this))
	, CommentsManager_ (new CommentsManager (this))
	, AutoSaveTimer_ (new QTimer (this))
	{
		connect (AutoSaveTimer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (checkAutoSave ()));
		XmlSettingsManager::Instance ().RegisterObject ("AutoSave",
				this, "handleAutoSaveIntervalChanged");
		handleAutoSaveIntervalChanged ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	QIcon Core::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetCoreProxy ()
	{
		return Proxy_;
	}

	void Core::Release ()
	{
		BlogPlatformPlugins_.clear ();
	}

	void Core::AddPlugin (QObject *plugin)
	{
		IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
		if (!plugin2)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "isn't a IPlugin2";
			return;
		}

		QByteArray sig = QMetaObject::normalizedSignature ("initPlugin (QObject*)");
		if (plugin->metaObject ()->indexOfMethod (sig) != -1)
			QMetaObject::invokeMethod (plugin,
					"initPlugin",
					Q_ARG (QObject*, PluginProxy_.get ()));

		QSet<QByteArray> classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin"))
			AddBlogPlatformPlugin (plugin);
	}

	QList<IBloggingPlatform*> Core::GetBloggingPlatforms () const
	{
		auto result = Util::ConcatMap (BlogPlatformPlugins_,
				[] (auto bpp)
				{
					const auto platforms = qobject_cast<IBloggingPlatformPlugin*> (bpp)->GetBloggingPlatforms ();
					return Util::Map (platforms, [] (auto obj) { return qobject_cast<IBloggingPlatform*> (obj); });
				});
		result.removeAll (0);
		return result;
	}

	QList<IAccount*> Core::GetAccounts () const
	{
		auto result = Util::ConcatMap (GetBloggingPlatforms (),
				[] (auto bp)
				{
					return Util::Map (bp->GetRegisteredAccounts (),
							[] (auto obj) { return qobject_cast<IAccount*> (obj); });
				});
		result.removeAll (0);
		return result;
	}

	IAccount* Core::GetAccountByID (const QByteArray& id) const
	{
		for (auto acc : GetAccounts ())
			if (acc->GetAccountID () == id)
				return acc;
		return 0;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::DelayedProfilesUpdate ()
	{
		QTimer::singleShot (15000, this, SLOT (updateProfiles ()));
	}

	StorageManager* Core::GetStorageManager () const
	{
		return StorageManager_;
	}

	CommentsManager* Core::GetCommentsManager () const
	{
		return CommentsManager_;
	}

	BlogiqueWidget* Core::CreateBlogiqueWidget ()
	{
		auto newTab = new BlogiqueWidget;
		connect (newTab,
				SIGNAL (removeTab (QWidget*)),
				&Core::Instance (),
				SIGNAL (removeTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (checkAutoSave ()),
				newTab,
				SLOT (handleAutoSave ()));
		connect (&Core::Instance (),
				SIGNAL (entryPosted ()),
				newTab,
				SLOT (handleEntryPosted ()));
		connect (&Core::Instance (),
				SIGNAL (entryRemoved ()),
				newTab,
				SLOT (handleEntryRemoved ()));
		connect (&Core::Instance (),
				SIGNAL (tagsUpdated (QHash<QString, int>)),
				newTab,
				SLOT (handleTagsUpdated (QHash<QString, int>)));
		connect (&Core::Instance (),
				SIGNAL (insertTag (QString)),
				newTab,
				SLOT (handleInsertTag (QString)));
		connect (&Core::Instance (),
				SIGNAL (gotError (int, QString, QString)),
				newTab,
				SLOT (handleGotError (int, QString, QString)));
		connect (&Core::Instance (),
				SIGNAL (gotError (int, QString, QString)),
				newTab,
				SLOT (handleGotError (int, QString, QString)));
		connect (&Core::Instance (),
				SIGNAL (accountAdded (QObject*)),
				newTab,
				SLOT (handleAccountAdded (QObject*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (QObject*)),
				newTab,
				SLOT (handleAccountRemoved (QObject*)));

		return newTab;
	}

	void Core::AddBlogPlatformPlugin (QObject *plugin)
	{
		IBloggingPlatformPlugin *ibpp = qobject_cast<IBloggingPlatformPlugin*> (plugin);
		if (!ibpp)
			qWarning () << Q_FUNC_INFO
					<< "plugin"
					<< plugin
					<< "tells it implements the IBlogPlatformPlugin but cast failed";
		else
		{
			BlogPlatformPlugins_ << plugin;
			handleNewBloggingPlatforms (ibpp->GetBloggingPlatforms ());
		}
	}

	void Core::handleNewBloggingPlatforms (const QObjectList& platforms)
	{
		for (const auto platformObj : platforms)
		{
			const auto platform = qobject_cast<IBloggingPlatform*> (platformObj);

			for (const auto accObj : platform->GetRegisteredAccounts ())
				addAccount (accObj);

			connect (platform->GetQObject (),
					SIGNAL (accountAdded (QObject*)),
					this,
					SLOT (addAccount (QObject*)));
			connect (platform->GetQObject (),
					SIGNAL (accountRemoved (QObject*)),
					this,
					SLOT (handleAccountRemoved (QObject*)));
			connect (platform->GetQObject (),
					SIGNAL (accountValidated (QObject*, bool)),
					this,
					SLOT (handleAccountValidated (QObject*, bool)));
			connect (platform->GetQObject (),
					SIGNAL (insertTag (QString)),
					this,
					SIGNAL (insertTag (QString)));
		}
	}

	void Core::addAccount (QObject *accObj)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		connect (accObj,
				SIGNAL (requestEntriesBegin ()),
				this,
				SIGNAL (requestEntriesBegin ()));
		connect (accObj,
				SIGNAL (entryPosted (QList<Entry>)),
				this,
				SLOT (handleEntryPosted (QList<Entry>)));
		connect (accObj,
				SIGNAL (entryRemoved (int)),
				this,
				SLOT (handleEntryRemoved (int)));
		connect (accObj,
				SIGNAL (entryUpdated (QList<Entry>)),
				this,
				SLOT (handleEntryUpdated (QList<Entry>)));
		connect (accObj,
				SIGNAL (tagsUpdated (QHash<QString, int>)),
				this,
				SIGNAL (tagsUpdated (QHash<QString, int>)));
		connect (accObj,
				SIGNAL (gotRecentComments (QList<CommentEntry>)),
				CommentsManager_,
				SLOT (handleGotRecentComments (QList<CommentEntry>)));
		connect (accObj,
				SIGNAL (commentsDeleted (QList<qint64>)),
				CommentsManager_,
				SLOT (handleCommentsDeleted (QList<qint64>)));
		connect (accObj,
				SIGNAL (gotError (int, QString, QString)),
				this,
				SIGNAL (gotError (int, QString, QString)));

		emit accountAdded (accObj);
	}

	void Core::handleAccountRemoved (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		emit accountRemoved (accObj);
	}

	void Core::handleAccountValidated (QObject *accObj, bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		emit accountValidated (accObj, validated);
	}

	void Core::updateProfiles ()
	{
		for (auto acc : GetAccounts ())
			acc->updateProfile ();
	}

	void Core::handleEntryPosted (const QList<Entry>& entries)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		auto e = Util::MakeNotification ("Blogique",
				tr ("Entry was posted successfully:") +
					QString (" <a href=\"%1\">%1</a>\n")
						.arg (entries.value (0).EntryUrl_.toString ()),
				Priority::Info);

		auto nh = new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open Link"),
				[this, entries] ()
				{
					auto urlEntity = Util::MakeEntity (entries.value (0).EntryUrl_,
							{},
							OnlyHandle | FromUserInitiated);
					SendEntity (urlEntity);
				});
		emit gotEntity (e);
		acc->RequestStatistics ();
		acc->RequestTags ();
		emit entryPosted ();
	}

	void Core::handleEntryRemoved (int)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		SendEntity (Util::MakeNotification ("Blogique",
				tr ("Entry was removed successfully."),
				Priority::Info));
		acc->RequestStatistics ();
		acc->RequestTags ();
		emit entryRemoved ();
	}

	void Core::handleEntryUpdated (const QList<Entry>& entries)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		if (entries.isEmpty ())
			return;

		SendEntity (Util::MakeNotification ("Blogique",
				tr ("Entry was updated successfully."),
				Priority::Info));
		acc->RequestStatistics ();
		acc->RequestTags ();
	}

	void Core::handleAutoSaveIntervalChanged ()
	{
		AutoSaveTimer_->start (XmlSettingsManager::Instance ()
				.property ("AutoSave").toInt () * 1000);
	}

	void Core::exportBlog ()
	{
		ExportWizard *wizard = new ExportWizard (Proxy_->GetRootWindowsManager ()->
				GetPreferredWindow ());
		wizard->setWindowTitle (tr ("Export blog"));
		wizard->show ();
	}
}
}

