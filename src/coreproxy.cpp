/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "coreproxy.h"
#include <algorithm>
#include <interfaces/ifinder.h>
#include "core.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "skinengine.h"
#include "tagsmanager.h"
#include "mwproxy.h"
#include "config.h"

namespace LeechCraft
{
	CoreProxy::CoreProxy (QObject *parent)
	: QObject (parent)
	, MWProxy_ (new MWProxy (this))
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

	IMWProxy* CoreProxy::GetMWProxy () const
	{
		return MWProxy_;
	}

	QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
	{
		return Core::Instance ().MapToSource (index);
	}

	Util::BaseSettingsManager* CoreProxy::GetSettingsManager () const
	{
		return XmlSettingsManager::Instance ();
	}

	QMainWindow* CoreProxy::GetMainWindow () const
	{
		return Core::Instance ().GetReallyMainWindow ();
	}

	ICoreTabWidget* CoreProxy::GetTabWidget () const
	{
		return Core::Instance ().GetReallyMainWindow ()->GetTabWidget ();
	}

	QIcon CoreProxy::GetIcon (const QString& icon, const QString& iconOff) const
	{
		return SkinEngine::Instance ().GetIcon (icon, iconOff);
	}

	void CoreProxy::UpdateIconset (const QList<QAction*>& actions) const
	{
		SkinEngine::Instance ().UpdateIconSet (actions);
	}

	ITagsManager* CoreProxy::GetTagsManager () const
	{
		return &TagsManager::Instance ();
	}

	QStringList CoreProxy::GetSearchCategories () const
	{
		const QList<IFinder*>& finders = Core::Instance ().GetPluginManager ()->
			GetAllCastableTo<IFinder*> ();

		QStringList result;
		for (QList<IFinder*>::const_iterator i = finders.begin (),
				end = finders.end (); i != end; ++i)
			result += (*i)->GetCategories ();
		result.removeDuplicates ();
		std::sort (result.begin (), result.end ());
		return result;
	}

	int CoreProxy::GetID ()
	{
		return Pool_.GetID ();
	}

	void CoreProxy::FreeID (int id)
	{
		Pool_.FreeID (id);
	}

	IPluginsManager* CoreProxy::GetPluginsManager () const
	{
		return Core::Instance ().GetPluginManager ();
	}

	QString CoreProxy::GetVersion () const
	{
		return LEECHCRAFT_VERSION;
	}

	QObject* CoreProxy::GetSelf ()
	{
		return this;
	}

	void CoreProxy::RegisterSkinnable (QAction *act)
	{
		SkinEngine::Instance ().UpdateIconSet (QList<QAction*> () << act);
	}

	bool CoreProxy::IsShuttingDown ()
	{
		return Core::Instance ().IsShuttingDown ();
	}
}
