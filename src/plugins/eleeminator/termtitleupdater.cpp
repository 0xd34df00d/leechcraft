/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "termtitleupdater.h"
#include <QTimer>
#include <qtermwidget.h>
#include "processgraphbuilder.h"
#include "termtab.h"

namespace LC::Eleeminator
{
	namespace
	{
		void UpdateTitle (QTermWidget& term, TermTab& tab)
		{
			auto cwd = term.workingDirectory ();
			while (cwd.endsWith ('/'))
				cwd.chop (1);

			const auto& tree = ProcessGraphBuilder { term.getShellPID () }.GetProcessTree ();
			const auto& processName = tree.Children_.isEmpty () ?
					tree.Command_ :
					tree.Children_.value (0).Command_;

			const auto& title = cwd.isEmpty () ?
					processName :
					(cwd.section ('/', -1, -1) + " : " + processName);
			emit tab.changeTabName (title);
		}
	}

	void SetupTitleUpdater (QTermWidget& term, TermTab& tab)
	{
		auto timer = new QTimer { &tab };
		timer->callOnTimeout ([&] { UpdateTitle (term, tab); });
		timer->start (3000);
	}
}
