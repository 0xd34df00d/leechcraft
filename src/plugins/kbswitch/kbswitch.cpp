/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kbswitch.h"
#include <QIcon>
#include <QStringListModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "keyboardlayoutswitcher.h"
#include "xmlsettingsmanager.h"
#include "kbctl.h"
#include "quarkproxy.h"
#include "flagiconprovider.h"
#include "layoutsconfigwidget.h"
#include "rulesstorage.h"
#include "optionsconfigwidget.h"

namespace LC
{
namespace KBSwitch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"kbswitchsettings.xml");

		KBCtl::Instance ();

		SettingsDialog_->SetCustomWidget ("LayoutsConfigWidget", new LayoutsConfigWidget);
		SettingsDialog_->SetCustomWidget ("OptionsConfigWidget", new OptionsConfigWidget);

		auto rulesStorage = KBCtl::Instance ().GetRulesStorage ();
		SettingsDialog_->SetDataSource ("KeyboardModel",
				new QStringListModel (rulesStorage->GetKBModelsStrings ()));

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

		Indicator_ = std::make_shared<QuarkComponent> ("kbswitch", "IndicatorQuark.qml");
		Indicator_->DynamicProps_.append ({ "KBSwitch_proxy", new QuarkProxy });
		Indicator_->ImageProviders_.append ({ "KBSwitch_flags", new FlagIconProvider });

		KBCtl::Instance ().scheduleApply ();
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
		return tr ("Provides keyboard layout configurator and plugin- or tab-grained layout control.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { Indicator_ };
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

LC_EXPORT_PLUGIN (leechcraft_kbswitch, LC::KBSwitch::Plugin);
