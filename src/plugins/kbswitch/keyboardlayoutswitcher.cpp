/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "keyboardlayoutswitcher.h"
#include <QtDebug>
#include <QWidget>
#include "xmlsettingsmanager.h"
#include <interfaces/ihavetabs.h>
#include "qmetatype.h"

#ifdef Q_OS_LINUX
	#include <X11/Xlib.h>
	#include <X11/XKBlib.h>
#endif

namespace LeechCraft
{
namespace KBSwitch
{
	KeyboardLayoutSwitcher::KeyboardLayoutSwitcher (QObject *parent)
	: QObject (parent)
	, LastCurrentWidget_ (0)
	{
		XmlSettingsManager::Instance ().RegisterObject ("SwitchingPolicy",
				this, "setSwitchingPolicy");
		setSwitchingPolicy ();
	}

	bool KeyboardLayoutSwitcher::IsGlobalPolicy () const
	{
		return CurrentSwitchingPloicy_ == SwitchingPolicy::Global;
	}

	void KeyboardLayoutSwitcher::updateKBLayouts (QWidget *current, QWidget *prev)
	{
		if (CurrentSwitchingPloicy_ == SwitchingPolicy::Global)
			return;

		if (LastCurrentWidget_ == current)
			return;

#ifdef Q_OS_LINUX
		int xkbEventType, xkbError, xkbReason;
		int mjr = XkbMajorVersion, mnr = XkbMinorVersion;
		Display *display = NULL;
		display = XkbOpenDisplay (NULL,
				&xkbEventType,
				&xkbError,
				&mjr,
				&mnr,
				&xkbReason);

		if (CurrentSwitchingPloicy_ == SwitchingPolicy::Tab)
		{
			if (prev)
			{
				ITabWidget *itw = qobject_cast<ITabWidget*> (prev);
				if (!itw)
				{
					qWarning () << Q_FUNC_INFO
							<< current
							<< "is not an ITabWidget class";
				}
				else
				{
					connect (itw->ParentMultiTabs (),
							SIGNAL (removeTab (QWidget*)),
							this,
							SLOT (removeWidget (QWidget*)),
							Qt::UniqueConnection);

					XkbStateRec xkbState;
					XkbGetState (display, XkbUseCoreKbd, &xkbState);
					Widget2KBLayoutIndex_ [prev] = xkbState.group;

				}
			}
			if (current)
			{
				LastCurrentWidget_ = current;

				if (Widget2KBLayoutIndex_.contains (current))
				{
					int xkbGroup = Widget2KBLayoutIndex_ [current];
					if (!XkbLockGroup (display, XkbUseCoreKbd, xkbGroup))
						qWarning () << Q_FUNC_INFO
								<< "Request to change layout not send";
				}
			}
		}
		else if (CurrentSwitchingPloicy_ == SwitchingPolicy::Plugin)
		{
			if (prev)
			{
				ITabWidget *itw = qobject_cast<ITabWidget*> (prev);
				if (!itw)
				{
					qWarning () << Q_FUNC_INFO
							<< current
							<< "is not an ITabWidget class";
				}
				else
				{
					connect (itw->ParentMultiTabs (),
							SIGNAL (removeTab (QWidget*)),
							this,
							SLOT (handleRemoveWidget (QWidget*)),
							Qt::UniqueConnection);

					XkbStateRec xkbState;
					XkbGetState (display, XkbUseCoreKbd, &xkbState);
					TabClass2KBLayoutIndex_ [itw->GetTabClassInfo ().TabClass_] = xkbState.group;
				}
			}

			if (current)
			{
				ITabWidget *itw = qobject_cast<ITabWidget*> (current);
				if (!itw)
				{
					qWarning () << Q_FUNC_INFO
							<< current
							<< "is not an ITabWidget class";
				}
				else
				{
					LastCurrentWidget_ = current;
					if (TabClass2KBLayoutIndex_.contains (itw->GetTabClassInfo ().TabClass_))
					{
						int xkbGroup = TabClass2KBLayoutIndex_ [itw->GetTabClassInfo ().TabClass_];
						if (!XkbLockGroup (display, XkbUseCoreKbd, xkbGroup))
							qWarning () << Q_FUNC_INFO
									<< "Request to change layout not send";
					}
				}
			}
		}

		XCloseDisplay (display);
#endif
	}

	void KeyboardLayoutSwitcher::setSwitchingPolicy ()
	{
		if (XmlSettingsManager::Instance ()
				.property ("SwitchingPolicy").toString () == "global")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Global;
		else if (XmlSettingsManager::Instance ()
				.property ("SwitchingPolicy").toString () == "plugin")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Plugin;
		else if (XmlSettingsManager::Instance ()
				.property ("SwitchingPolicy").toString () == "tab")
			CurrentSwitchingPloicy_ = SwitchingPolicy::Tab;
	}

	void KeyboardLayoutSwitcher::handleRemoveWidget (QWidget *widget)
	{
		Widget2KBLayoutIndex_.remove (widget);
	}

}
}