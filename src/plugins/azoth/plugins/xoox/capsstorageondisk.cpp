/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "capsstorageondisk.h"
#include <QHash>
#include <QDir>
#include <QDataStream>
#include <QElapsedTimer>
#include <QSqlError>
#include <QXmppDiscoveryIq.h>
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
#include <util/sys/paths.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include "util.h"

Q_DECLARE_METATYPE (QXmppDiscoveryIq::Identity);

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	CapsStorageOnDisk::CapsStorageOnDisk (const ILoadProgressReporter_ptr& lpr, QObject *parent)
	: QObject { parent }
	{
		qRegisterMetaType<QXmppDiscoveryIq::Identity> ("QXmppDiscoveryIq::Identity");
		qRegisterMetaTypeStreamOperators<QXmppDiscoveryIq::Identity> ("QXmppDiscoveryIq::Identity");

		DB_.setDatabaseName (Util::CreateIfNotExists ("azoth/xoox").filePath ("caps2.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		InitTables ();
		InitQueries ();

		Migrate (lpr);
	}

	namespace
	{
		QByteArray SerializeFeatures (const QStringList& features)
		{
			QByteArray result;

			QDataStream str { &result, QIODevice::WriteOnly };
			str << features;

			return result;
		}

		QStringList DeserializeFeatures (const QByteArray& data)
		{
			QStringList result;

			QDataStream str { data };
			str >> result;

			return result;
		}
	}

	std::optional<QStringList> CapsStorageOnDisk::GetFeatures (const QByteArray& ver) const
	{
		SelectFeatures_.bindValue (":ver", ver);
		Util::DBLock::Execute (SelectFeatures_);

		const auto finish = Util::MakeScopeGuard ([this] { SelectFeatures_.finish (); });

		if (!SelectFeatures_.next ())
			return {};
		else
			return DeserializeFeatures (SelectFeatures_.value (0).toByteArray ());
	}

	std::optional<QList<QXmppDiscoveryIq::Identity>> CapsStorageOnDisk::GetIdentities (const QByteArray& ver) const
	{
		SelectIdentities_.bindValue (":ver", ver);
		Util::DBLock::Execute (SelectIdentities_);

		QList<QXmppDiscoveryIq::Identity> result;
		while (SelectIdentities_.next ())
		{
			QXmppDiscoveryIq::Identity id;
			id.setCategory (SelectIdentities_.value (0).toString ());
			id.setLanguage (SelectIdentities_.value (1).toString ());
			id.setName (SelectIdentities_.value (2).toString ());
			id.setType (SelectIdentities_.value (3).toString ());
			result << id;
		}

		SelectIdentities_.finish ();

		return result;
	}

	void CapsStorageOnDisk::AddFeatures (const QByteArray& ver, const QStringList& features)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();

		InsertFeatures_.bindValue (":ver", ver);
		InsertFeatures_.bindValue (":features", SerializeFeatures (features));
		Util::DBLock::Execute (InsertFeatures_);

		lock.Good ();
	}

	void CapsStorageOnDisk::AddIdentities (const QByteArray& ver,
			const QList<QXmppDiscoveryIq::Identity>& identities)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();

		for (const auto& id : identities)
		{
			InsertIdentity_.bindValue (":ver", ver);
			InsertIdentity_.bindValue (":category", id.category ());
			InsertIdentity_.bindValue (":language", id.language ());
			InsertIdentity_.bindValue (":name", id.name ());
			InsertIdentity_.bindValue (":type", id.type ());
			Util::DBLock::Execute (InsertIdentity_);
		}

		lock.Good ();
	}

	void CapsStorageOnDisk::InitTables ()
	{
		if (DB_.tables ().contains ("Features"))
			return;

		Util::DBLock lock { DB_ };
		lock.Init ();

		Util::RunQuery (DB_, "azoth/xoox", "create_features");
		Util::RunQuery (DB_, "azoth/xoox", "create_identities");

		lock.Good ();
	}

	void CapsStorageOnDisk::InitQueries ()
	{
		InsertFeatures_ = QSqlQuery { DB_ };
		InsertFeatures_.prepare (Util::LoadQuery ("azoth/xoox", "insert_feature"));

		InsertIdentity_ = QSqlQuery { DB_ };
		InsertIdentity_.prepare (Util::LoadQuery ("azoth/xoox", "insert_identity"));

		SelectFeatures_ = QSqlQuery { DB_ };
		SelectFeatures_.prepare (Util::LoadQuery ("azoth/xoox", "select_features"));

		SelectIdentities_ = QSqlQuery { DB_ };
		SelectIdentities_.prepare (Util::LoadQuery ("azoth/xoox", "select_identities"));
	}

	void CapsStorageOnDisk::Migrate (const ILoadProgressReporter_ptr& lpr)
	{
		QFile file { Util::CreateIfNotExists ("azoth/xoox").filePath ("caps_s.db") };
		if (!file.exists ())
			return;

		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open file for reading"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QHash<QByteArray, QStringList> features;
		QHash<QByteArray, QList<QXmppDiscoveryIq::Identity>> identities;

		QDataStream stream { &file };
		quint8 ver = 0;
		stream >> ver;
		if (ver < 1 || ver > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown storage version"
					<< ver;
			return;
		}

		stream >> features;
		if (ver >= 2)
			stream >> identities;

		const auto& proc = lpr->InitiateProcess (tr ("Migrating capabilities database..."),
				0, features.size () + identities.size ());

		QElapsedTimer timer;
		timer.start ();

		Util::DBLock lock { DB_ };
		lock.Init ();

		for (const auto& pair : Util::Stlize (features))
		{
			AddFeatures (pair.first, pair.second);
			++*proc;
		}

		for (const auto& pair : Util::Stlize (identities))
		{
			AddIdentities (pair.first, pair.second);
			++*proc;
		}

		lock.Good ();

		qDebug () << Q_FUNC_INFO
				<< "migration of"
				<< features.size ()
				<< "features and"
				<< identities.size ()
				<< "identities took"
				<< timer.elapsed ()
				<< "ms";

		file.remove ();
	}
}
}
}
