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
class QAbstractItemModel;
class QStandardItemModel;
class QNetworkInterface;

namespace LC
{
namespace Lemon
{
	class PlatformBackend;

	class TrafficManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;

		std::shared_ptr<PlatformBackend> Backend_;

		struct InterfaceInfo
		{
			QStandardItem *Item_ = nullptr;
			qint64 PrevRead_ = 0;
			qint64 PrevWritten_ = 0;

			QVector<qint64> DownSpeeds_;
			QVector<qint64> UpSpeeds_;
		};
		QHash<QString, InterfaceInfo> ActiveInterfaces_;
	public:
		TrafficManager (std::shared_ptr<PlatformBackend>, QObject* = 0);

		QAbstractItemModel* GetModel () const;

		QVector<qint64> GetDownHistory (const QString&) const;
		QVector<qint64> GetUpHistory (const QString&) const;

		int GetBacktrackSize () const;
	private:
		void AddInterface (const QNetworkInterface&);
		void UpdateInterface (const QNetworkInterface&);
		void RemoveInterface (const QString&);

		void UpdateInterfaces ();
		void UpdateCounters ();
	signals:
		void updated ();
	};
}
}
