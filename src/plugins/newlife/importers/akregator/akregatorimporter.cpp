/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "akregatorimporter.h"
#include "akregatorimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	AkregatorImporter::AkregatorImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter (parent)
	{
		ImportPage_ = new AkregatorImportPage { proxy };
	}

	QStringList AkregatorImporter::GetNames () const
	{
		return { "Akregator" };
	}

	QStringList AkregatorImporter::GetIcons () const
	{
		return { "akregator" };
	}

	QList<QWizardPage*> AkregatorImporter::GetWizardPages () const
	{
		return { ImportPage_ };
	}
}
}
}
