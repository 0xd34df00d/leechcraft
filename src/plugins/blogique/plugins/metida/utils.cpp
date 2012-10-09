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

#include "utils.h"
#include <QMap>
#include <QObject>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
namespace MetidaUtils
{
	namespace
	{
		struct ErrorCode2Message
		{
			QMap<int, QString> ErrorCode2Message_;
			ErrorCode2Message ()
			{
				ErrorCode2Message_ [100] = QObject::tr ("Invalid username");
				ErrorCode2Message_ [101] = QObject::tr ("Invalid password");
				ErrorCode2Message_ [102] = QObject::tr ("Can't use custom/private security in communities");
				ErrorCode2Message_ [103] = QObject::tr ("Poll error");
				ErrorCode2Message_ [104] = QObject::tr ("Error adding one or more friends");
				ErrorCode2Message_ [105] = QObject::tr ("Challenge expired");
				ErrorCode2Message_ [150] = QObject::tr ("Can't post as non-user");
				ErrorCode2Message_ [151] = QObject::tr ("Banned from journal");
				ErrorCode2Message_ [152] = QObject::tr ("Can't post back-dated entries in a non-personal journal");
				ErrorCode2Message_ [153] = QObject::tr ("Incorrent time value");
				ErrorCode2Message_ [154] = QObject::tr ("Can't add a redirected account as a friend");
				ErrorCode2Message_ [155] = QObject::tr ("Non-validated email address");
				ErrorCode2Message_ [156] = QObject::tr ("Protocol authentication denied due to user’s failure to accept TOS");
				ErrorCode2Message_ [157] = QObject::tr ("Tags error");

				ErrorCode2Message_ [200] = QObject::tr ("Missing required argument(s)");
				ErrorCode2Message_ [201] = QObject::tr ("Unknown method");
				ErrorCode2Message_ [202] = QObject::tr ("Too many arguments");
				ErrorCode2Message_ [203] = QObject::tr ("Invalid argument(s)");
				ErrorCode2Message_ [204] = QObject::tr ("Invalid metadata datatype");
				ErrorCode2Message_ [205] = QObject::tr ("Unknown metadata");
				ErrorCode2Message_ [206] = QObject::tr ("Invalid destination journal username");
				ErrorCode2Message_ [207] = QObject::tr ("Protocol version mismatch");
				ErrorCode2Message_ [208] = QObject::tr ("Invalid text encoding");
				ErrorCode2Message_ [209] = QObject::tr ("Parameter out of range");
				ErrorCode2Message_ [210] = QObject::tr ("Client tried to edit with corrupt data. Preventing");
				ErrorCode2Message_ [211] = QObject::tr ("Invalid or malformed tag list");
				ErrorCode2Message_ [212] = QObject::tr ("Message body is too long");
				ErrorCode2Message_ [213] = QObject::tr ("Message body is empty");
				ErrorCode2Message_ [214] = QObject::tr ("Message looks like spam");

				ErrorCode2Message_ [300] = QObject::tr ("Don't have access to requested journal");
				ErrorCode2Message_ [301] = QObject::tr ("Access of restricted feature");
				ErrorCode2Message_ [302] = QObject::tr ("Can't edit post from requested journal");
				ErrorCode2Message_ [303] = QObject::tr ("Can't edit post in this community");
				ErrorCode2Message_ [304] = QObject::tr ("Can't delete post in this community");
				ErrorCode2Message_ [305] = QObject::tr ("Action forbidden; account is suspended");
				ErrorCode2Message_ [306] = QObject::tr ("This journal is temporarily in read-only mode. Try again in a couple minutes");
				ErrorCode2Message_ [307] = QObject::tr ("Selected journal no longer exists");
				ErrorCode2Message_ [308] = QObject::tr ("Account is locked and cannot be used");
				ErrorCode2Message_ [309] = QObject::tr ("Account is marked as a memorial (journal is locked and does not accept comments");
				ErrorCode2Message_ [310] = QObject::tr ("Account user needs to be age-verified before use");
				ErrorCode2Message_ [311] = QObject::tr ("Access temporarily disabled");
				ErrorCode2Message_ [312] = QObject::tr ("Not allowed to add tags to entries in this journal");
				ErrorCode2Message_ [313] = QObject::tr ("Must use existing tags for entries in this journal (can't create new ones)");
				ErrorCode2Message_ [314] = QObject::tr ("Only paid users are allowed to use this request");
				ErrorCode2Message_ [315] = QObject::tr ("User messaging is currently disabled");
				ErrorCode2Message_ [316] = QObject::tr ("Poster is read-only and cannot post entries");
				ErrorCode2Message_ [317] = QObject::tr ("Journal is read-only and entries cannot be posted to it");
				ErrorCode2Message_ [318] = QObject::tr ("Poster is read-only and cannot edit entries");
				ErrorCode2Message_ [319] = QObject::tr ("Journal is read-only and its entries cannot be edited");
				ErrorCode2Message_ [320] = QObject::tr ("Sorry, there was a problem with entry content");
				ErrorCode2Message_ [321] = QObject::tr ("Sorry, deleting is temporary disabled. Entry is 'private' now");

				ErrorCode2Message_ [402] = QObject::tr ("Your IP address has been temporarily banned for exceeding the login failure rate");
				ErrorCode2Message_ [404] = QObject::tr ("Cannot post");
				ErrorCode2Message_ [405] = QObject::tr ("Post frequency limit");
				ErrorCode2Message_ [406] = QObject::tr ("Client is making repeated requests. Perhaps it's broken?");
				ErrorCode2Message_ [407] = QObject::tr ("Moderation queue full");
				ErrorCode2Message_ [408] = QObject::tr ("Maximum queued posts for this <community+poster> combination reached");
				ErrorCode2Message_ [409] = QObject::tr ("Post is too large");
				ErrorCode2Message_ [410] = QObject::tr ("Your trial account has expired. Posting is now disabled");
				ErrorCode2Message_ [411] = QObject::tr ("Action frequency limit");

				ErrorCode2Message_ [500] = QObject::tr ("Internal server error");
				ErrorCode2Message_ [501] = QObject::tr ("Database error");
				ErrorCode2Message_ [502] = QObject::tr ("Database is temporarily unavailable");
				ErrorCode2Message_ [503] = QObject::tr ("Error obtaining necessary database lock");
				ErrorCode2Message_ [504] = QObject::tr ("Protocol mode no longer supported");
				ErrorCode2Message_ [505] = QObject::tr ("Account data format on server is old and needs to be upgraded");
				ErrorCode2Message_ [506] = QObject::tr ("Journal sync is temporarily unavailable");
			}
		};
	}

	QString GetLocalizedErrorMessage (int errorCode)
	{
		static ErrorCode2Message e2msg;
		if (!e2msg.ErrorCode2Message_.contains (errorCode))
			return QString ();

		return e2msg.ErrorCode2Message_ [errorCode];
	}

	QString GetStringForAccess (Access access)
	{
		switch (access)
		{
			case Access::Private:
				return "private";
			case Access::FriendsOnly:
			case Access::Custom:
				return "usemask";
			case Access::Public:
			default:
				return "public";
		}
	}

	QString GetStringForAdultContent (AdultContent adult)
	{
		switch (adult)
		{
			case AdultContent::AdultsFrom14:
				return "concepts";
			case AdultContent::AdultsFrom18:
				return "explicit";
			case AdultContent::WithoutAdultContent:
			default:
				return "none";
		}

	}

}
}
}
}

