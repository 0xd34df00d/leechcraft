/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imimporter.h"
#include "imimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Common
{
	IMImporter::IMImporter (const QString& name, const QString& icon, IMImportPage *page, QObject *parent)
	: AbstractImporter (parent)
	, Name_ (name)
	, Icon_ (icon)
	, Page_ (page)
	{
	}

	QStringList IMImporter::GetNames () const
	{
		return { Name_ };
	}

	QStringList IMImporter::GetIcons () const
	{
		return { Icon_ };
	}

	QList<QWizardPage*> IMImporter::GetWizardPages () const
	{
		return { Page_ };
	}
}
}
}
