/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreproxy.h"
#include <algorithm>
#include <QFileInfo>
#include <interfaces/ifinder.h>
#include "core.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "iconthemeengine.h"
#include "tagsmanager.h"
#include "entitymanager.h"
#include "config.h"
#include "colorthemeengine.h"
#include "rootwindowsmanager.h"

namespace LC
{
	class IconThemeManagerProxy : public QObject
								, public IIconThemeManager
	{
		const Loaders::IPluginLoader_ptr Loader_;
		IconThemeEngine& Engine_ = IconThemeEngine::Instance ();
	public:
		IconThemeManagerProxy (Loaders::IPluginLoader_ptr loader)
		: Loader_ { std::move (loader) }
		{
		}

		QIcon GetIcon (const QString& on, const QString& off) override
		{
			return Engine_.GetIcon (on, off);
		}

		void UpdateIconset (const QList<QAction*>& actions) override
		{
			Engine_.UpdateIconset (actions);
		}

		void ManageWidget (QWidget *w) override
		{
			Engine_.ManageWidget (w);
		}

		void RegisterChangeHandler (const std::function<void ()>& handler) override
		{
			Engine_.RegisterChangeHandler (handler);
		}

		QIcon GetPluginIcon () override
		{
			if (!Loader_)
			{
				qWarning () << Q_FUNC_INFO
						<< "requested plugin icon without a loader";
				return QIcon { "lcicons:/resources/images/defaultpluginicon.svg" };
			}

			auto basename = QFileInfo { Loader_->GetFileName () }.baseName ();
			return Engine_.GetPluginIcon (basename.section ('_', 1));
		}
	};

	CoreProxy::CoreProxy (Loaders::IPluginLoader_ptr loader)
	: EM_ { new EntityManager { this } }
	, IconThemeMgr_ { std::make_shared<IconThemeManagerProxy> (std::move (loader)) }
	{
	}

	QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
	{
		return Core::Instance ().GetNetworkAccessManager ();
	}

	IShortcutProxy* CoreProxy::GetShortcutProxy () const
	{
		return Core::Instance ().GetShortcutProxy ();
	}

	QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
	{
		return Core::Instance ().MapToSource (index);
	}

	Util::BaseSettingsManager* CoreProxy::GetSettingsManager () const
	{
		return XmlSettingsManager::Instance ();
	}

	IRootWindowsManager* CoreProxy::GetRootWindowsManager () const
	{
		return Core::Instance ().GetRootWindowsManager ();
	}

	IIconThemeManager* CoreProxy::GetIconThemeManager () const
	{
		return IconThemeMgr_.get ();
	}

	IColorThemeManager* CoreProxy::GetColorThemeManager () const
	{
		return &ColorThemeEngine::Instance ();
	}

	ITagsManager* CoreProxy::GetTagsManager () const
	{
		return &TagsManager::Instance ();
	}

	QStringList CoreProxy::GetSearchCategories () const
	{
		QStringList result;
		for (const auto& finder : Core::Instance ().GetPluginManager ()->GetAllCastableTo<IFinder*> ())
			result += finder->GetCategories ();
		result.removeDuplicates ();
		std::sort (result.begin (), result.end ());
		return result;
	}

	IPluginsManager* CoreProxy::GetPluginsManager () const
	{
		return Core::Instance ().GetPluginManager ();
	}

	IEntityManager* CoreProxy::GetEntityManager () const
	{
		return EM_;
	}

	QString CoreProxy::GetVersion () const
	{
		return LEECHCRAFT_VERSION;
	}

	void CoreProxy::RegisterSkinnable (QAction *act)
	{
		IconThemeEngine::Instance ().UpdateIconset ({ act });
	}

	bool CoreProxy::IsShuttingDown ()
	{
		return Core::Instance ().IsShuttingDown ();
	}

	ICoreProxy_ptr CoreProxy::UnsafeWithoutDeps ()
	{
		return std::make_shared<CoreProxy> (Loaders::IPluginLoader_ptr {});
	}
}
