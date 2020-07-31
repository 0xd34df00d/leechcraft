/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/media/iradiostationprovider.h>

namespace LC
{
namespace HotStreams
{
	enum StreamItemRoles
	{
		PristineName = Media::RadioItemRole::MaxRadioRole + 1,
		PlaylistFormat,
		UrlList
	};
}
}
