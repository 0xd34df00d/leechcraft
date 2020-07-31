/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storagebackend.h"
#include <stdexcept>
#include <util/sll/unreachable.h>
#include "sqlstoragebackend.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
	std::shared_ptr<StorageBackend> StorageBackend::Create (Type type)
	{
		switch (type)
		{
		case SBSQLite:
		case SBPostgres:
			return std::make_shared<SQLStorageBackend> (type);
		}

		Util::Unreachable ();
	}

	std::shared_ptr<StorageBackend> StorageBackend::Create ()
	{
		StorageBackend::Type type;
		const auto& strType = XmlSettingsManager::Instance ()->property ("StorageType").toString ();
		if (strType == "SQLite")
			type = StorageBackend::SBSQLite;
		else if (strType == "PostgreSQL")
			type = StorageBackend::SBPostgres;
		else
			throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
						.arg (strType)));

		return Create (type);
	}
}
}
