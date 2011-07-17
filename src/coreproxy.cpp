/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include "config.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

CoreProxy::CoreProxy (QObject *parent)
: QObject (parent)
{
}

QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}

const IShortcutProxy* CoreProxy::GetShortcutProxy () const
{
	return Core::Instance ().GetShortcutProxy ();
}

QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
{
	return Core::Instance ().MapToSource (index);
}

BaseSettingsManager* CoreProxy::GetSettingsManager () const
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

QMap<int, QString> CoreProxy::GetIconPath (const QString& icon) const
{
	return SkinEngine::Instance ().GetIconPath (icon);
}

QIcon CoreProxy::GetIcon (const QString& icon, const QString& iconOff) const
{
	return SkinEngine::Instance ().GetIcon (icon, iconOff);
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

#define LC_DEFINE_REGISTER(a) \
void CoreProxy::RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t functor) \
{ \
	Core::Instance ().RegisterHook (functor); \
}
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
