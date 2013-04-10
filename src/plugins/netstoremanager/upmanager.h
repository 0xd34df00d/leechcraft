/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <functional>
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QUrl>
#include <QSet>
#include <interfaces/core/icoreproxy.h>

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
		QSet<QString> Autoshare_;

		typedef std::function<void (QUrl, QByteArray)> URLHandler_f;
		QHash<QByteArray, QList<URLHandler_f>> URLHandlers_;

		ICoreProxy_ptr Proxy_;
	public:
		UpManager (ICoreProxy_ptr proxy, QObject* = 0);

		QAbstractItemModel* GetRepresentationModel () const;
		void ScheduleAutoshare (const QString&);
	private:
		void RemovePending (const QString&);
		IStoragePlugin* GetSenderPlugin ();
	public slots:
		void handleUploadRequest (IStorageAccount *isa, const QString& file,
				const QByteArray& id = QByteArray (), bool byHand = true);
	private slots:
		void handleGotURL (const QUrl& url, const QByteArray& id);
		void handleError (const QString& str, const QString& path);
		void handleUpStatusChanged (const QString& status, const QString& filePath);
		void handleUpFinished (const QByteArray& id, const QString& filePath);
		void handleUpProgress (quint64 done, quint64 total, const QString& filepath);
	signals:
		void fileUploaded (const QString&, const QUrl&);
	};
}
}

#endif
