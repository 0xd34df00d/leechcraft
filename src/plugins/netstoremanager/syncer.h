/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include <boost/bimap.hpp>
#include <QObject>
#include "interfaces/netstoremanager/isupportfilelistings.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	class IStorageAccount;

	class Syncer : public QObject
	{
		Q_OBJECT

		QString BasePath_;
		IStorageAccount *Account_;
		QHash<QString, StorageItem> Id2Item_;
		boost::bimaps::bimap<QByteArray, QString> Id2Path_;

	public:
		explicit Syncer (const QString& dirPath, IStorageAccount *isa, QObject *parent = 0);

		QByteArray GetAccountID () const;
		QString GetBasePath () const;

		void SetItems (const QList<StorageItem>& items);

	public slots:
		void start ();
		void stop ();

		void dirWasCreated (const QString& path);
		void dirWasRemoved (const QString& path);
		void fileWasCreated (const QString& path);
		void fileWasRemoved (const QString& path);
		void fileWasUpdated (const QString& path);
	};
}
}
