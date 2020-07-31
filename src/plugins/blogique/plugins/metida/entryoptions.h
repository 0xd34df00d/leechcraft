/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Blogique
{
namespace Metida
{
	enum Access
	{
		Public,
		FriendsOnly,
		Private,
		Custom,

		MAXAccess
	};

	enum CommentsManagement
	{
		DisableComments,
		EnableComments,
		WithoutNotification,

		MAXManagment,

		Default,
		ShowComments,
		ShowFriendsComments,
		ScreenComments,
		ScreenAnonymouseComments,
		ScreenNotFromFriendsWithLinks,

		MAXScreening
	};

	enum AdultContent
	{
		WithoutAdultContent,
		AdultsFrom14,
		AdultsFrom18,

		MAXAdult
	};
}
}
}
