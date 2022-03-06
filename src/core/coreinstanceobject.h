/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef COREINSTANCEOBJECT_H
#define COREINSTANCEOBJECT_H
#include <QObject>
#include "interfaces/iinfo.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/ihavetabs.h"
#include "interfaces/ipluginready.h"
#include "interfaces/ihaveshortcuts.h"

class IShortcutProxy;

namespace LC
{
	class SettingsTab;
	class CorePlugin2Manager;
	class ShortcutManager;

	namespace Util
	{
		class ShortcutManager;
	}

	class CoreInstanceObject : public QObject
							 , public IInfo
							 , public IHaveSettings
							 , public IHaveTabs
							 , public IHaveShortcuts
							 , public IPluginReady
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IHaveTabs IHaveShortcuts IPluginReady)

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		TabClasses_t Classes_;

		SettingsTab *SettingsTab_ = nullptr;

		CorePlugin2Manager *CorePlugin2Manager_;

		ShortcutManager *ShortcutManager_;

		Util::ShortcutManager *CoreShortcutManager_;
	public:
		CoreInstanceObject (QObject* = 0);

		// IInfo
		void SetProxy (ICoreProxy_ptr);
		void SetPluginInstance (QObject*);

		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		// IHaveSettings
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		// IHaveTabs
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		// IHaveShortcuts
		QMap<QString, ActionInfo> GetActionInfo () const;
		void SetShortcut (const QString& id, const QKeySequences_t& sequences);

		// IPluginReady
		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		CorePlugin2Manager* GetCorePluginManager () const;

		SettingsTab* GetSettingsTab ();

		IShortcutProxy* GetShortcutProxy () const;
		ShortcutManager* GetShortcutManager () const;
		Util::ShortcutManager* GetCoreShortcutManager () const;
	private:
		void BuildNewTabModel ();
		void LazyInitSettingsTab ();
	private slots:
		void handleSettingsButton (const QString&);
		void updateIconSet ();
		void updateColorTheme ();
#ifdef STRICT_LICENSING
		void notifyLicensing ();
#endif
	signals:
		void gotEntity (const LC::Entity&);
	};
}

#endif
