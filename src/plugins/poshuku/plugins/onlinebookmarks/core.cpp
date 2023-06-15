/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QStandardItemModel>
#include <QDateTime>
#include <QTimer>
#include <interfaces/iplugin2.h>
#include <interfaces/ipersistentstorageplugin.h>
#include <interfaces/iserviceplugin.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/poshuku/iproxyobject.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/xpc/passutils.h>
#include "accountssettings.h"
#include "pluginmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	Core::Core ()
	: PluginManager_ (new PluginManager)
	, AccountsSettings_ (new AccountsSettings)
	, DownloadTimer_ (new QTimer (this))
	, UploadTimer_ (new QTimer (this))
	{
		DownloadTimer_->setSingleShot (true);
		connect (DownloadTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkDownloadPeriod ()),
				Qt::UniqueConnection);

		UploadTimer_->setSingleShot (true);
		connect (UploadTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkUploadPeriod ()),
				Qt::UniqueConnection);
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return CoreProxy_;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	AccountsSettings* Core::GetAccountsSettingsWidget () const
	{
		return AccountsSettings_;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin";
		return classes;
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

		PluginManager_->AddPlugin (plugin);

		const QSet<QByteArray>& classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin"))
		{
			IServicePlugin *service = qobject_cast<IServicePlugin*> (plugin);
			if (!service)
			{
				qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< plugin
						<< "tells it implements the IServicePlugin but cast failed";
				return;
			}
			AddServicePlugin (service->GetBookmarksService ());
		}
	}

	void Core::AddServicePlugin (QObject *plugin)
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
		if (!ibs)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "is not an IBookmarksService";
			return;
		}

		ServicesPlugins_ << plugin;

		connect (plugin,
				SIGNAL (gotBookmarks (QObject*, const QVariantList&)),
				this,
				SLOT (handleGotBookmarks (QObject*, const QVariantList&)));

		connect (plugin,
				SIGNAL (bookmarksUploaded ()),
				this,
				SLOT (handleBookmarksUploaded ()));
	}

	QObjectList Core::GetServicePlugins () const
	{
		return ServicesPlugins_;
	}

	void Core::AddActiveAccount (QObject *obj)
	{
		if (!ActiveAccounts_.contains (obj))
			ActiveAccounts_ << obj;
	}

	void Core::SetActiveAccounts (QObjectList list)
	{
		ActiveAccounts_ = list;
	}

	QObjectList Core::GetActiveAccounts () const
	{
		return ActiveAccounts_;
	}

	void Core::DeletePassword (QObject *accObj)
	{
		auto account = qobject_cast<IAccount*> (accObj);
		const auto& key = account->GetAccountID ();

		const auto& plugins = CoreProxy_->GetPluginsManager ()->GetAllCastableTo<IPersistentStoragePlugin*> ();
		for (const auto plugin : plugins)
			if (const auto& storage = plugin->RequestStorage ())
				storage->Set (key, {});
	}

	QString Core::GetPassword (QObject *accObj)
	{
		const auto account = qobject_cast<IAccount*> (accObj);
		const auto service = qobject_cast<IBookmarksService*> (account->GetParentService ());
		return Util::GetPassword (account->GetAccountID (),
				tr ("Enter password for account %1 at service %2:")
					.arg ("<em>" + account->GetLogin () + "</em>")
					.arg ("<em>" + service->GetServiceName () + "</em>"),
				CoreProxy_);
	}

	void Core::SavePassword (QObject *accObj)
	{
		const auto account = qobject_cast<IAccount*> (accObj);
		Util::SavePassword (account->GetPassword (), account->GetAccountID (), CoreProxy_);
	}

	QModelIndex Core::GetServiceIndex (QObject *object) const
	{
		for (auto [item, service] : Util::Stlize (Item2Service_))
			if (service == qobject_cast<IBookmarksService*> (object))
				return item->index ();

		return {};
	}

	QObject* Core::GetBookmarksModel () const
	{
		IProxyObject *obj = qobject_cast<IProxyObject*> (PluginProxy_);
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "obj is not an IProxyObject"
					<< PluginProxy_;
			return 0;
		}

		return obj->GetFavoritesModel ();
	}

	QVariantList Core::GetUniqueBookmarks (IAccount *account,
			const QVariantList& allBookmarks, bool byService)
	{
		QVariantList list;
		for (const auto& bookmark : allBookmarks)
		{
			QString url = bookmark.toMap () ["URL"].toString ();
			if (url.isEmpty ())
				continue;

			if (!Url2Account_.contains (url))
				list << bookmark;
			else if (byService &&
					Url2Account_ [url] != account &&
					Url2Account_ [url]->GetParentService () == account->GetParentService ())
				list << bookmark;
		}
		return list;
	}

	QVariantList Core::GetAllBookmarks () const
	{
		QVariantList result;
		if (!QMetaObject::invokeMethod (GetBookmarksModel (),
				"getItemsMap",
				Q_RETURN_ARG (QList<QVariant>, result)))
			qWarning () << Q_FUNC_INFO
					<< "getItemsMap() metacall failed"
					<< result;

		return result;
	}

	void Core::handleGotBookmarks (QObject *accObj, const QVariantList& importBookmarks)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "isn't an IAccount object"
					<< accObj;
			return;
		}

		const auto iem = GetProxyHolder ()->GetEntityManager ();

		auto eBookmarks = Util::MakeEntity ({},
				{},
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/browser-import-data");
		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		iem->HandleEntity (eBookmarks);

		for (const auto& bookmark : importBookmarks)
			Url2Account_ [bookmark.toMap () ["URL"].toString ()] = account;

		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
			return;

		auto e = Util::MakeNotification ("OnlineBookmarks",
				ibs->GetServiceName () + ": bookmarks downloaded successfully",
				Priority::Info);
		iem->HandleEntity (e);
		AccountsSettings_->UpdateDates ();
	}

	void Core::handleBookmarksUploaded ()
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
			return;

		auto e = Util::MakeNotification ("OnlineBookmarks",
				ibs->GetServiceName () + ": bookmarks uploaded successfully",
				Priority::Info);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		AccountsSettings_->UpdateDates ();
	}

	void Core::syncBookmarks ()
	{
		downloadBookmarks ();
		uploadBookmarks ();
	}

	void Core::uploadBookmarks ()
	{
		QVariantList result = GetAllBookmarks ();
		IAccount *account = 0;

		for (const auto accObj : ActiveAccounts_)
		{
			account = qobject_cast<IAccount*> (accObj);
			if (!account)
				continue;
			account->SetLastUploadDateTime (QDateTime::currentDateTime ());
		}

		if (result.isEmpty ())
			return;

		const int type = XmlSettingsManager::Instance ()->Property ("UploadType", 0).toInt ();
		IBookmarksService *ibs = 0;
		switch (type)
		{
		case 0:
			for (auto accObj : ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;
				ibs->UploadBookmarks (account->GetQObject (), result);
			}
			break;
		case 1:
			for (auto accObj : ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;

				QVariantList list = GetUniqueBookmarks (account, result);
				ibs->UploadBookmarks (account->GetQObject (), list);
			}
			break;
		case 2:
			for (auto accObj : ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;

				QVariantList list = GetUniqueBookmarks (account, result, true);
				ibs->UploadBookmarks (account->GetQObject (), list);
			}
			break;
		}
	}

	void Core::downloadBookmarks ()
	{
		for (auto accObj : ActiveAccounts_)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			ibs->DownloadBookmarks (account->GetQObject (), account->GetLastDownloadDateTime ());
		}
	}

	void Core::downloadAllBookmarks ()
	{
		for (auto accObj : ActiveAccounts_)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			ibs->DownloadBookmarks (account->GetQObject (), QDateTime ());
		}
	}

	void Core::checkDownloadPeriod ()
	{
		long downloadPeriod = XmlSettingsManager::Instance ()->
				property ("DownloadPeriod").toInt () * 900;
		long lastCheckTimeInSec = XmlSettingsManager::Instance ()->
				Property ("LastDownloadCheck", 0).toInt ();

		long diff = lastCheckTimeInSec + downloadPeriod - QDateTime::currentSecsSinceEpoch ();
		if (diff > 0)
			DownloadTimer_->start (diff * 1000);
		else
		{
			downloadBookmarks ();
			XmlSettingsManager::Instance ()->setProperty ("LastDownloadCheck", QDateTime::currentSecsSinceEpoch ());
			DownloadTimer_->start (downloadPeriod * 1000);
		}
	}

	void Core::checkUploadPeriod ()
	{
		long uploadPeriod = XmlSettingsManager::Instance ()->
				property ("UploadPeriod").toInt () * 900;
		long lastCheckTimeInSec = XmlSettingsManager::Instance ()->
				Property ("LastUploadCheck", 0).toInt ();

		long diff = lastCheckTimeInSec + uploadPeriod - QDateTime::currentSecsSinceEpoch ();
		if (diff > 0)
			UploadTimer_->start (diff * 1000);
		else
		{
			uploadBookmarks ();
			XmlSettingsManager::Instance ()->setProperty ("LastUploadCheck", QDateTime::currentSecsSinceEpoch ());
			UploadTimer_->start (uploadPeriod * 1000);
		}
	}

}
}
}
