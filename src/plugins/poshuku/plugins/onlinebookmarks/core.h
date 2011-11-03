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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H

#include <QObject>
#include <QUrl>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/iaccount.h>
#include <interfaces/ibookmarksservice.h>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{

	class PluginManager;
	class AccountsSettings;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		QObject *PluginProxy_;
		boost::shared_ptr<PluginManager> PluginManager_;
		AccountsSettings *AccountsSettings_;
		QStandardItemModel *QuickUploadModel_;

		QObjectList ServicesPlugins_;
		QObjectList ActiveAccounts_;
		QHash<QString, IAccount*> Url2Account_;

		QHash<QStandardItem*, IAccount*> Item2Account_;
		QHash<QStandardItem*, IBookmarksService*> Item2Service_;

		QTimer *DownloadTimer_;
		QTimer *UploadTimer_;

		Core ();
	public:
		static Core& Instance ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		void SetPluginProxy (QObject*);

		AccountsSettings* GetAccountsSettingsWidget () const;

		QAbstractItemModel* GetQuickUploadModel () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		void AddServicePlugin (QObject*);
		QObjectList GetServicePlugins () const;

		void AddActiveAccount (QObject*);
		void SetActiveAccounts (QObjectList);
		QObjectList GetActiveAccounts () const;

// 		void UploadBookmark (const QString&,
// 				const QString&, const QStringList&);
		void DeletePassword (QObject*);
		QString GetPassword (QObject*);
		void SavePassword (QObject*);

		void AddAccounts (QObjectList);
		QModelIndex GetServiceIndex (QObject*) const;

		void SetQuickUploadButtons ();
	private:
		QObject* GetBookmarksModel () const;
		QVariantList GetUniqueBookmarks (IAccount*,
				const QVariantList&, bool byService = false);
		QVariantList GetAllBookmarks () const;
	private slots:
		void handleGotBookmarks (QObject*, const QVariantList&);
		void handleBookmarksUploaded ();
		void handleItemChanged (QStandardItem*);
	public slots:
		void syncBookmarks ();
		void uploadBookmarks ();
		void downloadBookmarks ();
		void downloadAllBookmarks ();
		void removeAccount (QObject*);

		void checkDownloadPeriod ();
		void checkUploadPeriod ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
