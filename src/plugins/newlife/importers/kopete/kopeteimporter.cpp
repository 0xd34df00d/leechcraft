/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kopeteimporter.h"
#include "kopeteimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	KopeteImporter::KopeteImporter (const ICoreProxy_ptr& proxy, QObject *parent)
	: IMImporter ("Kopete", "kopete", new KopeteImportPage { proxy }, parent)
	{
	}
}
}
}
