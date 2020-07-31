/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

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

namespace LC
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
		void handleError (const QString& str, const QString& path);
		void handleUpStatusChanged (const QString& status, const QString& filePath);
		void handleUpFinished (const QByteArray& id, const QString& filePath);
		void handleUpProgress (quint64 done, quint64 total, const QString& filepath);
	signals:
		void fileUploaded (const QString&, const QUrl&);
	};
}
}
