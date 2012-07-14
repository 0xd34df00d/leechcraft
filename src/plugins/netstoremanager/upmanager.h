/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_NETSTOREMANAGER_UPMANAGER_H
#define PLUGINS_NETSTOREMANAGER_UPMANAGER_H
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QUrl>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LeechCraft
{
struct Entity;

namespace NetStoreManager
{
	class IStorageAccount;
	class IStoragePlugin;

	class UpManager : public QObject
	{
		Q_OBJECT

		QHash<IStorageAccount*, QStringList> Uploads_;
		QStandardItemModel *ReprModel_;
		QHash<IStorageAccount*, QHash<QString, QList<QStandardItem*>>> ReprItems_;
	public:
		UpManager (QObject* = 0);

		QAbstractItemModel* GetRepresentationModel () const;
	private:
		void RemovePending (const QString&);
		IStoragePlugin* GetSenderPlugin ();
	public slots:
		void handleUploadRequest (IStorageAccount*, const QString&);
	private slots:
		void handleGotURL (const QUrl&, const QString&);
		void handleError (const QString&, const QString&);
		void handleUpStatusChanged (const QString&, const QString&);
		void handleUpProgress (quint64, quint64, const QString&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}

#endif
