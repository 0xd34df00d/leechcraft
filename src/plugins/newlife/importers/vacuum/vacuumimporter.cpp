/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vacuumimporter.h"
#include "vacuumimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	VacuumImporter::VacuumImporter (const ICoreProxy_ptr& proxy, QObject *obj)
	: Common::IMImporter ("Vacuum IM", "vacuum", new VacuumImportPage (proxy), obj)
	{
	}
}
}
}
