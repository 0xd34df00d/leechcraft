/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "inbandaccountactions.h"
#include <util/sll/slotclosure.h>
#include <interfaces/azoth/iproxyobject.h>
#include "accountsettingsholder.h"
#include "clientconnection.h"
#include "glooxaccount.h"
#include "util.h"
#include "xmppelementdescription.h"

namespace LC::Azoth::Xoox
{
	InBandAccountActions::InBandAccountActions (ClientConnection& conn, GlooxAccount& acc)
	: Conn_ { conn }
	, Acc_ { acc }
	{
	}

	void InBandAccountActions::UpdateServerPassword (const QString& newPass)
	{
		if (newPass.isEmpty ())
			return;

		const auto& jid = Acc_.GetSettings ()->GetJID ();
		const auto aPos = jid.indexOf ('@');

		XmppElementDescription queryDescr
		{
			.TagName_ = "query",
			.Attributes_ = { { "xmlns", XooxUtil::NsRegister } },
			.Children_ =
			{
				{ .TagName_ = "username", .Value_ = aPos > 0 ? jid.left (aPos) : jid },
				{ .TagName_ = "password", .Value_ = newPass },
			}
		};

		QXmppIq iq (QXmppIq::Set);
		iq.setTo (Acc_.GetDefaultReqHost ());
		iq.setExtensions ({ ToElement (queryDescr) });

		Conn_.SendPacketWCallback (iq,
				[this, newPass] (const QXmppIq& reply)
				{
					if (reply.type () != QXmppIq::Result)
						return;

					emit Acc_.serverPasswordUpdated (newPass);
					Acc_.GetParentProtocol ()->GetProxyObject ()->SetPassword (newPass, &Acc_);
					Conn_.SetPassword (newPass);
				});
	}

	namespace
	{
		QXmppIq MakeDeregisterIq ()
		{
			XmppElementDescription queryDescr
			{
				.TagName_ = "query",
				.Attributes_ = { { "xmlns", XooxUtil::NsRegister } },
				.Children_ = { { .TagName_ = "remove" } }
			};

			QXmppIq iq { QXmppIq::Set };
			iq.setExtensions ({ ToElement (queryDescr) });
			return iq;
		}
	}

	void InBandAccountActions::CancelRegistration ()
	{
		const auto worker = [this]
		{
			Conn_.SendPacketWCallback (MakeDeregisterIq (),
					[this] (const QXmppIq& reply)
					{
						if (reply.type () == QXmppIq::Result)
						{
							Acc_.GetParentProtocol ()->RemoveAccount (&Acc_);
							Acc_.ChangeState ({ SOffline, {} });
						}
						else
							qWarning () << Q_FUNC_INFO
									<< "unable to cancel the registration:"
									<< reply.type ();
					});
		};

		if (Acc_.GetState ().State_ != SOffline)
		{
			worker ();
			return;
		}

		Acc_.ChangeState ({ SOnline, {} });
		new Util::SlotClosure<Util::ChoiceDeletePolicy>
		{
			[this, worker]
			{
				switch (Acc_.GetState ().State_)
				{
				case SOffline:
				case SError:
				case SConnecting:
					return Util::ChoiceDeletePolicy::Delete::No;
				default:
					break;
				}

				worker ();

				return Util::ChoiceDeletePolicy::Delete::Yes;
			},
			&Acc_,
			SIGNAL (statusChanged (EntryStatus)),
			&Acc_
		};
	}
}
