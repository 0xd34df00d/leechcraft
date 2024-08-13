/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "navigator.h"
#include <QDir>
#include <QFileInfo>
#include <QtDebug>
#include <util/threads/coro.h>
#include <util/threads/coro/context.h>
#include "components/layout/pageslayoutmanager.h"
#include "components/services/documentloader.h"
#include "navigationhistory.h"
#include "filewatcher.h"

namespace LC::Monocle
{
	Navigator::NavCtx::NavCtx (Navigator& nav)
	: Nav_ { nav }
	{
	}

	void Navigator::NavCtx::Navigate (const NavigationAction& act)
	{
		Nav_.Navigate (act);
	}

	void Navigator::NavCtx::Navigate (const ExternalNavigationAction& act)
	{
		Nav_.StartLoading (act.TargetDocument_, {}, act.DocumentNavigation_);
	}

	Navigator::Navigator (const PagesLayoutManager& layoutMgr, DocumentLoader& loader, QObject *parent)
	: QObject { parent }
	, Loader_ { loader }
	, Layout_ { layoutMgr }
	, History_ { *new NavigationHistory { [this] { return GetCurrentPosition (); }, this } }
	, Watcher_ { *new FileWatcher { this } }
	{
		connect (&History_,
				&NavigationHistory::navigationRequested,
				this,
				[this] (const ExternalNavigationAction& nav)
				{
					if (nav.TargetDocument_ == CurrentPath_)
						emit positionRequested (nav.DocumentNavigation_);
					else
						StartLoading (nav.TargetDocument_, DocumentOpenOption::NoHistoryRecord, nav.DocumentNavigation_);
				});

		connect (&Watcher_,
				&FileWatcher::reloadNeeded,
				this,
				[this] (const QString& doc)
				{
					if (doc != CurrentPath_)
						qWarning () << "path mismatch" << doc << CurrentPath_;
					StartLoading (doc,
							DocumentOpenOption::IgnoreErrors | DocumentOpenOption::NoHistoryRecord,
							GetCurrentPosition ().DocumentNavigation_);
				});
	}

	LinkExecutionContext& Navigator::GetNavigationContext ()
	{
		return NavCtx_;
	}

	const NavigationHistory& Navigator::GetNavigationHistory () const
	{
		return History_;
	}

	void Navigator::Navigate (const NavigationAction& nav)
	{
		History_.SaveCurrentPos ();
		emit positionRequested (nav);
	}

	void Navigator::OpenDocument (const QString& path)
	{
		StartLoading (path, {}, {});
	}

	void Navigator::StartLoading (QString path,
			DocumentOpenOptions options,
			const std::optional<NavigationAction>& targetPos)
	{
		if (!CurrentPath_.isEmpty () && QFileInfo { path }.isRelative ())
			path = QFileInfo { CurrentPath_ }.dir ().absoluteFilePath (path);

		[] (auto pThis, auto path, auto options, auto targetPos) -> Util::ContextTask<>
		{
			co_await Util::AddContextObject (*pThis);
			const auto& document = co_await pThis->Loader_.LoadDocument (path);
			if (!document || !document->IsValid ())
			{
				qWarning () << "unable to navigate to" << path;
				if (!(options & DocumentOpenOption::IgnoreErrors))
					emit pThis->loadingFailed (path);
				co_return;
			}

			pThis->Watcher_.SetWatchedFile (path);
			pThis->CurrentPath_ = path;

			emit pThis->loaded (document, path);
			if (targetPos)
				emit pThis->positionRequested (*targetPos);
		} (this, path, options, targetPos);
	}

	ExternalNavigationAction Navigator::GetCurrentPosition () const
	{
		// TODO properly handle lack of current page
		const auto pos = Layout_.GetCurrentPagePos ();
		if (!pos)
			return {};

		return
		{
			CurrentPath_,
			{ pos->Page_, PageRelativeRectBase { pos->Pos_, pos->Pos_ } }
		};
	}
}
