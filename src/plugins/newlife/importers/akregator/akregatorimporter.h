/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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
	class AkregatorImportPage;

	class AkregatorImporter : public AbstractImporter
	{
		AkregatorImportPage *ImportPage_;
	public:
		AkregatorImporter (const ICoreProxy_ptr&, QWidget* = nullptr);

		QStringList GetNames () const override;
		QStringList GetIcons () const override;
		QList<QWizardPage*> GetWizardPages () const override;
	};
}
}
}
