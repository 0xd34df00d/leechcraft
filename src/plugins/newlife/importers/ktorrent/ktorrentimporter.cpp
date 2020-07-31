/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ktorrentimporter.h"
#include "ktorrentimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	KTorrentImporter::KTorrentImporter (const ICoreProxy_ptr& proxy, QWidget *parent)
	: AbstractImporter { parent }
	, ImportPage_ { new KTorrentImportPage { proxy } }
	{
	}

	QStringList KTorrentImporter::GetNames () const
	{
		return { "KTorrent" };
	}

	QStringList KTorrentImporter::GetIcons () const
	{
		return { "ktorrent" };
	}

	QList<QWizardPage*> KTorrentImporter::GetWizardPages () const
	{
		return { ImportPage_ };
	}
}
}
}
