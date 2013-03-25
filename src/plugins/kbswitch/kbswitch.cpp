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

#include "kbswitch.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "keyboardlayoutswitcher.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace KBSwitch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"kbswitchsettings.xml");

		KBLayoutSwitcher_ = new KeyboardLayoutSwitcher (this);

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));
		connect (rootWM->GetQObject (),
				SIGNAL (currentWindowChanged (int, int)),
				this,
				SLOT(handleCurrentWindowChanged (int, int)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.KBSwitch";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "KBSwitch";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides plugin- or tab-grained keyboard layout control.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/kbswitch/resources/images/kbswitch.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::handleCurrentChanged (int index)
	{
		if (KBLayoutSwitcher_->IsGlobalPolicy ())
			return;

		auto ictw = qobject_cast<ICoreTabWidget*> (sender ());
		QWidget *currentWidget = ictw->Widget (index);
		QWidget *prevWidget = ictw->GetPreviousWidget ();
		KBLayoutSwitcher_->updateKBLayouts (currentWidget, prevWidget);
	}

	void Plugin::handleCurrentWindowChanged (int from, int to)
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();

		auto currentTW = rootWM->GetTabWidget (to);
		auto prevTW = rootWM->GetTabWidget (from);
		auto currentWidget = currentTW->Widget (currentTW->CurrentIndex ());
		auto prevWidget = prevTW->Widget (prevTW->CurrentIndex ());
		KBLayoutSwitcher_->updateKBLayouts (currentWidget, prevWidget);
	}

	void Plugin::handleWindow (int index)
	{
		auto tabWidget = Proxy_->GetRootWindowsManager ()->GetTabWidget (index);
		connect (tabWidget->GetQObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentChanged (int)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_kbswitch, LeechCraft::KBSwitch::Plugin);
