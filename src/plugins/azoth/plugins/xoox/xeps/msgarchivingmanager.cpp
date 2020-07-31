/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgarchivingmanager.h"
#include <algorithm>
#include <functional>
#include <QDomElement>
#include <QXmppClient.h>
#include <util/sll/qtutil.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const QString NsArchive = "urn:xmpp:archive";

	namespace
	{
		template<typename T>
		class BaseConverter
		{
			const QString Str_;
			const T T_;

			const QString DefStr_;
			const T DefT_;
		protected:
			QMap<QString, T> Map_;
		public:
			BaseConverter (const QString& str, const QString& defStr, const T defT)
			: Str_ (str)
			, T_ (defT)
			, DefStr_ (defStr)
			, DefT_ (defT)
			{
			}

			BaseConverter (const T t, const QString& defStr, const T defT)
			: Str_ (QString ())
			, T_ (t)
			, DefStr_ (defStr)
			, DefT_ (defT)
			{
			}

			operator QString () const
			{
				return Map_.key (T_, DefStr_);
			}

			operator T () const
			{
				return Map_.value (Str_, DefT_);
			}
		};

		class OTRConverter : public BaseConverter<MsgArchOTR>
		{
		public:
			OTRConverter (const QString& str)
			: BaseConverter (str, "concede", MsgArchOTR::Concede)
			{
				InitMap ();
			}

			OTRConverter (MsgArchOTR otr)
			: BaseConverter (otr, "concede", MsgArchOTR::Concede)
			{
				InitMap ();
			}
		private:
			void InitMap ()
			{
				Map_ ["approve"] = MsgArchOTR::Approve;
				Map_ ["concede"] = MsgArchOTR::Concede;
				Map_ ["forbid"] = MsgArchOTR::Forbid;
				Map_ ["oppose"] = MsgArchOTR::Oppose;
				Map_ ["prefer"] = MsgArchOTR::Prefer;
				Map_ ["require"] = MsgArchOTR::Require;
			}
		};

		class SaveConverter : public BaseConverter<MsgArchSave>
		{
		public:
			SaveConverter (const QString& str)
			: BaseConverter (str, "false", MsgArchSave::False)
			{
				InitMap ();
			}

			SaveConverter (MsgArchSave save)
			: BaseConverter (save, "false", MsgArchSave::False)
			{
				InitMap ();
			}
		private:
			void InitMap ()
			{
				Map_ ["body"] = MsgArchSave::Body;
				Map_ ["false"] = MsgArchSave::False;
				Map_ ["message"] = MsgArchSave::Message;
				Map_ ["stream"] = MsgArchSave::Stream;
			}
		};

		class MethodConverter : public BaseConverter<MsgArchMethod>
		{
		public:
			MethodConverter (const QString& str)
			: BaseConverter (str, "manual", MsgArchMethod::Manual)
			{
				InitMap ();
			}

			MethodConverter (MsgArchMethod meth)
			: BaseConverter (meth, "manual", MsgArchMethod::Manual)
			{
				InitMap ();
			}
		private:
			void InitMap ()
			{
				Map_ ["auto"] = MsgArchMethod::Auto;
				Map_ ["local"] = MsgArchMethod::Local;
				Map_ ["manual"] = MsgArchMethod::Manual;
			}
		};

		class MethodPolicyConverter : public BaseConverter<MsgArchMethodPolicy>
		{
		public:
			MethodPolicyConverter (const QString& str)
			: BaseConverter (str, "prefer", MsgArchMethodPolicy::Prefer)
			{
				InitMap ();
			}

			MethodPolicyConverter (MsgArchMethodPolicy pol)
			: BaseConverter (pol, "prefer", MsgArchMethodPolicy::Prefer)
			{
				InitMap ();
			}
		private:
			void InitMap ()
			{
				Map_ ["concede"] = MsgArchMethodPolicy::Concede;
				Map_ ["forbid"] = MsgArchMethodPolicy::Forbid;
				Map_ ["prefer"] = MsgArchMethodPolicy::Prefer;
			}
		};
	}

	bool operator< (MsgArchMethod m1, MsgArchMethod m2)
	{
		return static_cast<int> (m1) < static_cast<int> (m2);
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
		QXmppElement elem;
		elem.setTagName ("pref");
		elem.setAttribute ("xmlns", NsArchive);

		QXmppIq iq;
		iq.setExtensions (QXmppElementList () << elem);
		client ()->sendPacket (iq);
	}

	MsgArchPrefs MsgArchivingManager::GetPrefs () const
	{
		return Prefs_;
	}

	void MsgArchivingManager::SetArchSetting (const MsgArchSetting& setting, const QString& jid)
	{
		QXmppElement def;
		def.setTagName (jid.isEmpty () ? "default" : "item");
		def.setAttribute ("otr", OTRConverter (setting.OTR_));
		def.setAttribute ("save", SaveConverter (setting.Save_));
		if (setting.Expire_ > 0)
			def.setAttribute ("expire", QString::number (setting.Expire_));
		if (!jid.isEmpty ())
			def.setAttribute ("jid", jid);

		QXmppElement pref;
		pref.setTagName ("pref");
		pref.setAttribute ("xmlns", NsArchive);
		pref.appendChild (def);

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (QXmppElementList () << pref);
		client ()->sendPacket (iq);
	}

	void MsgArchivingManager::SetMethodPolicies (const QMap<MsgArchMethod, MsgArchMethodPolicy>& map)
	{
		if (map.isEmpty ())
			return;

		QXmppElement pref;
		pref.setTagName ("pref");
		pref.setAttribute ("xmlns", NsArchive);

		for (const auto& [meth, policy] : Util::Stlize (map))
		{
			QXmppElement elem;
			elem.setTagName ("method");
			elem.setAttribute ("type", MethodConverter (meth));
			elem.setAttribute ("use", MethodPolicyConverter (policy));
			pref.appendChild (elem);
		}

		QXmppIq iq (QXmppIq::Set);
		iq.setExtensions (QXmppElementList () << pref);
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

		auto handleMeth = [this] (const QDomElement& meth)
		{
			Prefs_.MethodPolicies_ [MethodConverter (meth.attribute ("type"))] =
					MethodPolicyConverter (meth.attribute ("use"));
		};
		handleMeth (autoMeth);
		handleMeth (localMeth);
		handleMeth (manualMeth);

		auto handleSetting = [] (const QDomElement& elem) -> MsgArchSetting
		{
			MsgArchSetting setting =
			{
				OTRConverter (elem.attribute ("otr")),
				SaveConverter (elem.attribute ("save")),
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

		emit archPreferencesChanged ();
	}
}
}
}
