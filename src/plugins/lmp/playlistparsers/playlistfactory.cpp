/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistfactory.h"
#include "m3u.h"
#include "xspf.h"
#include "pls.h"

namespace LC
{
namespace LMP
{
	PlaylistParser_f MakePlaylistParser (const QString& file)
	{
		if (file.endsWith ("m3u") || file.endsWith ("m3u8"))
			return M3U::Read2Sources;
		else if (file.endsWith ("xspf"))
			return XSPF::Read2Sources;
		else if (file.endsWith ("pls"))
			return PLS::Read2Sources;

		return PlaylistParser_f ();
	}
}
}
