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

#pragma once

#include <memory>
#include <QObject>
#include <QHash>

class QStandardItem;
class QNetworkConfiguration;
class QNetworkConfigurationManager;
class QAbstractItemModel;
class QStandardItemModel;

class QNetworkSession;
typedef std::shared_ptr<QNetworkSession> QNetworkSession_ptr;

namespace LeechCraft
{
namespace Lemon
{
	class ModelManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QNetworkConfigurationManager *ConfManager_;

		struct InterfaceInfo
		{
			QStandardItem *Item_;
			qint64 PrevRead_;
			qint64 PrevWritten_;

			QNetworkSession_ptr LastSession_;

			QList<qint64> DownSpeeds_;
			QList<qint64> UpSpeeds_;

			InterfaceInfo (QStandardItem *item = 0)
			: Item_ (item)
			, PrevRead_ (0)
			, PrevWritten_ (0)
			{
			}
		};
		QHash<QString, InterfaceInfo> ActiveInterfaces_;
	public:
		ModelManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;
	private slots:
		void addConfiguration (const QNetworkConfiguration&);
		void updateCounters ();
	};
}
}
