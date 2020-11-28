/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagenotification.h"
#include <stdexcept>
#include <QVBoxLayout>

namespace LC::Util
{
	PageNotification::PageNotification (QWidget *widget)
	: QWidget (widget)
	{
		auto lay = qobject_cast<QVBoxLayout*> (widget->parentWidget ()->layout ());
		if (!lay)
			throw std::runtime_error ("Passed parent object has no QVBoxLayout");

		const auto idx = lay->indexOf (widget);
		lay->insertWidget (idx + 1, this);
	}
}
