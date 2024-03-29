/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "fancytrayicon.h"

namespace LC::Util
{
	class FancyTrayIconImpl : public QObject
	{
	public:
		using QObject::QObject;

		virtual ~FancyTrayIconImpl () = default;

		virtual void UpdateIcon () = 0;
		virtual void UpdateTooltip () = 0;
		virtual void UpdateMenu () = 0;
		virtual void UpdateStatus () = 0;
	};
}
