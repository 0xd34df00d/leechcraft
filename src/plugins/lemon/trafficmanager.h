/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <QVector>

class QStandardItem;
class QNetworkConfiguration;
class QNetworkConfigurationManager;
class QAbstractItemModel;
class QStandardItemModel;

class QNetworkSession;
typedef std::shared_ptr<QNetworkSession> QNetworkSession_ptr;

namespace LC
{
namespace Lemon
{
	class PlatformBackend;

	class TrafficManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QNetworkConfigurationManager *ConfManager_;

		std::shared_ptr<PlatformBackend> Backend_;

		struct InterfaceInfo
		{
			QString Name_;

			QStandardItem *Item_;
			qint64 PrevRead_;
			qint64 PrevWritten_;

			QNetworkSession_ptr LastSession_;

			QVector<qint64> DownSpeeds_;
			QVector<qint64> UpSpeeds_;

			InterfaceInfo (QStandardItem *item = 0)
			: Item_ (item)
			, PrevRead_ (0)
			, PrevWritten_ (0)
			{
			}
		};
		QHash<QString, InterfaceInfo> ActiveInterfaces_;
	public:
		TrafficManager (std::shared_ptr<PlatformBackend>, QObject* = 0);

		QAbstractItemModel* GetModel () const;

		QVector<qint64> GetDownHistory (const QString&) const;
		QVector<qint64> GetUpHistory (const QString&) const;

		int GetBacktrackSize () const;
	private slots:
		void addConfiguration (const QNetworkConfiguration&);
		void removeConfiguration (const QNetworkConfiguration&);
		void handleConfigChanged (const QNetworkConfiguration&);

		void updateCounters ();
	signals:
		void updated ();
	};
}
}
