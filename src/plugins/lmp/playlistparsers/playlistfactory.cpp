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

#include "playlistfactory.h"
#include "m3u.h"
#include "xspf.h"

namespace LeechCraft
{
namespace LMP
{
	PlaylistParser_f MakePlaylistParser (const QString& file)
	{
		if (file.endsWith ("m3u") || file.endsWith ("m3u8"))
			return M3U::Read2Sources;
		else if (file.endsWith ("xspf"))
			return XSPF::Read2Sources;

		return PlaylistParser_f ();
	}
}
}
