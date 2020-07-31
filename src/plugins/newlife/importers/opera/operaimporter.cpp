/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "operaimporter.h"
#include "operaimportselectpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	OperaImporter::OperaImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter (parent)
	{
		ImportSelectPage_ = new OperaImportSelectPage (proxy);
	}

	QStringList OperaImporter::GetNames () const
	{
		return { "Opera" };
	}

	QStringList OperaImporter::GetIcons () const
	{
		return { "opera" };
	}

	QList<QWizardPage*> OperaImporter::GetWizardPages () const
	{
		return { ImportSelectPage_ };
	}
}
}
}
