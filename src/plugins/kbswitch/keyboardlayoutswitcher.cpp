/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "keyboardlayoutswitcher.h"
#include <QtDebug>
#include <QWidget>
#include <util/sll/unreachable.h>
#include <util/x11/xwrapper.h>
#include <interfaces/ihavetabs.h>
#include "xmlsettingsmanager.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

namespace LC
{
namespace KBSwitch
{
	KeyboardLayoutSwitcher::KeyboardLayoutSwitcher (QObject *parent)
	: QObject { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ("SwitchingPolicy",
				this, "setSwitchingPolicy");
		setSwitchingPolicy ();
	}

	bool KeyboardLayoutSwitcher::IsGlobalPolicy () const
	{
		return CurrentSwitchingPloicy_ == SwitchingPolicy::Global;
	}

	void KeyboardLayoutSwitcher::UpdateSavedState (ITabWidget *itw, int group)
	{
		switch (CurrentSwitchingPloicy_)
		{
		case SwitchingPolicy::Global:
			break;
		case SwitchingPolicy::Tab:
			Widget2KBLayoutIndex_ [itw] = group;
			break;
		case SwitchingPolicy::Plugin:
			TabClass2KBLayoutIndex_ [itw->GetTabClassInfo ().TabClass_] = group;
			break;
		}
	}

	std::optional<int> KeyboardLayoutSwitcher::GetSavedState (ITabWidget *itw) const
	{
		using Res_t = std::optional<int>;

		switch (CurrentSwitchingPloicy_)
		{
		case SwitchingPolicy::Global:
			return {};
		case SwitchingPolicy::Tab:
			return Widget2KBLayoutIndex_.contains (itw) ?
					Res_t { Widget2KBLayoutIndex_ [itw] } :
					Res_t {};
		case SwitchingPolicy::Plugin:
		{
			const auto& tc = itw->GetTabClassInfo ().TabClass_;
			return TabClass2KBLayoutIndex_.contains (tc) ?
					Res_t { TabClass2KBLayoutIndex_ [tc] } :
					Res_t {};
		}
		}

		Util::Unreachable ();
	}

	void KeyboardLayoutSwitcher::updateKBLayouts (QWidget *current, QWidget *prev)
	{
		if (CurrentSwitchingPloicy_ == SwitchingPolicy::Global)
			return;

		if (LastCurrentWidget_ == current)
			return;

		const auto display = Util::XWrapper::Instance ().GetDisplay ();

		if (const auto prevItw = qobject_cast<ITabWidget*> (prev))
		{
			connect (prevItw->ParentMultiTabs (),
					SIGNAL (removeTab (QWidget*)),
					this,
					SLOT (removeWidget (QWidget*)),
					Qt::UniqueConnection);

			XkbStateRec xkbState;
			XkbGetState (display, XkbUseCoreKbd, &xkbState);
			UpdateSavedState (prevItw, xkbState.group);
		}

		if (const auto currentItw = qobject_cast<ITabWidget*> (current))
		{
			LastCurrentWidget_ = current;

			const auto xkbGroup = GetSavedState (currentItw);
			if (xkbGroup && !XkbLockGroup (display, XkbUseCoreKbd, *xkbGroup))
				qWarning () << Q_FUNC_INFO
						<< "failed to change layout for new tab"
						<< current;
		}
	}

	void KeyboardLayoutSwitcher::setSwitchingPolicy ()
	{
		const auto& propStr = XmlSettingsManager::Instance ().property ("SwitchingPolicy").toString ();
		if (propStr == "global")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Global;
		else if (propStr == "plugin")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Plugin;
		else if (propStr == "tab")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Tab;
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown property value"
					<< propStr;
	}

	void KeyboardLayoutSwitcher::handleRemoveWidget (QWidget *widget)
	{
		Widget2KBLayoutIndex_.remove (qobject_cast<ITabWidget*> (widget));
	}
}
}
