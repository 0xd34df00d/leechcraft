/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "carbonsmanager.h"
#include <optional>
#include <QDomElement>
#include <QXmppClient.h>
#include <QXmppMessage.h>
#include <QXmppUtils.h>
#include <QXmppXmlRegistry.h>
#include <util/sll/qtutil.h>

namespace LC::Azoth::Xoox
{
	namespace
	{
		const QString NsCarbons = "urn:xmpp:carbons:2"_qs;

		std::optional<QXmppMessage> UnwrapForwarded (const QDomElement& wrapper)
		{
			const auto& forwarded = wrapper.firstChildElement ("forwarded"_qs);
			const auto& message = forwarded.firstChildElement ("message"_qs);
			if (message.isNull ())
				return {};

			QXmppMessage result;
			result.parse (message);

			if (const auto& delay = forwarded.firstChildElement ("delay"_qs);
				!delay.isNull ())
				result.setStamp (QXmppUtils::datetimeFromString (delay.attribute ("stamp"_qs)).toLocalTime ());

			return result;
		}

		struct CarbonReceived
		{
			static constexpr std::tuple XmlTag { u"received", u"urn:xmpp:carbons:2" };

			QXmppMessage Msg_;

			bool parse (const QDomElement& el)
			{
				const auto& msg = UnwrapForwarded (el);
				if (msg)
					Msg_ = *msg;
				return msg.has_value ();
			}

			void toXml (QXmlStreamWriter*) const
			{
				throw std::runtime_error { "carbons should never be serialized this way" };
			}
		};

		struct CarbonSent
		{
			static constexpr std::tuple XmlTag { u"sent", u"urn:xmpp:carbons:2" };

			QXmppMessage Msg_;

			bool parse (const QDomElement& el)
			{
				const auto& msg = UnwrapForwarded (el);
				if (msg)
					Msg_ = *msg;
				return msg.has_value ();
			}

			void toXml (QXmlStreamWriter*) const
			{
				throw std::runtime_error { "carbons should never be serialized this way" };
			}
		};

		[[maybe_unused]]
		const bool Registered = []
		{
			QXmpp::Xml::Registry::registerElement<CarbonReceived> (QXmpp::Xml::Scope::Message);
			QXmpp::Xml::Registry::registerElement<CarbonSent> (QXmpp::Xml::Scope::Message);
			return true;
		} ();
	}

	QStringList CarbonsManager::discoveryFeatures () const
	{
		return { NsCarbons };
	}

	bool CarbonsManager::handleStanza (const QDomElement& stanza)
	{
		if (stanza.tagName () == "iq"_qs &&
				stanza.attribute ("id"_qs) == LastReqId_)
		{
			LastReqId_.clear ();

			if (stanza.attribute ("type"_qs) == "result"_qs)
			{
				LastConfirmedState_ = LastReqState_;
				emit stateChanged (LastConfirmedState_);
			}
			else
			{
				QXmppIq iq;
				iq.parse (stanza);
				emit stateChangeError (iq);
			}

			return true;
		}

		return false;
	}

	void CarbonsManager::SetEnabled (bool enabled)
	{
		QXmppIq iq { QXmppIq::Set };

		QXmppElement elem;
		elem.setTagName (enabled ? "enable"_qs : "disable"_qs);
		elem.setAttribute ("xmlns"_qs, NsCarbons);
		iq.setExtensions ({ elem });

		client ()->sendPacket (iq);

		LastReqId_ = iq.id ();
		LastReqState_ = enabled;
	}

	bool CarbonsManager::IsEnabled () const
	{
		return LastConfirmedState_;
	}

	bool CarbonsManager::CheckMessage (const QXmppMessage& msg)
	{
		if (const auto& fromBare = QXmppUtils::jidToBareJid (msg.from ());
			!fromBare.isEmpty () && fromBare != client ()->configuration ().jidBare ())
			return false;

		if (const auto& received = msg.extensions ().get<CarbonReceived> ())
		{
			HandleMessage (received->Msg_);
			return true;
		}

		if (const auto& sent = msg.extensions ().get<CarbonSent> ())
		{
			HandleMessage (sent->Msg_);
			return true;
		}

		return false;
	}

	void CarbonsManager::HandleMessage (const QXmppMessage& msg)
	{
		if (msg.to ().isEmpty ())
			return;

		emit gotMessage (msg);
	}
}
