/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
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
		QObject *PluginProxy_ = nullptr;
		std::shared_ptr<PluginManager> PluginManager_;
		AccountsSettings *AccountsSettings_;

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

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		void AddServicePlugin (QObject*);
		QObjectList GetServicePlugins () const;

		void AddActiveAccount (QObject*);
		void SetActiveAccounts (QObjectList);
		QObjectList GetActiveAccounts () const;

		void DeletePassword (QObject*);
		QString GetPassword (QObject*);
		void SavePassword (QObject*);

		QModelIndex GetServiceIndex (QObject*) const;
		QVariantList GetAllBookmarks () const;
	private:
		QObject* GetBookmarksModel () const;
		QVariantList GetUniqueBookmarks (IAccount*,
				const QVariantList&, bool byService = false);
	private slots:
		void handleGotBookmarks (QObject*, const QVariantList&);
		void handleBookmarksUploaded ();
	public slots:
		void syncBookmarks ();
		void uploadBookmarks ();
		void downloadBookmarks ();
		void downloadAllBookmarks ();

		void checkDownloadPeriod ();
		void checkUploadPeriod ();
	signals:
		void gotEntity (const LC::Entity&);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
