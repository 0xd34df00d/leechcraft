/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sdsession.h"
#include <QStandardItemModel>
#include <QDomElement>
#include <QTimer>
#include <QtDebug>
#include <QXmppDiscoveryIq.h>
#include "glooxaccount.h"
#include "clientconnection.h"
#include "clientconnectionextensionsmanager.h"
#include "sdmodel.h"
#include "capsmanager.h"
#include "vcarddialog.h"
#include "formbuilder.h"
#include "util.h"
#include "executecommanddialog.h"
#include "xep0232handler.h"
#include "discomanagerwrapper.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	SDSession::SDSession (GlooxAccount *account)
	: Model_ (new SDModel (this))
	, Account_ (account)
	{
		ID2Action_ ["view-vcard"] = [this] (const ItemInfo& ii) { ViewVCard (ii); };
		ID2Action_ ["add-to-roster"] = [this] (const ItemInfo& ii) { AddToRoster (ii); };
		ID2Action_ ["register"] = [this] (const ItemInfo& ii) { Register (ii); };
		ID2Action_ ["execute-ad-hoc"] = [this] (const ItemInfo& ii) { ExecuteAdHoc (ii); };
		ID2Action_ ["join-conference"] = [this] (const ItemInfo& ii) { JoinConference (ii); };
	}

	namespace
	{
		template<typename T>
		QList<QStandardItem*> AppendRow (T *parent,
				const QStringList& strings,
				const QString& jid,
				const QString& node)
		{
			QList<QStandardItem*> items;
			for (const auto& string : strings)
			{
				auto item = new QStandardItem (string);
				items << item;
				item->setEditable (false);
			}
			items.at (0)->setData (jid, SDSession::DRJID);
			items.at (0)->setData (node, SDSession::DRNode);
			parent->appendRow (items);
			return items;
		}
	}

	void SDSession::SetQuery (const QString& query)
	{
		Query_ = query;

		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("JID"), tr ("Node") });

		auto items = AppendRow (Model_,
				{ query, query, "" },
				query,
				"");
		JID2Node2Item_ [query] [""] = items.at (0);

		items.at (0)->setData (true, DRFetchedMore);

		QPointer<SDSession> ptr (this);
		const auto discoWrapper = Account_->GetClientConnection ()->GetDiscoManagerWrapper ();
		discoWrapper->RequestInfo (query,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleInfo (iq); },
				true);
		discoWrapper->RequestItems (query,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleItems (iq); },
				true);
	}

	QString SDSession::GetQuery () const
	{
		return Query_;
	}

	QAbstractItemModel* SDSession::GetRepresentationModel () const
	{
		return Model_;
	}

	QList<QPair<QByteArray, QString>> SDSession::GetActionsFor (const QModelIndex& index)
	{
		if (!index.isValid ())
			return {};

		const auto& sibling = index.sibling (index.row (), CName);
		const auto item = Model_->itemFromIndex (sibling);
		const auto& info = Item2Info_.value (item);

		auto idHasCat = [&info] (const QString& name)
		{
			return std::any_of (info.Identities_.begin (), info.Identities_.end (),
					[&name] (const auto& id) { return id.category () == name; });
		};

		QList<QPair<QByteArray, QString>> result;
		if (info.Caps_.contains ("vcard-temp") &&
				!info.JID_.isEmpty ())
			result.append ({ "view-vcard", tr ("View VCard...") });
		if (!info.JID_.isEmpty ())
			result.append ({ "add-to-roster", tr ("Add to roster...") });
		if (info.Caps_.contains (XooxUtil::NsRegister))
			result.append ({ "register", tr ("Register...") });
		if (info.Caps_.contains ("http://jabber.org/protocol/commands"))
			result.append ({ "execute-ad-hoc", tr ("Execute...") });
		if (idHasCat ("conference"))
			result.append ({ "join-conference", tr ("Join...") });

		result.append ({ "refresh", tr ("Refresh...") });

		return result;
	}

	void SDSession::ExecuteAction (const QModelIndex& index, const QByteArray& id)
	{
		if (!index.isValid ())
			return;

		if (id == "refresh")
		{
			const auto& sibling = index.sibling (index.row (), CName);
			const auto item = Model_->itemFromIndex (sibling);
			if (item->rowCount ())
				item->removeRows (0, item->rowCount ());
			item->setData (false, SDSession::DRFetchedMore);
			Model_->fetchMore (sibling);
			return;
		}

		const auto& sibling = index.sibling (index.row (), CName);
		const auto item = Model_->itemFromIndex (sibling);
		const ItemInfo& info = Item2Info_ [item];

		if (!ID2Action_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown ID"
					<< id;
			return;
		}

		ID2Action_ [id] (info);
	}

	namespace
	{
		QString GetMUCDescr (const QXmppDataForm& form)
		{
			for (const auto& field : form.fields ())
				if (field.key () == "FORM_TYPE" && field.value () != "http://jabber.org/protocol/muc#roominfo")
					return {};
				else if (field.key () == "muc#roominfo_description")
					return field.value ().toString ();

			return {};
		}
	}

	void SDSession::HandleInfo (const QXmppDiscoveryIq& iq)
	{
		QStandardItem *item = JID2Node2Item_ [iq.from ()] [iq.queryNode ()];
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no parent node for"
					<< iq.from ();
			return;
		}

		const QModelIndex& index = item->index ();
		const QModelIndex& sibling = index.sibling (index.row (), CName);
		QStandardItem *targetItem = Model_->itemFromIndex (sibling);

		const auto& identities = iq.identities ();
		if (identities.size () == 1)
		{
			const QXmppDiscoveryIq::Identity& id = identities.at (0);
			const QString& text = id.name ();
			if (!text.isEmpty ())
				targetItem->setText (text);
		}

		auto normalize = [] (QString& text)
		{
			text.replace ("\n", "<br />")
					.remove ("\r")
					.replace ("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
		};

		auto tooltip = targetItem->text ().toHtmlEscaped () + "<br />";
		normalize (tooltip);

		auto mucDescr = GetMUCDescr (iq.form ());
		normalize (mucDescr);
		if (!mucDescr.isEmpty ())
		{
			tooltip += tr ("MUC description: %1.")
					.arg (mucDescr);
			tooltip += "<br />";
		}

		const auto& verStruct = XEP0232Handler::FromDataForm (iq.form ());
		if (!verStruct.IsNull ())
		{
			QStringList verInfos;
			auto append = [&verInfos] (const QString& tr, const QString& val)
			{
				if (!val.isEmpty ())
					verInfos << tr.arg (val);
			};
			append (tr ("OS: %1."), verStruct.OS_);
			append (tr ("OS version: %1."), verStruct.OSVer_);
			append (tr ("Software: %1."), verStruct.Software_);
			append (tr ("Software version: %1."), verStruct.SoftwareVer_);

			tooltip = "<strong>" + tr ("Version:") + "</strong><ul><li>";
			tooltip += verInfos.join ("</li><li>");
			tooltip += "</li></ul>";
		}

		tooltip += "<strong>" + tr ("Identities:") + "</strong><ul>";
		for (const auto& id : identities)
		{
			if (id.name ().isEmpty ())
				continue;

			QStringList identityDescr;
			auto append = [&identityDescr] (const QString& text, const QString& name)
			{
				if (!text.isEmpty ())
					identityDescr << name + ' ' + text;
			};
			append (id.name (), tr ("Identity name:"));
			append (id.category (), tr ("Category:"));
			append (id.type (), tr ("Type:"));
			append (id.language (), tr ("Language:"));

			tooltip += "<li>" + identityDescr.join ("<br/>") + "</li>";
		}
		tooltip += "</ul>";

		const auto& caps = Account_->GetClientConnection ()->
				GetCapsManager ()->GetCaps (iq.features ());
		if (!caps.isEmpty ())
		{
			tooltip += "<strong>" + tr ("Capabilities:");
			tooltip += "</strong><ul><li>";
			tooltip += caps.join ("</li><li>");
			tooltip += "</li></ul>";
		}

		targetItem->setToolTip (tooltip);

		ItemInfo info
		{
			iq.features (),
			iq.identities (),
			iq.from (),
			iq.queryNode ()
		};
		Item2Info_ [targetItem] = info;
	}

	void SDSession::HandleItems (const QXmppDiscoveryIq& iq)
	{
		auto parentItem = JID2Node2Item_ [iq.from ()] [iq.queryNode ()];
		if (!parentItem)
		{
			qWarning () << Q_FUNC_INFO
					<< "no parent node for"
					<< iq.from ();
			return;
		}

		QPointer<SDSession> ptr (this);
		const auto& items = iq.items ();
		for (const auto& item : items)
		{
			auto items = AppendRow (parentItem,
					{ item.name (), item.jid (), item.node () },
					item.jid (),
					item.node ());
			JID2Node2Item_ [item.jid ()] [item.node ()] = items.at (0);
		}

		[=] (this auto self, int start)
		{
			if (!ptr ||
					start >= items.size ())
				return;

			const auto batchSize = 300;

			// TODO remove the cast after done migrating to Qt 6
			for (int end = std::min (start + batchSize, static_cast<int> (items.size ())); start < end; ++start)
			{
				const auto& item = items.at (start);
				ptr->Account_->GetClientConnection ()->GetDiscoManagerWrapper ()->RequestInfo (item.jid (),
						[ptr] (const QXmppDiscoveryIq& infoIq)
						{
							if (ptr)
								ptr->HandleInfo (infoIq);
						},
						true,
						item.node ());
			}

			QTimer::singleShot (2000, [=] { self (start); });
		} (0);
	}

	void SDSession::QueryItem (QStandardItem *item)
	{
		item->setData (true, DRFetchedMore);

		QPointer<SDSession> ptr (this);
		const auto& jid = item->data (DRJID).toString ();
		const auto& node = item->data (DRNode).toString ();
		Account_->GetClientConnection ()->GetDiscoManagerWrapper ()->RequestItems (jid,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleItems (iq); },
				true,
				node);
	}

	void SDSession::ViewVCard (const SDSession::ItemInfo& info)
	{
		const auto& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		QPointer<VCardDialog> dia { new VCardDialog { Account_ } };
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		Account_->GetClientConnection ()->FetchVCard (jid,
				[dia] (const QXmppVCardIq& iq) { if (dia) dia->UpdateInfo (iq); },
				true);
	}

	void SDSession::AddToRoster (const SDSession::ItemInfo& info)
	{
		const QString& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		Account_->AddEntry (jid, QString (), QStringList ());
	}

	void SDSession::Register (const SDSession::ItemInfo& info)
	{
		const QString& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		QXmppIq iq;
		iq.setType (QXmppIq::Get);
		iq.setTo (jid);
		QXmppElement elem;
		elem.setTagName ("query");
		elem.setAttribute ("xmlns", XooxUtil::NsRegister);
		iq.setExtensions ({ elem });

		Account_->GetClientConnection ()->SendPacketWCallback (iq,
				[safeThis = QPointer { this }] (const QXmppIq& iq)
				{
					if (safeThis)
						safeThis->handleRegistrationForm (iq);
				});
	}

	void SDSession::ExecuteAdHoc (const SDSession::ItemInfo& info)
	{
		const auto& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		auto dia = info.Node_.isEmpty () ?
				new ExecuteCommandDialog { jid, Account_ } :
				new ExecuteCommandDialog { jid, info.Node_, Account_ };
		dia->show ();
		connect (dia,
				SIGNAL (finished (int)),
				dia,
				SLOT (deleteLater ()));
	}

	void SDSession::JoinConference (const SDSession::ItemInfo& info)
	{
		const QString& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		Account_->JoinRoom (jid, Account_->GetNick (), {});
	}

	void SDSession::handleRegistrationForm (const QXmppIq& iq)
	{
		QXmppDataForm form;
		for (const auto& elem : iq.extensions ())
		{
			if (elem.tagName () != "query" ||
					elem.attribute ("xmlns") != XooxUtil::NsRegister)
				continue;

			auto x = elem.firstChildElement ("x");

			// Ugly workaround for ejabberd.
			if (!x.attributeNames ().contains ("type"))
				x.setAttribute ("type", "form");

			form.parse (XooxUtil::XmppElem2DomElem (x));
			if (!form.isNull ())
				break;
		}

		if (form.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no form found, sorry";
			return;
		}

		FormBuilder builder { {}, &Account_->GetClientConnection ()->Exts ().Get<XMPPBobManager> () };
		const auto widget = builder.CreateForm (form);
		if (!XooxUtil::RunFormDialog (widget))
			return;

		form = builder.GetForm ();
		form.setType (QXmppDataForm::Submit);

		QXmppIq regIq;
		regIq.setType (QXmppIq::Set);
		regIq.setTo (iq.from ());
		QXmppElement elem;
		elem.setTagName ("query");
		elem.setAttribute ("xmlns", XooxUtil::NsRegister);
		elem.appendChild (XooxUtil::Form2XmppElem (form));

		regIq.setExtensions ({ elem });

		Account_->GetClientConnection ()->GetClient ()->sendPacket (regIq);
	}
}
}
}
