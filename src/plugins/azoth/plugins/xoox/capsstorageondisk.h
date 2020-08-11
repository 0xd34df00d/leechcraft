/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QXmppDiscoveryIq.h>
#include <interfaces/core/iloadprogressreporter.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class CapsStorageOnDisk : public QObject
	{
		QSqlDatabase DB_ = QSqlDatabase::addDatabase ("QSQLITE", "org.LeechCraft.Azoth.Xoox.Caps");

		QSqlQuery InsertFeatures_;
		QSqlQuery InsertIdentity_;

		mutable QSqlQuery SelectFeatures_;
		mutable QSqlQuery SelectIdentities_;
	public:
		CapsStorageOnDisk (const ILoadProgressReporter_ptr&, QObject* = nullptr);

		std::optional<QStringList> GetFeatures (const QByteArray&) const;
		std::optional<QList<QXmppDiscoveryIq::Identity>> GetIdentities (const QByteArray&) const;

		void AddFeatures (const QByteArray&, const QStringList&);
		void AddIdentities (const QByteArray&, const QList<QXmppDiscoveryIq::Identity>&);
	private:
		void InitTables ();
		void InitQueries ();

		void Migrate (const ILoadProgressReporter_ptr&);
	};
}
}
}
