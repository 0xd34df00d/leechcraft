/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "lmpproxy.h"
#include "core.h"
#include "util.h"
#include "localcollection.h"
#include "localfileresolver.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	LMPProxy::LMPProxy ()
	{
	}

	ILocalCollection* LMPProxy::GetLocalCollection () const
	{
		return Core::Instance ().GetLocalCollection ();
	}

	ITagResolver* LMPProxy::GetTagResolver () const
	{
		return Core::Instance ().GetLocalFileResolver ();
	}

	QString LMPProxy::FindAlbumArt (const QString& near, bool includeCollection) const
	{
		return FindAlbumArtPath (near, !includeCollection);
	}

	QList<QFileInfo> LMPProxy::RecIterateInfo (const QString& path, bool followSymLinks)
	{
		return LMP::RecIterateInfo (path, followSymLinks);
	}
}
}
