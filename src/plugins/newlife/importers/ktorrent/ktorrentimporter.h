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
	class KTorrentImportPage;

	class KTorrentImporter : public AbstractImporter
	{
		KTorrentImportPage * const ImportPage_;
	public:
		KTorrentImporter (const ICoreProxy_ptr& proxy, QWidget* = nullptr);

		QStringList GetNames () const override;
		QStringList GetIcons () const override;
		QList<QWizardPage*> GetWizardPages () const override;
	};
}
}
}
