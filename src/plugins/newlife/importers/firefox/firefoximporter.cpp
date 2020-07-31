/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "firefoximporter.h"
#include "firefoximportpage.h"
#include "firefoxprofileselectpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	FirefoxImporter::FirefoxImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter (parent)
	{
		ImportPage_ = new FirefoxImportPage ();
		ProfileSelectPage_ = new FirefoxProfileSelectPage (proxy);
	}

	QStringList FirefoxImporter::GetNames () const
	{
		return QStringList ("Firefox");
	}

	QStringList FirefoxImporter::GetIcons () const
	{
		return { "firefox" };
	}

	QList<QWizardPage*> FirefoxImporter::GetWizardPages () const
	{
		return { ImportPage_, ProfileSelectPage_ };
	}
}
}
}
