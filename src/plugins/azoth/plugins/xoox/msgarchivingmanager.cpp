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

#include "msgarchivingmanager.h"
#include <algorithm>
#include <functional>
#include <QDomElement>
#include <QXmppClient.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsArchive = "urn:xmpp:archive";

	namespace
	{
		MsgArchOTR OTRFromStr (const QString& str)
		{
			QMap<QString, MsgArchOTR> map;
			map ["approve"] = MsgArchOTR::Approve;
			map ["concede"] = MsgArchOTR::Concede;
			map ["forbid"] = MsgArchOTR::Forbid;
			map ["oppose"] = MsgArchOTR::Oppose;
			map ["prefer"] = MsgArchOTR::Prefer;
			map ["require"] = MsgArchOTR::Require;
			return map.value (str, MsgArchOTR::Concede);
		}

		MsgArchSave SaveFromStr (const QString& str)
		{
			QMap<QString, MsgArchSave> map;
			map ["body"] = MsgArchSave::Body;
			map ["false"] = MsgArchSave::False;
			map ["message"] = MsgArchSave::Message;
			map ["stream"] = MsgArchSave::Stream;
			return map.value (str, MsgArchSave::False);
		}

		MsgArchMethod MethodFromStr (const QString& str)
		{
			if (str == "auto")
				return MsgArchMethod::Auto;
			else if (str == "local")
				return MsgArchMethod::Local;
			else
				return MsgArchMethod::Manual;
		}

		MsgArchMethodPolicy MethodPolicyFromStr (const QString& str)
		{
			if (str == "concede")
				return MsgArchMethodPolicy::Concede;
			else if (str == "forbid")
				return MsgArchMethodPolicy::Forbid;
			else
				return MsgArchMethodPolicy::Prefer;
		}
	}

	bool operator< (MsgArchMethod m1, MsgArchMethod m2)
	{
		return static_cast<int> (m1) < static_cast<int> (m2);
	}

	MsgArchPrefs::MsgArchPrefs ()
	: Valid_ (false)
	{
	}

	MsgArchivingManager::MsgArchivingManager (ClientConnection *conn)
	: Conn_ (conn)
	{
	}

	QStringList MsgArchivingManager::discoveryFeatures () const
	{
		return QStringList (NsArchive);
	}

	bool MsgArchivingManager::handleStanza (const QDomElement& elem)
	{
		if (elem.tagName () != "iq")
			return false;

		const auto& pref = elem.firstChildElement ("pref");
		if (pref.namespaceURI () == NsArchive)
		{
			HandlePref (pref);
			return true;
		}

		return false;
	}

	void MsgArchivingManager::RequestPrefs ()
	{
		QXmppIq iq;

		QXmppElement elem;
		elem.setTagName ("pref");
		elem.setAttribute ("xmlns", NsArchive);
		iq.setExtensions (elem);

		client ()->sendPacket (iq);
	}

	void MsgArchivingManager::HandlePref (const QDomElement& prefElem)
	{
		const QDomElement& autoSave = prefElem.firstChildElement ("auto");
		const QDomElement& defaultPref = prefElem.firstChildElement ("default");
		QDomElement autoMeth, localMeth, manualMeth;
		QDomElement meth = prefElem.firstChildElement ("method");
		while (!meth.isNull ())
		{
			const auto& type = meth.attribute ("type");
			if (type == "auto")
				autoMeth = meth;
			else if (type == "local")
				localMeth = meth;
			else if (type == "manual")
				manualMeth = meth;

			meth = meth.nextSiblingElement ("method");
		}

		if (!Prefs_.Valid_)
		{
			std::vector<QDomElement> elems =
					{ autoSave, defaultPref, autoMeth, localMeth, manualMeth };
			Prefs_.Valid_ = std::all_of (elems.begin (), elems.end (),
					[] (const QDomElement& elem) { return !elem.isNull (); });
		}

		Prefs_.AutoSave_ = autoSave.attribute ("save") == "true";

		auto handleMeth = [&Prefs_] (const QDomElement& meth)
		{
			Prefs_.MethodPolicies_ [MethodFromStr (meth.attribute ("type"))] =
					MethodPolicyFromStr (meth.attribute ("use"));
		};
		handleMeth (autoMeth);
		handleMeth (localMeth);
		handleMeth (manualMeth);

		auto handleSetting = [] (const QDomElement& elem)
		{
			MsgArchSetting setting =
			{
				OTRFromStr (elem.attribute ("otr")),
				SaveFromStr (elem.attribute ("save")),
				elem.attribute ("expire").toLongLong ()
			};
			return setting;
		};
		Prefs_.Default_ = handleSetting (defaultPref);

		QDomElement item = prefElem.firstChildElement ("item");
		while (!item.isNull ())
		{
			Prefs_.ItemSettings_ [item.attribute ("jid")] = handleSetting (item);
			item = item.nextSiblingElement ("item");
		}
	}
}
}
}
