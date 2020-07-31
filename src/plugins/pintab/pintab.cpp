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
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/irootwindowsmanager.h>

namespace LC
{
namespace PinTab
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("pintab");

		Proxy_ = proxy;
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (tabAdded (int, int)),
				this,
				SLOT (checkPinState (int, int)));
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (tabIsRemoving (int, int)),
				this,
				SLOT (handleTabRemoving (int, int)));
		connect (proxy->GetRootWindowsManager ()->GetQObject (),
				SIGNAL (windowRemoved (int)),
				this,
				SLOT (handleWindowRemoved (int)));

		PinTab_ = new QAction (tr ("Pin tab"), this);
		connect (PinTab_,
				&QAction::triggered,
				this,
				[this] { PinTab (-1, Proxy_->GetRootWindowsManager ()->GetPreferredWindowIndex ()); });

		UnPinTab_ = new QAction (tr ("Unpin tab"), this);
		connect (UnPinTab_,
				&QAction::triggered,
				this,
				[this] { UnPinTab (-1, Proxy_->GetRootWindowsManager ()->GetPreferredWindowIndex ()); });

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
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::PinTab (int tabIndex, int windowIndex)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowIndex);
		if (!window)
			return;
		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowIndex);
		if (!tw)
			return;

		if (tabIndex == -1)
			tabIndex = sender ()->property ("Leechcraft/PinTab/CurrentIndex").toInt ();

		if (tabIndex < 0 ||
				tabIndex >= tw->WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< tabIndex;
			return;
		}

		const auto widget = tw->Widget (tabIndex);
		widget->setProperty ("SessionData/org.LeechCraft.PinTab.PinState", true);

		auto pair = qMakePair (tw->TabText (tabIndex), tw->TabButton (tabIndex, CloseSide_));
		tw->SetTabText (tabIndex, "");
		tw->SetTabClosable (tabIndex, false);

		Window2Widget2TabData_ [window] [widget] = pair;

		tw->MoveTab (tabIndex, Window2Widget2TabData_ [window].count () - 1);
	}

	void Plugin::UnPinTab (int tabIndex, int windowIndex)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowIndex);
		if (!window)
			return;

		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowIndex);
		if (!tw)
			return;

		if (tabIndex == -1)
			tabIndex = sender ()->property ("Leechcraft/PinTab/CurrentIndex").toInt ();

		if (tabIndex < 0 ||
				tabIndex >= tw->WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< tabIndex;
			return;
		}

		const auto widget = tw->Widget (tabIndex);

		tw->Widget (tabIndex)->setProperty ("SessionData/org.LeechCraft.PinTab.PinState", false);
		auto data = Window2Widget2TabData_ [window].take (widget);

		tw->SetTabText (tabIndex, data.first);
		tw->SetTabClosable (tabIndex, true, data.second);

		tw->MoveTab (tabIndex, Window2Widget2TabData_.value (window).count ());
	}

	void Plugin::hookTabContextMenuFill (LC::IHookProxy_ptr,
			QMenu *menu, int index, int windowId)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;
		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw)
			return;

		const auto firstAction = tw->GetPermanentActions ().value (0);

		const auto widget = tw->Widget (index);
		if (Window2Widget2TabData_.value (window).contains (widget))
		{
			menu->insertAction (firstAction, UnPinTab_);
			UnPinTab_->setProperty ("Leechcraft/PinTab/CurrentIndex", index);
		}
		else
		{
			menu->insertAction (firstAction, PinTab_);
			PinTab_->setProperty ("Leechcraft/PinTab/CurrentIndex", index);
		}
	}

	void Plugin::hookTabFinishedMoving (LC::IHookProxy_ptr, int index, int windowId)
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
			PinTab (index, windowId);
			tw->MoveTab (Window2Widget2TabData_ [window].count () - 1, index);
		}
		else if (Window2Widget2TabData_ [window].contains (widget) &&
				index &&
				!Window2Widget2TabData_ [window].contains (prevWidget))
		{
			UnPinTab (index, windowId);
			tw->MoveTab (Window2Widget2TabData_ [window].count (), index);
		}
	}

	void Plugin::hookTabSetText (IHookProxy_ptr proxy, int index, int windowId)
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

	void Plugin::checkPinState (int windowId, int index)
	{
		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw ||
				tw->WidgetCount () <= index)
			return;

		bool isPinned = tw->Widget (index)->
				property ("SessionData/org.LeechCraft.PinTab.PinState").toBool ();

		if (isPinned)
			PinTab (index, windowId);
	}

	void Plugin::handleTabRemoving (int windowId, int index)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (windowId);
		if (!window)
			return;

		auto tw = Proxy_->GetRootWindowsManager ()->GetTabWidget (windowId);
		if (!tw)
			return;

		Window2Widget2TabData_ [window].remove (tw->Widget (index));
	}

	void Plugin::handleWindowRemoved (int index)
	{
		auto window = Proxy_->GetRootWindowsManager ()->GetMainWindow (index);
		if (!window)
			return;

		Window2Widget2TabData_.remove (window);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pintab, LC::PinTab::Plugin);
