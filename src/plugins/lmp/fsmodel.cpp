/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fsmodel.h"
#include <QFileIconProvider>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	class FSIconProvider : public QFileIconProvider
	{
	public:
		QIcon icon (const QFileInfo& info) const
		{
			if (!info.isDir ())
				return QFileIconProvider::icon (info);

			const auto& path = info.absoluteFilePath ();
			const auto status = Core::Instance ().GetLocalCollection ()->GetDirStatus (path);
			if (status != LocalCollection::DirStatus::None)
				return GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("folder-bookmark");

			return QFileIconProvider::icon (info);
		}
	};

	FSModel::FSModel (QObject *parent)
	: DndActionsMixin<QFileSystemModel> (parent)
	{
		setIconProvider (new FSIconProvider);

		setSupportedDragActions (Qt::CopyAction);
	}
}
}
