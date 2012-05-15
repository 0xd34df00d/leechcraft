/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <interfaces/blogique/ipluginproxy.h>
#include "ljbloggingplatform.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	Core::Core ()
	: PluginProxy_ (0)
	{
		InitErrorMessages ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
		if (LJPlatform_)
		{
			LJPlatform_->SetPluginProxy (PluginProxy_);
			LJPlatform_->Prepare ();
		}
	}

	void Core::CreateBloggingPlatfroms (QObject *parentPlatform)
	{
		LJPlatform_ = std::make_shared<LJBloggingPlatform> (parentPlatform);
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetCoreProxy ()
	{
		return Proxy_;
	}

	QObjectList Core::GetBloggingPlatforms () const
	{
		return LJPlatform_ ? QObjectList () << LJPlatform_.get () : QObjectList ();
	}

	void Core::SetPluginProxy (QObject *pluginProxy)
	{
		PluginProxy_ = pluginProxy;
	}

	IPluginProxy* Core::GetPluginProxy ()
	{
		return qobject_cast<IPluginProxy*> (PluginProxy_);
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	QString Core::GetLocalizedErrorMessage (int errorCode)
	{
		if (!ErrorCode2Message_.contains (errorCode))
			return QString ();

		return ErrorCode2Message_ [errorCode];
	}

	void Core::InitErrorMessages ()
	{
		ErrorCode2Message_ [100] = tr ("Invalid username");
		ErrorCode2Message_ [101] = tr ("Invalid password");
		ErrorCode2Message_ [102] = tr ("Can't use custom/private security in communities");
		ErrorCode2Message_ [103] = tr ("Poll error");
		ErrorCode2Message_ [104] = tr ("Error adding one or more friends");
		ErrorCode2Message_ [105] = tr ("Challenge expired");
		ErrorCode2Message_ [150] = tr ("Can't post as non-user");
		ErrorCode2Message_ [151] = tr ("Banned from journal");
		ErrorCode2Message_ [152] = tr ("Can't post back-dated entries in a non-personal journal");
		ErrorCode2Message_ [153] = tr ("Incorrent time value");
		ErrorCode2Message_ [154] = tr ("Can't add a redirected account as a friend");
		ErrorCode2Message_ [155] = tr ("Non-validated email address");
		ErrorCode2Message_ [156] = tr ("Protocol authentication denied due to user’s failure to accept TOS");
		ErrorCode2Message_ [157] = tr ("Tags error");

		ErrorCode2Message_ [200] = tr ("Missing required argument(s)");
		ErrorCode2Message_ [201] = tr ("Unknown method");
		ErrorCode2Message_ [202] = tr ("Too many arguments");
		ErrorCode2Message_ [203] = tr ("Invalid argument(s)");
		ErrorCode2Message_ [204] = tr ("Invalid metadata datatype");
		ErrorCode2Message_ [205] = tr ("Unknown metadata");
		ErrorCode2Message_ [206] = tr ("Invalid destination journal username");
		ErrorCode2Message_ [207] = tr ("Protocol version mismatch");
		ErrorCode2Message_ [208] = tr ("Invalid text encoding");
		ErrorCode2Message_ [209] = tr ("Parameter out of range");
		ErrorCode2Message_ [210] = tr ("Client tried to edit with corrupt data. Preventing");
		ErrorCode2Message_ [211] = tr ("Invalid or malformed tag list");
		ErrorCode2Message_ [212] = tr ("Message body is too long");
		ErrorCode2Message_ [213] = tr ("Message body is empty");
		ErrorCode2Message_ [214] = tr ("Message looks like spam");

		ErrorCode2Message_ [300] = tr ("Don't have access to requested journal");
		ErrorCode2Message_ [301] = tr ("Access of restricted feature");
		ErrorCode2Message_ [302] = tr ("Can't edit post from requested journal");
		ErrorCode2Message_ [303] = tr ("Can't edit post in this community");
		ErrorCode2Message_ [304] = tr ("Can't delete post in this community");
		ErrorCode2Message_ [305] = tr ("Action forbidden; account is suspended");
		ErrorCode2Message_ [306] = tr ("This journal is temporarily in read-only mode. Try again in a couple minutes");
		ErrorCode2Message_ [307] = tr ("Selected journal no longer exists");
		ErrorCode2Message_ [308] = tr ("Account is locked and cannot be used");
		ErrorCode2Message_ [309] = tr ("Account is marked as a memorial (journal is locked and does not accept comments");
		ErrorCode2Message_ [310] = tr ("Account user needs to be age-verified before use");
		ErrorCode2Message_ [311] = tr (" Access temporarily disabled");
		ErrorCode2Message_ [312] = tr ("Not allowed to add tags to entries in this journal");
		ErrorCode2Message_ [313] = tr ("Must use existing tags for entries in this journal (can't create new ones)");
		ErrorCode2Message_ [314] = tr ("Only paid users are allowed to use this request");
		ErrorCode2Message_ [315] = tr ("User messaging is currently disabled");
		ErrorCode2Message_ [316] = tr ("Poster is read-only and cannot post entries");
		ErrorCode2Message_ [317] = tr ("Journal is read-only and entries cannot be posted to it");
		ErrorCode2Message_ [318] = tr ("Poster is read-only and cannot edit entries");
		ErrorCode2Message_ [319] = tr ("Journal is read-only and its entries cannot be edited");
		ErrorCode2Message_ [320] = tr ("Sorry, there was a problem with entry content");
		ErrorCode2Message_ [321] = tr ("Sorry, deleting is temporary disabled. Entry is 'private' now");

		ErrorCode2Message_ [402] = tr ("Your IP address has been temporarily banned for exceeding the login failure rate");
		ErrorCode2Message_ [404] = tr ("Cannot post");
		ErrorCode2Message_ [405] = tr ("Post frequency limit");
		ErrorCode2Message_ [406] = tr ("Client is making repeated requests. Perhaps it's broken?");
		ErrorCode2Message_ [407] = tr ("Moderation queue full");
		ErrorCode2Message_ [408] = tr ("Maximum queued posts for this <community+poster> combination reached");
		ErrorCode2Message_ [409] = tr (" Post is too large");
		ErrorCode2Message_ [410] = tr ("Your trial account has expired. Posting is now disabled");
		ErrorCode2Message_ [411] = tr ("Action frequency limit");

		ErrorCode2Message_ [500] = tr ("Internal server error");
		ErrorCode2Message_ [501] = tr ("Database error");
		ErrorCode2Message_ [502] = tr ("Database is temporarily unavailable");
		ErrorCode2Message_ [503] = tr ("Error obtaining necessary database lock");
		ErrorCode2Message_ [504] = tr ("Protocol mode no longer supported");
		ErrorCode2Message_ [505] = tr ("Account data format on server is old and needs to be upgraded");
		ErrorCode2Message_ [506] = tr ("Journal sync is temporarily unavailable");
	}

}
}
}
