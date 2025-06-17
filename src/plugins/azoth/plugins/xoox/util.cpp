/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
#include <QDataStream>
#include <QFile>
#include <QtDebug>
#include <QXmppPresence.h>
#include <QXmppUtils.h>
#include <QXmppGlobal.h>
#include <util/sll/parsejson.h>
#include <util/sll/prelude.h>
#include "entrybase.h"
#include "capsdatabase.h"
#include "glooxaccount.h"

QDataStream& operator<< (QDataStream& out, const QXmppDiscoveryIq::Identity& id)
{
	out << static_cast<quint8> (1)
		<< id.category ()
		<< id.language ()
		<< id.name ()
		<< id.type ();
	return out;
}

QDataStream& operator>> (QDataStream& in, QXmppDiscoveryIq::Identity& id)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
	{
		qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
		return in;
	}

	QString category, language, name, type;
	in >> category
		>> language
		>> name
		>> type;
	id.setCategory (category);
	id.setLanguage (language);
	id.setName (name);
	id.setType (type);

	return in;
}

namespace LC
{
namespace Azoth
{
namespace Xoox
{
namespace XooxUtil
{
	const QString NsRegister { "jabber:iq:register" };

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
		QPair<QStringList, StaticClientInfo> ParseClientIdObj (const QVariantMap& map)
		{
			const auto& nodeVar = map ["node"];

			const auto nodes = [&nodeVar] () -> QStringList
			{
				switch (nodeVar.metaType ().id ())
				{
				case QMetaType::QString:
					return { nodeVar.toString () };
				case QMetaType::QStringList:
					return nodeVar.toStringList ();
				case QMetaType::QVariantList:
					return Util::Map (nodeVar.toList (), &QVariant::toString);
				default:
					qWarning () << "unsupported node field" << nodeVar;
					return {};
				}
			} ();

			const auto& id = map ["id"].toString ();
			const auto& name = map ["name"].toString ();

			if (nodes.isEmpty () || id.isEmpty () || name.isEmpty ())
				qWarning () << "missing data for map" << map;

			return { nodes, { id, name } };
		}

		class StaticClientInfoHolder
		{
			QHash<QString, StaticClientInfo> FullMatches_;
			QList<QPair<QString, StaticClientInfo>> PartialMatches_;
		public:
			StaticClientInfoHolder ()
			{
				QFile file { ":/azoth/xoox/resources/data/clientids.json" };
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << "unable to open file:" << file.errorString ();
					return;
				}

				const auto& json = Util::ParseJson (&file, Q_FUNC_INFO).toMap ();

				for (const auto& itemVar : json ["fullMatches"].toList ())
				{
					const auto& pair = ParseClientIdObj (itemVar.toMap ());
					for (const auto& node : pair.first)
					{
						if (FullMatches_.contains (node))
							qWarning () << Q_FUNC_INFO
									<< "duplicate node:"
									<< node;
						FullMatches_ [node] = pair.second;
					}
				}

				for (const auto& itemVar : json ["partialMatches"].toList ())
				{
					const auto& pair = ParseClientIdObj (itemVar.toMap ());
					for (const auto& node : pair.first)
						PartialMatches_.push_back ({ node, pair.second });
				}

				std::sort (PartialMatches_.begin (), PartialMatches_.end (), Util::ComparingBy (Util::Fst));
			}

			StaticClientInfo operator[] (const QString& node) const
			{
				const auto pos = FullMatches_.find (node);
				if (pos != FullMatches_.end ())
					return *pos;

				for (auto i = PartialMatches_.begin (), end = PartialMatches_.end (); i != end; ++i)
					if (node.startsWith (i->first))
						return i->second;

				return {};
			}
		};
	}

	StaticClientInfo GetStaticClientInfo (const QString& node)
	{
		static const StaticClientInfoHolder holder;
		return holder [node];
	}

	QDomElement XmppElem2DomElem (const QXmppElement& elem)
	{
		QByteArray arr;
		QXmlStreamWriter w (&arr);
		elem.toXml (&w);

		QDomDocument doc;
		if (!doc.setContent (arr, QDomDocument::ParseOption::UseNamespaceProcessing))
			qCritical () << "unable to set XML contents" << arr;
		return doc.documentElement ();
	}

	QXmppElement Form2XmppElem (const QXmppDataForm& form)
	{
		QByteArray formData;
		QXmlStreamWriter w (&formData);
		form.toXml (&w);
		QDomDocument doc;
		if (!doc.setContent (formData))
			qCritical () << Q_FUNC_INFO
					<< "unable to set XML contents"
					<< formData;
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

	bool CheckUserFeature (EntryBase *base, const QString& variant,
			const QString& feature, const CapsDatabase *capsDB)
	{
		if (variant.isEmpty ())
			return true;

		const QByteArray& ver = base->GetVariantVerString (variant);
		if (ver.isEmpty ())
			return true;

		const auto& feats = capsDB->Get (ver);
		if (feats.isEmpty ())
			return true;

		return feats.contains (feature);
	}

	QXmppMessage Forwarded2Message (const QXmppElement& wrapper)
	{
		const auto& forwardedElem = wrapper.tagName () == "forwarded" ?
				wrapper :
				wrapper.firstChildElement ("forwarded");
		if (forwardedElem.isNull ())
			return {};

		const auto& messageElem = forwardedElem.firstChildElement ("message");
		if (messageElem.isNull ())
			return {};

		QXmppMessage original;
		original.parse (messageElem.sourceDomElement ());

		auto delayElem = forwardedElem.firstChildElement ("delay");
		if (!delayElem.isNull ())
		{
			const auto& sourceDT = QXmppUtils::datetimeFromString (delayElem.attribute ("stamp"));
			original.setStamp (sourceDT.toLocalTime ());
		}

		return original;
	}

	EntryStatus PresenceToStatus (const QXmppPresence& pres)
	{
		EntryStatus st (static_cast<State> (pres.availableStatusType () + 1), pres.statusText ());
		if (pres.type () == QXmppPresence::Unavailable)
			st.State_ = SOffline;
		return st;
	}

	QXmppPresence StatusToPresence (State state, const QString& text, int prio)
	{
		QXmppPresence::Type presType = state == SOffline ?
				QXmppPresence::Unavailable :
				QXmppPresence::Available;

		QXmppPresence pres (presType);
		if (state != SOffline && state <= SInvisible)
			pres.setAvailableStatusType (static_cast<QXmppPresence::AvailableStatusType> (state - 1));
		pres.setStatusText (text);
		pres.setPriority (prio);

		return pres;
	}

	QString GetBareJID (const QString& entryId, GlooxAccount *acc)
	{
		const auto underscoreCount = acc->GetAccountID ().count ('_') + 1;
		return entryId.section ('_', underscoreCount);
	}

}
}
}
}
