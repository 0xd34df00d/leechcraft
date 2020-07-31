/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>

namespace LC
{
namespace Blogique
{
	/** @brief Interface representing a profile widget.
	*
	**/
	class IProfileWidget
	{
	public:
		virtual ~IProfileWidget () {}

		virtual void updateProfile () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blogique::IProfileWidget,
		"org.Deviant.LeechCraft.Blogique.IProfileWidget/1.0")
