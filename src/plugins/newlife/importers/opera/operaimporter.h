/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxyfwd.h>
#include "abstractimporter.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	class OperaImportSelectPage;

	class OperaImporter : public AbstractImporter
	{
		OperaImportSelectPage *ImportSelectPage_;
	public:
		OperaImporter (const ICoreProxy_ptr&, QWidget* = nullptr);

		QStringList GetNames () const override;
		QStringList GetIcons () const override;
		QList<QWizardPage*> GetWizardPages () const override;
	};
}
}
}
