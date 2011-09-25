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
#include <interfaces/iinfo.h>
#include <util/util.h>

class QAbstractItemModel;
class QStandardItemModel;

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
		AccountsSettings* AccountsSettings_;
		boost::shared_ptr<PluginManager> PluginManager_;

		QObjectList ServicesPlugins_;
		QObjectList ActiveAccounts_;
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

		void UploadBookmark (const QString&,
				const QString&, const QStringList&);

		void DeletePassword (QObject*);
		void SavePassword (QObject*);
		QString GetPassword (QObject*);
	private slots:
		void handleGotBookmarks (const QVariantList&);
	public slots:
		void syncBookmarks ();
		void uploadBookmarks ();
		void downloadBookmarks ();
		void downloadAllBookmarks ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_CORE_H
