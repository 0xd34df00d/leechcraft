/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihavesettings.h>

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Eleeminator
{
	class ColorSchemesManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveShortcuts
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveShortcuts IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Eleeminator")

		TabClassInfo TermTabTC_;

		Util::ShortcutManager *ShortcutMgr_;

		Util::XmlSettingsDialog_ptr XSD_;

		ColorSchemesManager *ColorSchemesMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		QMap<QByteArray, ActionInfo> GetActionInfo () const override;
		void SetShortcut (const QByteArray&, const QKeySequences_t&) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;
	};
}
