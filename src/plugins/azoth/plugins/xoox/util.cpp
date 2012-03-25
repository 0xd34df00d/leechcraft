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

#include "util.h"
#include <memory>
#include <QObject>
#include <QHash>
#include <QDomDocument>
#include <QDomElement>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QXmppPresence.h>
#include "entrybase.h"
#include "core.h"
#include "capsdatabase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
namespace XooxUtil
{
	QString RoleToString (const QXmppMucItem::Role& role)
	{
		switch (role)
		{
		case QXmppMucItem::NoRole:
			return QObject::tr ("guest");
		case QXmppMucItem::VisitorRole:
			return QObject::tr ("visitor");
		case QXmppMucItem::ParticipantRole:
			return QObject::tr ("participant");
		case QXmppMucItem::ModeratorRole:
			return QObject::tr ("moderator");
		default:
			return QObject::tr ("unspecified");
		}
	}

	QString AffiliationToString (const QXmppMucItem::Affiliation& affiliation)
	{
		switch (affiliation)
		{
		case QXmppMucItem::OutcastAffiliation:
			return QObject::tr ("outcast");
		case QXmppMucItem::NoAffiliation:
			return QObject::tr ("newcomer");
		case QXmppMucItem::MemberAffiliation:
			return QObject::tr ("member");
		case QXmppMucItem::AdminAffiliation:
			return QObject::tr ("admin");
		case QXmppMucItem::OwnerAffiliation:
			return QObject::tr ("owner");
		default:
			return QObject::tr ("unspecified");
		}
	}

	namespace
	{
		struct Node2ClientID
		{
			QHash<QString, QString> Node2ClientID_;

			Node2ClientID ()
			{
				Node2ClientID_ ["http://2010.qip.ru/caps"] = "qipinfium";
				Node2ClientID_ ["http://bitlbee.org/xmpp/caps"] = "bitlbee";
				Node2ClientID_ ["http://bombus-im.org/java"] = "bombus";
				Node2ClientID_ ["http://bombusmod.net.ru/caps"] = "bombusmod";
				Node2ClientID_ ["http://bombusmod-qd.wen.ru/caps"] = "bombusmodqd";
				Node2ClientID_ ["http://emess.eqx.su/caps"] = "emess";
				Node2ClientID_ ["http://fatal-bot.spb.ru/caps"] = "fatal-bot";
				Node2ClientID_ ["http://fatal-dev.ru/bot/caps"] = "fatal-bot";
				Node2ClientID_ ["http://gajim.org"] = "gajim";
				Node2ClientID_ ["http://gajim.org/caps"] = "gajim";
				Node2ClientID_ ["http://isida-bot.com"] = "isida-bot";
				Node2ClientID_ ["http://jabiru.mzet.net/caps"] = "jabiru";
				Node2ClientID_ ["http://jimm.net.ru/caps"] = "jimm";
				Node2ClientID_ ["http://jtalk.ustyugov.net/caps"] = "jtalk";
				Node2ClientID_ ["http://kopete.kde.org/jabber/caps"] = "kopete";
				Node2ClientID_ ["http://leechcraft.org/azoth"] = "leechcraft-azoth";
				Node2ClientID_ ["http://mail.google.com/xmpp/client/caps"] = "mail.google.com";
				Node2ClientID_ ["http://mcabber.com/caps"] = "mcabber";
				Node2ClientID_ ["http://miranda-im.org/caps"] = "miranda";
				Node2ClientID_ ["http://online.yandex.ru/caps"] = "yaonline";
				Node2ClientID_ ["http://palringo.com/caps"] = "palringo";
				Node2ClientID_ ["http://pda.qip.ru/caps"] = "qippda";
				Node2ClientID_ ["http://pidgin.im/"] = "pidgin";
				Node2ClientID_ ["http://pidgin.im/caps"] = "pidgin";
				Node2ClientID_ ["http://psi-im.org/caps"] = "psi";
				Node2ClientID_ ["http://psi-dev.googlecode.com/caps"] = "psiplus";
				Node2ClientID_ ["http://pyicqt.googlecode.com//protocol/caps"] = "pyicq-t";
				Node2ClientID_ ["http://qip.ru/caps"] = "qipinfium";
				Node2ClientID_ ["http://qip.ru/caps?QIP Mobile Java"] = "qipmobile";
				Node2ClientID_ ["http://qutim.org"] = "qutim";
				Node2ClientID_ ["http://qutim.org/"] = "qutim";
				Node2ClientID_ ["http://sip-communicator.org"] = "sip-communicator";
				Node2ClientID_ ["http://swift.im"] = "swift";
				Node2ClientID_ ["http://talkgadget.google.com/client/caps"] = "talkgadget.google.com";
				Node2ClientID_ ["http://telepathy.freedesktop.org/caps"] = "telepathy.freedesktop.org";
				Node2ClientID_ ["http://tkabber.jabber.ru"] = "tkabber";
				Node2ClientID_ ["http://tkabber.jabber.ru/"] = "tkabber";
				Node2ClientID_ ["http://trillian.im/caps"] = "trillian";
				Node2ClientID_ ["http://vacuum-im.googlecode.com"] = "vacuum";
				Node2ClientID_ ["http://www.android.com/gtalk/client/caps"] = "android";
				Node2ClientID_ ["http://www.android.com/gtalk/client/caps2"] = "android";
				Node2ClientID_ ["http://www.apple.com/ichat/caps"] = "ichat";
				Node2ClientID_ ["http://www.google.com/xmpp/client/caps"] = "talk.google.com";
				Node2ClientID_ ["http://www.igniterealtime.org/projects/smack/"] = "smack";
				Node2ClientID_ ["http://www.lonelycatgames.com/slick/caps"] = "slick";
				Node2ClientID_ ["https://www.jappix.com/"] = "jappix";
			}
		};
	}

	QString GetClientIDName (const QString& node)
	{
		static Node2ClientID n2ci;
		const QString& result = n2ci.Node2ClientID_.value (node);
		if (!result.isEmpty ())
			return result;

		if (node.startsWith ("http://bombus-im.org/java#"))
			return "bombus";

		return QString ();
	}

	namespace
	{
		struct Node2ClientHR
		{
			QHash<QString, QString> Node2ClientHR_;

			Node2ClientHR ()
			{
				Node2ClientHR_ ["http://2010.qip.ru/caps"] = "QIP Infium";
				Node2ClientHR_ ["http://bitlbee.org/xmpp/caps"] = "Bitlbee";
				Node2ClientHR_ ["http://bombus-im.org/java"] = "Bombus";
				Node2ClientHR_ ["http://bombusmod.net.ru/caps"] = "BombusMod";
				Node2ClientHR_ ["http://bombusmod-qd.wen.ru/caps"] = "BombusMod-QD";
				Node2ClientHR_ ["http://emess.eqx.su/caps"] = "EMess";
				Node2ClientHR_ ["http://fatal-bot.spb.ru/caps"] = "Fatal-bot";
				Node2ClientHR_ ["http://fatal-dev.ru/bot/caps"] = "Fatal-bot";
				Node2ClientHR_ ["http://gajim.org"] = "Gajim";
				Node2ClientHR_ ["http://gajim.org/caps"] = "Gajim";
				Node2ClientHR_ ["http://isida-bot.com"] = "iSida Bot";
				Node2ClientHR_ ["http://jabiru.mzet.net/caps"] = "Jabiru";
				Node2ClientHR_ ["http://jimm.net.ru/caps"] = "Jimm";
				Node2ClientHR_ ["http://jtalk.ustyugov.net/caps"] = "JTalk";
				Node2ClientHR_ ["http://kopete.kde.org/jabber/caps"] = "Kopete";
				Node2ClientHR_ ["http://leechcraft.org/azoth"] = "LeechCraft Azoth";
				Node2ClientHR_ ["http://mail.google.com/xmpp/client/caps"] = "GMail chat widget";
				Node2ClientHR_ ["http://mcabber.com/caps"] = "MCabber";
				Node2ClientHR_ ["http://miranda-im.org/caps"] = "Miranda IM";
				Node2ClientHR_ ["http://online.yandex.ru/caps"] = "Ya.Online";
				Node2ClientHR_ ["http://palringo.com/caps"] = "Palringo";
				Node2ClientHR_ ["http://pda.qip.ru/caps"] = "QIP PDA";
				Node2ClientHR_ ["http://pidgin.im/"] = "Pidgin IM";
				Node2ClientHR_ ["http://pidgin.im/caps"] = "Pidgin IM";
				Node2ClientHR_ ["http://psi-im.org/caps"] = "Psi";
				Node2ClientHR_ ["http://psi-dev.googlecode.com/caps"] = "Psi+";
				Node2ClientHR_ ["http://pyicqt.googlecode.com//protocol/caps"] = "PyICQ-t";
				Node2ClientHR_ ["http://qip.ru/caps"] = "QIP Infium";
				Node2ClientHR_ ["http://qip.ru/caps?QIP Mobile Java"] = "QIP Mobile";
				Node2ClientHR_ ["http://qutim.org"] = "QutIM";
				Node2ClientHR_ ["http://qutim.org/"] = "QutIM";
				Node2ClientHR_ ["http://sip-communicator.org"] = "SIP Communicator";
				Node2ClientHR_ ["http://swift.im"] = "Swift";
				Node2ClientHR_ ["http://talkgadget.google.com/client/caps"] = "Google Talk gadget";
				Node2ClientHR_ ["http://telepathy.freedesktop.org/caps"] = "Telepathy";
				Node2ClientHR_ ["http://tkabber.jabber.ru"] = "Tkabber";
				Node2ClientHR_ ["http://tkabber.jabber.ru/"] = "Tkabber";
				Node2ClientHR_ ["http://trillian.im/caps"] = "Trillian";
				Node2ClientHR_ ["http://vacuum-im.googlecode.com"] = "Vacuum-IM";
				Node2ClientHR_ ["http://www.android.com/gtalk/client/caps"] = "Android";
				Node2ClientHR_ ["http://www.android.com/gtalk/client/caps2"] = "Android";
				Node2ClientHR_ ["http://www.apple.com/ichat/caps"] = "iChat";
				Node2ClientHR_ ["http://www.google.com/xmpp/client/caps"] = "Google Talk";
				Node2ClientHR_ ["http://www.igniterealtime.org/projects/smack/"] = "Smack XMPP library";
				Node2ClientHR_ ["http://www.lonelycatgames.com/slick/caps"] = "Slick";
				Node2ClientHR_ ["https://www.jappix.com/"] = "Jappix";
			}
		};
	}

	QString GetClientHRName (const QString& node)
	{
		static Node2ClientHR n2ch;
		const QString& result = n2ch.Node2ClientHR_.value (node);
		if (!result.isEmpty ())
			return result;

		if (node.startsWith ("http://bombus-im.org/java#"))
			return "Bombus";

		return QString ();
	}

	QDomElement XmppElem2DomElem (const QXmppElement& elem)
	{
		QByteArray arr;
		QXmlStreamWriter w (&arr);
		elem.toXml (&w);

		QDomDocument doc;
		doc.setContent (arr);
		return doc.documentElement ();
	}

	QXmppElement Form2XmppElem (const QXmppDataForm& form)
	{
		QByteArray formData;
		QXmlStreamWriter w (&formData);
		form.toXml (&w);
		QDomDocument doc;
		doc.setContent (formData);
		return doc.documentElement ();
	}

	bool RunFormDialog (QWidget *widget)
	{
		QDialog *dialog (new QDialog ());
		dialog->setWindowTitle (widget->windowTitle ());
		dialog->setLayout (new QVBoxLayout ());
		dialog->layout ()->addWidget (widget);
		QDialogButtonBox *box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		dialog->layout ()->addWidget (box);
		QObject::connect (box,
				SIGNAL (accepted ()),
				dialog,
				SLOT (accept ()));
		QObject::connect (box,
				SIGNAL (rejected ()),
				dialog,
				SLOT (reject ()));

		const bool result = dialog->exec () == QDialog::Accepted;
		dialog->deleteLater ();
		return result;
	}

	bool CheckUserFeature (EntryBase *base, const QString& variant, const QString& feature)
	{
		if (variant.isEmpty ())
			return true;

		const QByteArray& ver = base->GetVariantVerString (variant);
		if (ver.isEmpty ())
			return true;

		const QStringList& feats = Core::Instance ().GetCapsDatabase ()->Get (ver);
		if (feats.isEmpty ())
			return true;

		return feats.contains (feature);
	}

	EntryStatus PresenceToStatus (const QXmppPresence& pres)
	{
		const QXmppPresence::Status& status = pres.status ();
		EntryStatus st (static_cast<State> (status.type ()), status.statusText ());
		if (pres.type () == QXmppPresence::Unavailable)
			st.State_ = SOffline;
		return st;
	}
}
}
}
}
