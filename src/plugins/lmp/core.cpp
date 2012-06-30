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

#include "core.h"
#include <interfaces/iplugin2.h>
#include "localfileresolver.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"
#include "playlistmanager.h"
#include "devsync/syncmanager.h"
#include "interfaces/lmp/ilmpplugin.h"
#include "interfaces/lmp/isyncplugin.h"

namespace LeechCraft
{
namespace LMP
{
	Core::Core ()
	: Resolver_ (new LocalFileResolver)
	, Collection_ (new LocalCollection)
	, PLManager_ (new PlaylistManager)
	, SyncManager_ (new SyncManager)
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy ()
	{
		return Proxy_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::PostInit ()
	{
		Collection_->FinalizeInit ();
	}

	void Core::AddPlugin (QObject *pluginObj)
	{
		auto ip2 = qobject_cast<IPlugin2*> (pluginObj);
		auto ilmpPlug = qobject_cast<ILMPPlugin*> (pluginObj);

		if (!ilmpPlug)
		{
			qWarning () << Q_FUNC_INFO
					<< pluginObj
					<< "doesn't implement ILMPPlugin";
			return;
		}

		const auto& classes = ip2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.LMP.CollectionSync") &&
			qobject_cast<ISyncPlugin*> (pluginObj))
			SyncPlugins_ << pluginObj;
	}

	QList<QObject*> Core::GetSyncPlugins () const
	{
		return SyncPlugins_;
	}

	LocalFileResolver* Core::GetLocalFileResolver () const
	{
		return Resolver_;
	}

	LocalCollection* Core::GetLocalCollection () const
	{
		return Collection_;
	}

	PlaylistManager* Core::GetPlaylistManager () const
	{
		return PLManager_;
	}

	SyncManager* Core::GetSyncManager () const
	{
		return SyncManager_;
	}

	void Core::rescan ()
	{
		Collection_->Rescan ();
	}
}
}
