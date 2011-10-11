/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "core.h"
#include <QStandardItemModel>
#include <QDateTime>
#include <interfaces/iplugin2.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/iserviceplugin.h>
#include <util/util.h>
#include "accountssettings.h"
#include "pluginmanager.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	Core::Core ()
	: PluginManager_ (new PluginManager)
	, AccountsSettings_ (new AccountsSettings)
	{
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
				SIGNAL (gotBookmarks (IAccount*, const QVariantList&)),
				this,
				SLOT (handleGotBookmarks (IAccount*, const QVariantList&)));

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

// 	void Core::UploadBookmark(const QString& title,
// 			const QString& url, const QStringList& tags)
// 	{
// 		Q_FOREACH (QObject *accObj, ActiveAccounts_)
// 		{
// 			IAccount *account = qobject_cast<IAccount*> (accObj);
// 			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
// 			QVariantList list;
// 			QVariantMap map;
// 			map ["Title"] = title;
// 			map ["Url"] = url;
// 			map ["Tags"] = tags;
// 			list << map;
// 			if (account->GetBookmarksDiff (list).isEmpty ())
// 				continue;
// 			ibs->UploadBookmarks (account, list);
// 		}
// 	}

	void Core::DeletePassword (QObject *accObj)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		QVariantList keys;
		keys << account->GetAccountID ();

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-clear");
		emit gotEntity (e);
	}

	QString Core::GetPassword (QObject *accObj)
	{
		QVariantList keys;
		IAccount *account = qobject_cast<IAccount*> (accObj);
		keys << account->GetAccountID ();

		const QVariantList& result = Util::GetPersistentData (keys, this);
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

	void Core::SavePassword (QObject *accObj)
	{
		QVariantList keys;
		IAccount *account = qobject_cast<IAccount*> (accObj);
		keys << account->GetAccountID ();

		QVariantList passwordVar;
		passwordVar << account->GetPassword ();
		QVariantList values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		emit gotEntity (e);
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
		Q_FOREACH (const QVariant& bookmark, allBookmarks)
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

	void Core::handleGotBookmarks (IAccount *account, const QVariantList& importBookmarks)
	{
		LeechCraft::Entity eBookmarks = Util::MakeEntity (QVariant (),
				QString (),
				static_cast<LeechCraft::TaskParameter> (FromUserInitiated | OnlyHandle),
				"x-leechcraft/browser-import-data");

		eBookmarks.Additional_ ["BrowserBookmarks"] = importBookmarks;
		emit gotEntity (eBookmarks);

		Q_FOREACH (const QVariant& bookmark, importBookmarks)
			Url2Account_ [bookmark.toMap () ["URL"].toString ()] = account;

		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
			return;

		LeechCraft::Entity e = Util::MakeNotification ("OnlineBookmarks",
				ibs->GetServiceName () + ": bookmarks downloaded successfully",
				PInfo_);
		emit gotEntity (e);
		AccountsSettings_->UpdateDates ();
	}

	void Core::handleBookmarksUploaded ()
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
			return;

		LeechCraft::Entity e = Util::MakeNotification ("OnlineBookmarks",
				ibs->GetServiceName () + ": bookmarks uploaded successfully",
				PInfo_);
		emit gotEntity (e);
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
		if (result.isEmpty ())
			return;

		const int type = XmlSettingsManager::Instance ()->Property ("UploadType", 0).toInt ();
		IAccount *account = 0;
		IBookmarksService *ibs = 0;
		switch (type)
		{
		case 0:
			Q_FOREACH (QObject *accObj, ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;
				ibs->UploadBookmarks (account->GetObject (), result);
			}
			break;
		case 1:
			Q_FOREACH (QObject *accObj, ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;

				QVariantList list = GetUniqueBookmarks (account, result);
				ibs->UploadBookmarks (account->GetObject (), list);
			}
			break;
		case 2:
			Q_FOREACH (QObject *accObj, ActiveAccounts_)
			{
				account = qobject_cast<IAccount*> (accObj);
				if (!account)
					continue;
				ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
				if (!ibs)
					continue;

				QVariantList list = GetUniqueBookmarks (account, result, true);
				ibs->UploadBookmarks (account->GetObject (), list);
			}
			break;
		}
	}

	void Core::downloadBookmarks ()
	{
		Q_FOREACH (QObject *accObj, ActiveAccounts_)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			ibs->DownloadBookmarks (account->GetObject (), account->GetLastDownloadDateTime ());
		}
	}

	void Core::downloadAllBookmarks ()
	{
		Q_FOREACH (QObject *accObj, ActiveAccounts_)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			ibs->DownloadBookmarks (account->GetObject (), QDateTime ());
		}
	}

}
}
}

