/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storagemanager.h"
#include <QString>
#include <QUrl>
#include <QDir>
#include <QtDebug>

namespace LC
{
namespace HttHare
{
	AccessDeniedException::~AccessDeniedException () noexcept
	{
	}

	StorageManager::StorageManager ()
	{
	}

	QString StorageManager::ResolvePath (QUrl url) const
	{
		if (url.path ().startsWith ("/"))
			url.setPath (url.path ().mid (1));

		const auto& path = QUrl::fromLocalFile (QDir::homePath () + '/')
				.resolved (url).toLocalFile ();
		const QFileInfo fi { path };
		if (!fi.absoluteFilePath ().startsWith (QDir::homePath ()))
			throw AccessDeniedException ();

		return path;
	}
}
}
