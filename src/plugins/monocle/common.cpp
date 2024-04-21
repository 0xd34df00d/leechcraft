/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "common.h"
#include <QString>
#include <QToolBar>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/unreachable.h>
#include <util/sll/visitor.h>

namespace LC::Monocle
{
	QString LayoutMode2Name (LayoutMode mode)
	{
		switch (mode)
		{
		case LayoutMode::OnePage:
			return "one"_qs;
		case LayoutMode::TwoPages:
			return "two"_qs;
		case LayoutMode::TwoPagesShifted:
			return "twoshifted"_qs;
		}

		Util::Unreachable ();
	}

	LayoutMode Name2LayoutMode (const QString& name)
	{
		if (name == "one"_qs)
			return LayoutMode::OnePage;
		if (name == "two"_qs)
			return LayoutMode::TwoPages;
		if (name == "twoshifted"_qs)
			return LayoutMode::TwoPagesShifted;

		qWarning () << "unknown layout mode" << name;
		return LayoutMode::OnePage;
	}

	void AddToolbarEntries (QToolBar& toolbar, const QVector<ToolbarEntry>& entries)
	{
		for (const auto& entry : entries)
			Util::Visit (entry,
					[&] (QAction *action) { toolbar.addAction (action); },
					[&] (QWidget *widget) { toolbar.addWidget (widget); });
	}
}
