/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eleeminator.h"
#include <QIcon>
#include <QtDebug>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "termtab.h"
#include "xmlsettingsmanager.h"
#include "colorschemesmanager.h"

namespace LC
{
namespace Eleeminator
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		ShortcutMgr_ = new Util::ShortcutManager { proxy };
		ShortcutMgr_->SetObject (this);

		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Close",
				{
					tr ("Close terminal tab"),
					QString { "Ctrl+Shift+W" },
					proxy->GetIconThemeManager ()->GetIcon ("tab-close")
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Clear",
				{
					tr ("Clear terminal window"),
					QString { "Ctrl+Shift+L" },
					proxy->GetIconThemeManager ()->GetIcon ("edit-clear")
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Copy",
				{
					tr ("Copy selected text to clipboard"),
					QString { "Ctrl+Shift+C" },
					proxy->GetIconThemeManager ()->GetIcon ("edit-copy")
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Paste",
				{
					tr ("Paste text from clipboard"),
					QString { "Ctrl+Shift+V" },
					proxy->GetIconThemeManager ()->GetIcon ("edit-paste")
				});

		ColorSchemesMgr_ = new ColorSchemesManager;

		Util::InstallTranslator ("eleeminator");

		TermTabTC_ =
		{
			GetUniqueID () + ".TermTab",
			tr ("Terminal"),
			tr ("Termianl emulator."),
			GetIcon (),
			15,
			TFOpenableByRequest | TFOverridesTabClose
		};

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "eleeminatorsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Eleeminator";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Eleeminator";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Embedded LeechCraft terminal emulator.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TermTabTC_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == TermTabTC_.TabClass_)
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (TermTabTC_.VisibleName_,
					new TermTab { Proxy_, ShortcutMgr_, TermTabTC_, ColorSchemesMgr_, this });
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		ShortcutMgr_->SetShortcut (id, sequences);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_eleeminator, LC::Eleeminator::Plugin);
