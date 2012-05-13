/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "fsmodel.h"
#include <QFileIconProvider>
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
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
				return Core::Instance ().GetProxy ()->GetIcon ("folder-bookmark");

			return QFileIconProvider::icon (info);
		}
	};

	FSModel::FSModel (QObject *parent)
	: QFileSystemModel (parent)
	{
		setIconProvider (new FSIconProvider);
	}
}
}
