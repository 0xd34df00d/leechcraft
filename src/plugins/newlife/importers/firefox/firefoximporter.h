/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "abstractimporter.h"
#include "firefoxprofileselectpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	class FirefoxImportPage;

	class FirefoxImporter : public AbstractImporter
	{
		FirefoxImportPage *ImportPage_;
		FirefoxProfileSelectPage *ProfileSelectPage_;
	public:
		FirefoxImporter (const ICoreProxy_ptr&, QWidget* = 0);

		QStringList GetNames () const;
		QStringList GetIcons () const;
		QList<QWizardPage*> GetWizardPages () const;
	};
}
}
}
