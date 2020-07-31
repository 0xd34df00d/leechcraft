/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "abstractimporter.h"

namespace LC
{
namespace NewLife
{
namespace Common
{
	class IMImportPage;

	class IMImporter : public AbstractImporter
	{
		QString Name_;
		QString Icon_;
	protected:
		IMImportPage *Page_;

		IMImporter (const QString&, const QString&, IMImportPage*, QObject* = 0);
	public:
		QStringList GetNames () const override;
		QStringList GetIcons () const override;
		QList<QWizardPage*> GetWizardPages () const override;
	};
}
}
}
