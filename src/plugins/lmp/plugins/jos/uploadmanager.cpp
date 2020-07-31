/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadmanager.h"
#include <QtDebug>
#include <interfaces/lmp/iunmountablesync.h>
#include "connection.h"

namespace LC
{
namespace LMP
{
namespace jOS
{
	UploadManager::UploadManager (QObject *parent)
	: QObject { parent }
	{
	}

	void UploadManager::SetInfo (const QString& origLocalPath, const UnmountableFileInfo& info)
	{
		Infos_ [origLocalPath] = info;
	}

	void UploadManager::Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& to)
	{
		if (!AvailableConnections_.contains (to))
			try
			{
				const auto& conn = std::make_shared<Connection> (to);
				AvailableConnections_ [to] = conn;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				emit uploadFinished (localPath,
						QFile::OpenError,
						 tr ("Unable to contact the device: %1.").arg (e.what ()));
				return;
			}

		AvailableConnections_ [to]->Upload (localPath, Infos_.take (origLocalPath));
	}
}
}
}
