/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mwproxy.h"
#include <QDockWidget>
#include <QToolBar>
#include "core.h"
#include "mainwindow.h"
#include "dockmanager.h"

namespace LC
{
	MWProxy::MWProxy (MainWindow *win, QObject *parent)
	: QObject (parent)
	, Win_ (win)
	{
	}

	void MWProxy::AddDockWidget (QDockWidget *w, const DockWidgetParams& params)
	{
		Core::Instance ().GetDockManager ()->AddDockWidget (w, params);
		ToggleViewActionVisiblity (w, true);
	}

	void MWProxy::AssociateDockWidget (QDockWidget *dock, QWidget *tab)
	{
		Core::Instance ().GetDockManager ()->AssociateDockWidget (dock, tab);
	}

	void MWProxy::SetDockWidgetVisibility (QDockWidget *dock, bool visible)
	{
		Core::Instance ().GetDockManager ()->SetDockWidgetVisibility (dock, visible);
	}

	void MWProxy::ToggleViewActionVisiblity (QDockWidget *w, bool visible)
	{
		Core::Instance ().GetDockManager ()->ToggleViewActionVisiblity (w, visible);
	}

	void MWProxy::SetViewActionShortcut (QDockWidget *w, const QKeySequence& seq)
	{
		w->toggleViewAction ()->setShortcut (seq);
	}

	void MWProxy::ToggleVisibility ()
	{
		Win_->showHideMain ();
	}

	void MWProxy::ShowMain ()
	{
		Win_->showMain ();
	}

	QMenu* MWProxy::GetMainMenu ()
	{
		return Win_->GetMainMenu ();
	}

	void MWProxy::HideMainMenu ()
	{
		Win_->HideMainMenu ();
	}
}
