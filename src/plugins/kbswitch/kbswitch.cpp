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
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "keyboardlayoutswitcher.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace KBSwitch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"kbswitchsettings.xml");

		KBLayoutSwitcher_ = new KeyboardLayoutSwitcher (this);

		MainTabWidget_ = proxy->GetTabWidget ();
		connect (MainTabWidget_->GetObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentChanged (int)));
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
		return QIcon (":/kbswitch/resources/images/kbswitch.svg");
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::handleCurrentChanged (int index)
	{
		if (KBLayoutSwitcher_->IsGlobalPolicy ())
			return;

		QWidget *currentWidget = MainTabWidget_->Widget (index);
		QWidget *prevWidget = MainTabWidget_->GetPreviousWidget ();
		KBLayoutSwitcher_->updateKBLayouts (currentWidget, prevWidget);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_kbswitch, LeechCraft::KBSwitch::Plugin);
