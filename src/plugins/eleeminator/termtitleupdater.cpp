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

		struct ChildProcessInfo
		{
			int Pid_;
			QString Command_;
		};

		std::optional<ChildProcessInfo> CachedChild_;
	public:
		TitleUpdater (QTermWidget& term, TermTab& tab);
	private:
		void UpdateTitle ();
		void RefreshCachedChild ();
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

		RefreshCachedChild ();

		const auto& cmd = CachedChild_ ?
				CachedChild_->Command_ :
				Tab_.GetTabClassInfo ().VisibleName_;
		const auto& title = cwd.isEmpty () ?
				cmd :
				(cwd.section ('/', -1, -1) + " : " + cmd);

		emit Tab_.changeTabName (title);
	}

	void TitleUpdater::RefreshCachedChild ()
	{
		if (CachedChild_)
		{
			const auto& cachedChildParent = GetParentPid (QString::number (CachedChild_->Pid_));
			if (cachedChildParent && *cachedChildParent == Term_.getShellPID ())
				return;
		}

		const auto& tree = ProcessGraphBuilder { Term_.getShellPID () }.GetProcessTree ();
		if (tree.Children_.isEmpty ())
		{
			CachedChild_.reset ();
			return;
		}

		const auto& child = tree.Children_.value (0);
		CachedChild_ = ChildProcessInfo { child.Pid_, child.Command_ };
	}

	void SetupTitleUpdater (QTermWidget& term, TermTab& tab)
	{
		new TitleUpdater { term, tab };
	}
}
