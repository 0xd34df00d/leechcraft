/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pintab.h"
#include <QIcon>
#include <QMenu>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/irootwindowsmanager.h>

namespace LC::PinTab
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (tabAdded (int, QWidget*)),
				this,
				SLOT (checkPinState (int, QWidget*)));
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (tabIsRemoving (int, QWidget*)),
				this,
				SLOT (handleTabRemoving (int, QWidget*)));
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (windowRemoved (int)),
				this,
				SLOT (handleWindowRemoved (int)));

		CloseSide_ = proxy->GetRootWindowsManager ()->GetTabWidget (0)->GetCloseButtonPosition ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.PinTab";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "PinTab";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support tabs pinning.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Core.Plugins/1.0" };
	}

	void Plugin::PinTab (QWidget *tab, int windowIndex)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowIndex);
		if (!window)
			return;
		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowIndex);
		if (!tw)
			return;

		tab->setProperty ("SessionData/org.LeechCraft.PinTab.PinState", true);

		const auto tabIndex = tw->IndexOf (tab);

		auto pair = qMakePair (tw->TabText (tabIndex), tw->TabButton (tabIndex, CloseSide_));
		tw->SetTabText (tabIndex, {});
		tw->SetTabClosable (tabIndex, false);

		Window2Widget2TabData_ [window] [tab] = pair;

		tw->MoveTab (tabIndex, Window2Widget2TabData_ [window].count () - 1);
	}

	void Plugin::UnPinTab (QWidget *tab, int windowIndex)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowIndex);
		if (!window)
			return;

		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowIndex);
		if (!tw)
			return;

		const auto tabIndex = tw->IndexOf (tab);

		tab->setProperty ("SessionData/org.LeechCraft.PinTab.PinState", false);
		auto data = Window2Widget2TabData_ [window].take (tab);

		tw->SetTabText (tabIndex, data.first);
		tw->SetTabClosable (tabIndex, true, data.second);

		tw->MoveTab (tabIndex, Window2Widget2TabData_.value (window).count ());
	}

	void Plugin::hookTabContextMenuFill (const IHookProxy_ptr&,
			QMenu *menu, int index, int windowId)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;
		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw)
			return;

		const auto widget = tw->Widget (index);

		const auto action = new QAction { menu };
		if (Window2Widget2TabData_.value (window).contains (widget))
		{
			action->setText (tr ("Unpin tab"));
			action->setIcon (QIcon::fromTheme ("object-unlocked"));
			connect (action,
					&QAction::triggered,
					this,
					[=, this] { UnPinTab (widget, windowId); });
		}
		else
		{
			action->setText (tr ("Pin tab"));
			action->setIcon (QIcon::fromTheme ("object-locked"));
			connect (action,
					&QAction::triggered,
					this,
					[=, this] { PinTab (widget, windowId); });
		}

		menu->insertAction (tw->GetPermanentActions ().value (0), action);
	}

	void Plugin::hookTabFinishedMoving (const IHookProxy_ptr&, int index, int windowId)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;

		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw)
			return;

		const auto widget = tw->Widget (index);
		const auto nextWidget = tw->Widget (index + 1);
		const auto prevWidget = tw->Widget (index - 1);

		if (Window2Widget2TabData_.value (window).contains (nextWidget) &&
				!Window2Widget2TabData_.value (window).contains (widget))
		{
			PinTab (widget, windowId);
			tw->MoveTab (Window2Widget2TabData_ [window].count () - 1, index);
		}
		else if (Window2Widget2TabData_ [window].contains (widget) &&
				index &&
				!Window2Widget2TabData_ [window].contains (prevWidget))
		{
			UnPinTab (widget, windowId);
			tw->MoveTab (Window2Widget2TabData_ [window].count (), index);
		}
	}

	void Plugin::hookTabSetText (const IHookProxy_ptr& proxy, int index, int windowId)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;

		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw)
			return;

		const auto widget = tw->Widget (index);
		if (Window2Widget2TabData_.value (window).contains (widget))
			proxy->CancelDefault ();
	}

	void Plugin::checkPinState (int windowId, QWidget *tab)
	{
		if (tab->property ("SessionData/org.LeechCraft.PinTab.PinState").toBool ())
			PinTab (tab, windowId);
	}

	void Plugin::handleTabRemoving (int windowId, QWidget *tab)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;

		Window2Widget2TabData_ [window].remove (tab);
	}

	void Plugin::handleWindowRemoved (int index)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (index);
		if (!window)
			return;

		Window2Widget2TabData_.remove (window);
	}
}

LC_EXPORT_PLUGIN (leechcraft_pintab, LC::PinTab::Plugin);
