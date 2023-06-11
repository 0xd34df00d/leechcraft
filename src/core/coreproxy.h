/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/core/icoreproxy.h"
#include "loaders/ipluginloader.h"

namespace LC
{
	class EntityManager;
	class IconThemeManagerProxy;

	/** Implements the ICoreProxy's interface.
	 */
	class CoreProxy : public QObject
					, public ICoreProxy
	{
		EntityManager * const EM_;

		const std::shared_ptr<IconThemeManagerProxy> IconThemeMgr_;
	public:
		explicit CoreProxy (Loaders::IPluginLoader_ptr);

		QNetworkAccessManager* GetNetworkAccessManager () const override;
		IShortcutProxy* GetShortcutProxy () const override;
		QModelIndex MapToSource (const QModelIndex&) const override;

		Util::BaseSettingsManager* GetSettingsManager () const override;

		IRootWindowsManager* GetRootWindowsManager () const override;

		IIconThemeManager* GetIconThemeManager () const override;

		IColorThemeManager* GetColorThemeManager () const override;

		ITagsManager* GetTagsManager () const override;
		QStringList GetSearchCategories () const override;
		IPluginsManager* GetPluginsManager () const override;
		IEntityManager* GetEntityManager () const override;
		QString GetVersion () const override;
		void RegisterSkinnable (QAction*) override;
		bool IsShuttingDown () override;

		/* This is unsafe since this returns a core proxy without certain dependencies,
		 * like the IPluginLoader_ptr that's normally passed to the ctor.
		 *
		 * Beware of the effects.
		 */
		static ICoreProxy_ptr UnsafeWithoutDeps ();
	};
}
