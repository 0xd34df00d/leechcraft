/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QShortcut>
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Util
{
	void CreateShortcuts (const QList<QKeySequence>& shortcuts,
			const std::function<void ()>& func, QWidget *parent)
	{
		for (const auto& sc : shortcuts)
		{
			const auto obj = new QShortcut { sc, parent };
			new Util::SlotClosure<Util::NoDeletePolicy>
			{
				func,
				obj,
				SIGNAL (activated ()),
				parent
			};
		}
	}

	void CreateShortcuts (const QList<QKeySequence>& shortcuts,
			QObject *object, const char *metamethod, QWidget *parent)
	{
		for (const auto& sc : shortcuts)
			QObject::connect (new QShortcut { sc, parent },
					SIGNAL (activated ()),
					object,
					metamethod);
	}
}
}
