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

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Eleeminator
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

		ICoreProxy_ptr Proxy_;
		TabClassInfo TermTabTC_;

		Util::ShortcutManager *ShortcutMgr_;

		Util::XmlSettingsDialog_ptr XSD_;

		ColorSchemesManager *ColorSchemesMgr_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QMap<QString, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QString&, const QKeySequences_t&);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	};
}
}
