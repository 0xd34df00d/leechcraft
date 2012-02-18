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

#include "storagebackend.h"
#include <stdexcept>
#include <QFile>
#include <QDebug>
#include "sqlstoragebackend.h"
#include "sqlstoragebackend_mysql.h"

namespace LeechCraft
{
namespace Aggregator
{
	QString StorageBackend::LoadQuery (const QString& engine, const QString& name)
	{
		QFile file (QString (":/resources/sql/%1/%2.sql")
				.arg (engine)
				.arg (name));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< name
					<< "for engine"
					<< engine
					<< "for reading";
			return QString ();
		}
		return file.readAll ();
	}

	StorageBackend::StorageBackend (QObject *parent)
	: QObject (parent)
	{
	}

	StorageBackend::~StorageBackend ()
	{
	}

	std::shared_ptr<StorageBackend> StorageBackend::Create (const QString& strType, const QString& id)
	{
		StorageBackend::Type type;
		if (strType == "SQLite")
			type = StorageBackend::SBSQLite;
		else if (strType == "PostgreSQL")
			type = StorageBackend::SBPostgres;
		else if (strType == "MySQL")
			type = StorageBackend::SBMysql;
		else
			throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
						.arg (strType)));

		return Create (type, id);
	}

	std::shared_ptr<StorageBackend> StorageBackend::Create (Type type, const QString& id)
	{
		std::shared_ptr<StorageBackend> result;
		switch (type)
		{
			case SBSQLite:
			case SBPostgres:
				result.reset (new SQLStorageBackend (type, id));
				break;
			case SBMysql:
				result.reset (new SQLStorageBackendMysql (type, id));
		}
		return result;
	}
}
}
