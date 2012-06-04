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
#include "localfileresolver.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"
#include "playlistmanager.h"

namespace LeechCraft
{
namespace LMP
{
	Core::Core ()
	: Resolver_ (new LocalFileResolver)
	, Collection_ (new LocalCollection)
	, PLManager_ (new PlaylistManager)
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

	void Core::rescan ()
	{
		Collection_->Rescan ();
	}
}
}
