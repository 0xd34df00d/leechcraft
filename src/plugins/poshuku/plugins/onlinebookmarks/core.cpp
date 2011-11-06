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
#include <QTimer>
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
	, QuickUploadModel_ (new QStandardItemModel)
	, DownloadTimer_ (new QTimer (this))
	, UploadTimer_ (new QTimer (this))
	{
		connect (QuickUploadModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)),
				Qt::UniqueConnection);

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

	QAbstractItemModel* Core::GetQuickUploadModel () const
	{
		return QuickUploadModel_;
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

	void Core::AddAccounts (QObjectList accObjects)
	{
		Q_FOREACH (QObject *accObj, accObjects)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			if (!account)
			{
				qWarning () << Q_FUNC_INFO
						<< "isn't an IAccount object"
						<< accObj;
				continue;
			}

			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			if (!ibs)
			{
				qWarning () << Q_FUNC_INFO
						<< "isn't an IBookmarksService"
						<< account->GetParentService ();
				continue;
			}

			const QModelIndex& index = GetServiceIndex (ibs->GetObject ());
			QStandardItem *parentItem = 0;
			if (!index.isValid ())
			{
				parentItem = new QStandardItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
				parentItem->setEditable (false);
				Item2Service_ [parentItem] = ibs;
				QuickUploadModel_->appendRow (parentItem);
			}
			else
				parentItem = QuickUploadModel_->itemFromIndex (index);

			QStandardItem *item = new QStandardItem (account->GetLogin ());
			item->setEditable (false);
			item->setCheckable (true);
			item->setCheckState (account->IsQuickUpload () ? Qt::Checked : Qt::Unchecked);
			Item2Account_ [item] = account;
			parentItem->appendRow (item);
		}
	}

	QModelIndex Core::GetServiceIndex (QObject *object) const
	{
		Q_FOREACH (QStandardItem *item, Item2Service_.keys ())
		if (Item2Service_ [item] == qobject_cast<IBookmarksService*> (object))
			return item->index ();

		return QModelIndex ();
	}

	void Core::SetQuickUploadButtons ()
	{

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

	void Core::handleItemChanged (QStandardItem *item)
	{
		if (item->checkState () == Qt::Unchecked)
		{
			Item2Account_ [item]->SetQuickUpload (false);
			return;
		}

		QStandardItem *parentItem = item->parent ();
		for (int i = 0; i < parentItem->rowCount (); ++i)
		{
			QStandardItem *childItem = parentItem->child (i);
			if (childItem != item &&
					childItem->checkState () == Qt::Checked)
			{
				childItem->setCheckState (Qt::Unchecked);
				if (Item2Account_.contains (childItem))
					Item2Account_ [childItem]->SetQuickUpload (false);
			}
		}

		Item2Account_ [item]->SetQuickUpload (true);
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

	void Core::removeAccount (QObject *accObj)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "isn't an IAccount object"
					<< accObj;
			return;
		}

		QStandardItem *item = Item2Account_.key (account);
		if (item)
		{
			const QModelIndex& parentIndex = item->parent ()->index ();
			QuickUploadModel_->removeRow (item->row (), parentIndex);
			if (!QuickUploadModel_->rowCount (parentIndex))
			{
				Item2Service_.remove (QuickUploadModel_->itemFromIndex (parentIndex));
				QuickUploadModel_->removeRow (parentIndex.row ());
			}
			Item2Account_.remove (item);
		}
	}

	void Core::checkDownloadPeriod ()
	{
		uint downloadPeriod = XmlSettingsManager::Instance ()->
				property ("DownloadPeriod").toInt () * 3600;
		uint lastCheckTimeInSec = XmlSettingsManager::Instance ()->
				Property ("LastDownloadCheck", 0).toInt ();

		uint diff = lastCheckTimeInSec + downloadPeriod - QDateTime::currentDateTime ().toTime_t ();
		if (diff > 0)
			DownloadTimer_->start (diff);
		else
		{
			downloadBookmarks ();
			XmlSettingsManager::Instance ()->setProperty ("LastDownloadCheck",
					QDateTime::currentDateTime ().toTime_t ());
			DownloadTimer_->start (downloadPeriod);
		}
	}

	void Core::checkUploadPeriod ()
	{
		uint uploadPeriod = XmlSettingsManager::Instance ()->
				property ("UploadPeriod").toInt () * 3600;
		uint lastCheckTimeInSec = XmlSettingsManager::Instance ()->
				Property ("LastUploadCheck", 0).toInt ();

		uint diff = lastCheckTimeInSec + uploadPeriod - QDateTime::currentDateTime ().toTime_t ();
		if (diff > 0)
			UploadTimer_->start (diff);
		else
		{
			uploadBookmarks ();
			XmlSettingsManager::Instance ()->setProperty ("LastUploadCheck",
					QDateTime::currentDateTime ().toTime_t ());
			DownloadTimer_->start (uploadPeriod);
		}
	}
}
}
}
