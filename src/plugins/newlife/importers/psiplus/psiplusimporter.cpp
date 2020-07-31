/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "psiplusimporter.h"
#include "psiplusimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	PsiPlusImporter::PsiPlusImporter (const ICoreProxy_ptr& proxy, QObject *parent)
	: Common::IMImporter ("Psi+", "psi-plus", new PsiPlusImportPage (proxy), parent)
	{
	}
}
}
}
