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
	class TitleUpdater : public QObject
	{
		QTermWidget& Term_;
		TermTab& Tab_;

		QTimer Timer_;
	public:
		TitleUpdater (QTermWidget& term, TermTab& tab);
	private:
		void UpdateTitle ();
	};

	TitleUpdater::TitleUpdater (QTermWidget& term, TermTab& tab)
	: QObject { &tab }
	, Term_ { term }
	, Tab_ { tab }
	{
		Timer_.setTimerType (Qt::VeryCoarseTimer);
		Timer_.callOnTimeout (this, &TitleUpdater::UpdateTitle);
		Timer_.start (3000);

		QTimer::singleShot (0, this, &TitleUpdater::UpdateTitle);
	}

	void TitleUpdater::UpdateTitle ()
	{
		auto cwd = Term_.workingDirectory ();
		while (cwd.endsWith ('/'))
			cwd.chop (1);

		const auto& tree = ProcessGraphBuilder { Term_.getShellPID () }.GetProcessTree ();
		const auto& processName = tree.Children_.isEmpty () ?
				tree.Command_ :
				tree.Children_.value (0).Command_;

		const auto& title = cwd.isEmpty () ?
				processName :
				(cwd.section ('/', -1, -1) + " : " + processName);
		emit Tab_.changeTabName (title);
	}

	void SetupTitleUpdater (QTermWidget& term, TermTab& tab)
	{
		new TitleUpdater { term, tab };
	}
}
