/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lifereaimporter.h"
#include "lifereaimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	LifereaImporter::LifereaImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter (parent)
	{
		ImportPage_ = new LifereaImportPage (proxy);
	}

	QStringList LifereaImporter::GetNames () const
	{
		return { "Liferea" };
	}

	QStringList LifereaImporter::GetIcons () const
	{
		return { "liferea" };
	}

	QList<QWizardPage*> LifereaImporter::GetWizardPages () const
	{
		return { ImportPage_ };
	}
}
}
}
