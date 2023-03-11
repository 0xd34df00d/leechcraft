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
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include "termtab.h"
#include "xmlsettingsmanager.h"
#include "colorschemesmanager.h"

namespace LC::Eleeminator
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		const auto& proxy = GetProxyHolder ();

		ShortcutMgr_ = new Util::ShortcutManager { proxy };
		ShortcutMgr_->SetObject (this);

		auto& itm = *proxy->GetIconThemeManager ();

		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Close",
				{
					tr ("Close terminal tab"),
					"Ctrl+Shift+W"_qs,
					itm.GetIcon ("tab-close"_qs)
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Clear",
				{
					tr ("Clear terminal window"),
					"Ctrl+Shift+L"_qs,
					itm.GetIcon ("edit-clear"_qs)
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Copy",
				{
					tr ("Copy selected text to clipboard"),
					"Ctrl+Shift+C"_qs,
					itm.GetIcon ("edit-copy"_qs)
				});
		ShortcutMgr_->RegisterActionInfo (GetUniqueID () + ".Paste",
				{
					tr ("Paste text from clipboard"),
					"Ctrl+Shift+V"_qs,
					itm.GetIcon ("edit-paste"_qs)
				});

		ColorSchemesMgr_ = new ColorSchemesManager;

		Util::InstallTranslator ("eleeminator"_qs);

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
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "eleeminatorsettings.xml"_qs);
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
		return "Eleeminator"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Embedded LeechCraft terminal emulator.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TermTabTC_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == TermTabTC_.TabClass_)
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (TermTabTC_.VisibleName_,
					new TermTab { ShortcutMgr_, TermTabTC_, ColorSchemesMgr_, this });
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	QMap<QByteArray, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QByteArray& id, const QKeySequences_t& sequences)
	{
		ShortcutMgr_->SetShortcut (id, sequences);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}

LC_EXPORT_PLUGIN (leechcraft_eleeminator, LC::Eleeminator::Plugin);
