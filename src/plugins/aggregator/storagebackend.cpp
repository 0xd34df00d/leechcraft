/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storagebackend.h"
#include <stdexcept>
#include <QDebug>
#include "sqlstoragebackend.h"
#include "storagebackendmanager.h"

namespace LC
{
namespace Aggregator
{
	StorageBackend_ptr StorageBackend::Create (const QString& strType, const QString& id)
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

	StorageBackend_ptr StorageBackend::Create (Type type, const QString& id)
	{
		StorageBackend_ptr result;
		switch (type)
		{
		case SBSQLite:
		case SBPostgres:
			result = std::make_shared<SQLStorageBackend> (type, id);
			break;
		case SBMysql:
			throw std::runtime_error { "MySQL backend not supported yet, sorry" };
		}
		qDebug () << Q_FUNC_INFO
				<< "created connection";

		StorageBackendManager::Instance ().Register (result);

		return result;
	}
}
}
