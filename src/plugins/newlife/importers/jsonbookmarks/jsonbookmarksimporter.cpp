/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jsonbookmarksimporter.h"
#include "jsonbookmarksimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	JsonBookmarksImporter::JsonBookmarksImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter { parent }
	, ImportPage_ { new JsonBookmarksImportPage { proxy } }
	{
	}

	QStringList JsonBookmarksImporter::GetNames () const
	{
		return { tr ("JSON bookmarks") };
	}

	QStringList JsonBookmarksImporter::GetIcons () const
	{
		return { "favorites" };
	}

	QList<QWizardPage*> JsonBookmarksImporter::GetWizardPages () const
	{
		return { ImportPage_ };
	}
}
}
}
