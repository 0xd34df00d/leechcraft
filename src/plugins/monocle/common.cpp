/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "common.h"
#include <QString>
#include <QtDebug>
#include <util/sll/unreachable.h>

namespace LC::Monocle
{
	QString LayoutMode2Name (LayoutMode mode)
	{
		switch (mode)
		{
		case LayoutMode::OnePage:
			return "one";
		case LayoutMode::TwoPages:
			return "two";
		case LayoutMode::TwoPagesShifted:
			return "twoshifted";
		}

		Util::Unreachable ();
	}

	LayoutMode Name2LayoutMode (const QString& name)
	{
		if (name == "one")
			return LayoutMode::OnePage;
		if (name == "two")
			return LayoutMode::TwoPages;
		if (name == "twoshifted")
			return LayoutMode::TwoPagesShifted;

		qWarning () << Q_FUNC_INFO
				<< "unknown layout mode"
				<< name;
		return LayoutMode::OnePage;
	}
}
