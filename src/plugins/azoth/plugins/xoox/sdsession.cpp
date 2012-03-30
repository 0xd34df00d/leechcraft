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

#include "sdsession.h"
#include <QStandardItemModel>
#include <QDomElement>
#include <QtDebug>
#include <QXmppDiscoveryIq.h>
#include "glooxaccount.h"
#include "clientconnection.h"
#include "sdmodel.h"
#include "capsmanager.h"
#include "vcarddialog.h"
#include "formbuilder.h"
#include "util.h"
#include "executecommanddialog.h"
#include "xep0232handler.h"

namespace LeechCraft
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
			Q_FOREACH (const QString& string, strings)
			{
				QStandardItem *item = new QStandardItem (string);
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
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("JID") << tr ("Node"));

		auto items = AppendRow (Model_,
				QStringList (query) << query << "",
				query,
				"");
		JID2Node2Item_ [query] [""] = items.at (0);

		items.at (0)->setData (true, DRFetchedMore);

		QPointer<SDSession> ptr (this);
		Account_->GetClientConnection ()->RequestInfo (query,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleInfo (iq); });
		Account_->GetClientConnection ()->RequestItems (query,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleItems (iq); });
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
		QList<QPair<QByteArray, QString>> result;
		if (!index.isValid ())
			return result;

		const QModelIndex& sibling = index.sibling (index.row (), CName);
		QStandardItem *item = Model_->itemFromIndex (sibling);
		const ItemInfo& info = Item2Info_ [item];

		auto idHasCat = [&info] (const QString& name)
		{
			return std::find_if (info.Identities_.begin (), info.Identities_.end (),
					[&name] (decltype (*info.Identities_.begin ()) id) { return id.category () == name; }) != info.Identities_.end ();
		};

		if (info.Caps_.contains ("vcard-temp") &&
				!info.JID_.isEmpty ())
			result << QPair<QByteArray, QString> ("view-vcard", tr ("View VCard..."));
		if (!info.JID_.isEmpty ())
			result << QPair<QByteArray, QString> ("add-to-roster", tr ("Add to roster..."));
		if (info.Caps_.contains ("jabber:iq:register"))
			result << QPair<QByteArray, QString> ("register", tr ("Register..."));
		if (info.Caps_.contains ("http://jabber.org/protocol/commands"))
		{
			bool found = false;
			Q_FOREACH (const auto& id, info.Identities_)
				if (id.category () == "automation" &&
						id.type () == "command-node")
				{
					found = true;
					break;
				}

			if (found)
				result << QPair<QByteArray, QString> ("execute-ad-hoc", tr ("Execute..."));
		}
		if (idHasCat ("conference"))
			result << QPair<QByteArray, QString> ("join-conference", tr ("Join..."));

		result << QPair<QByteArray, QString> ("refresh", tr ("Refresh..."));

		return result;
	}

	void SDSession::ExecuteAction (const QModelIndex& index, const QByteArray& id)
	{
		if (!index.isValid ())
			return;

		if (id == "refresh")
		{
			const QModelIndex& sibling = index.sibling (index.row (), CName);
			QStandardItem *item = Model_->itemFromIndex (sibling);
			if (item->rowCount ())
				item->removeRows (0, item->rowCount ());
			item->setData (false, SDSession::DRFetchedMore);
			Model_->fetchMore (sibling);
			return;
		}

		const QModelIndex& sibling = index.sibling (index.row (), CName);
		QStandardItem *item = Model_->itemFromIndex (sibling);
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
		struct Appender
		{
			QStringList Strings_;

			Appender ()
			{
			}

			Appender& operator() (const QString& text, const QString& name)
			{
				if (!text.isEmpty ())
					Strings_ << name + ' ' + text;

				return *this;
			}

			QString operator() () const
			{
				return Strings_.join ("<br/>");
			}
		};

		QString GetMUCDescr (const QXmppDataForm& form)
		{
			QString result;
			Q_FOREACH (const QXmppDataForm::Field& field, form.fields ())
				if (field.key () == "FORM_TYPE" && field.value () != "http://jabber.org/protocol/muc#roominfo")
					return QString ();
				else if (field.key () == "muc#roominfo_description")
				{
					result = field.value ().toString ();
					break;
				}

			return result;
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

		if (iq.identities ().size () == 1)
		{
			const QXmppDiscoveryIq::Identity& id = iq.identities ().at (0);
			const QString& text = id.name ();
			if (!text.isEmpty ())
				targetItem->setText (text);
		}

		QString tooltip = Qt::escape (targetItem->text ()) + "<br />";

		const QString& mucDescr = GetMUCDescr (iq.form ());
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
		Q_FOREACH (const auto& id, iq.identities ())
		{
			if (id.name ().isEmpty ())
				continue;

			tooltip += "<li>";
			tooltip += Appender ()
					(id.name (), tr ("Identity name:"))
					(id.category (), tr ("Category:"))
					(id.type (), tr ("Type:"))
					(id.language (), tr ("Language:"))
					();
			tooltip += "</li>";
		}
		tooltip += "</ul>";

		const QStringList& caps = Account_->GetClientConnection ()->
				GetCapsManager ()->GetCaps (iq.features ());
		if (!caps.isEmpty ())
		{
			tooltip += "<strong>" + tr ("Capabilities:");
			tooltip += "</strong><ul><li>";
			tooltip += caps.join ("</li><li>");
			tooltip += "</li></ul>";
		}

		targetItem->setToolTip (tooltip);

		ItemInfo info =
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
		QStandardItem *parentItem = JID2Node2Item_ [iq.from ()] [iq.queryNode ()];
		if (!parentItem)
		{
			qWarning () << Q_FUNC_INFO
					<< "no parent node for"
					<< iq.from ();
			return;
		}

		QPointer<SDSession> ptr (this);
		Q_FOREACH (const auto& item, iq.items ())
		{
			auto items = AppendRow (parentItem,
					QStringList (item.name ()) << item.jid () << item.node (),
					item.jid (),
					item.node ());
			JID2Node2Item_ [item.jid ()] [item.node ()] = items.at (0);

			Account_->GetClientConnection ()->RequestInfo (item.jid (),
					[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleInfo (iq); },
					item.node ());
		}
	}

	void SDSession::QueryItem (QStandardItem *item)
	{
		item->setData (true, DRFetchedMore);

		QPointer<SDSession> ptr (this);
		const QString& jid = item->data (DRJID).toString ();
		const QString& node = item->data (DRNode).toString ();
		Account_->GetClientConnection ()->RequestItems (jid,
				[ptr] (const QXmppDiscoveryIq& iq) { if (ptr) ptr->HandleItems (iq); },
				node);
	}

	void SDSession::ViewVCard (const SDSession::ItemInfo& info)
	{
		const QString& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		QPointer<VCardDialog> dia (new VCardDialog (Account_));
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		Account_->GetClientConnection ()->FetchVCard (jid,
				[dia] (const QXmppVCardIq& iq) { if (dia) dia->UpdateInfo (iq); });
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
		elem.setAttribute ("xmlns", "jabber:iq:register");
		iq.setExtensions (QXmppElementList (elem));

		Account_->GetClientConnection ()->SendPacketWCallback (iq, this, "handleRegistrationForm");
	}

	void SDSession::ExecuteAdHoc (const SDSession::ItemInfo& info)
	{
		const QString& jid = info.JID_;
		if (jid.isEmpty ())
			return;

		ExecuteCommandDialog *dia = new ExecuteCommandDialog (jid, info.Node_, Account_);
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

		Account_->JoinRoom (jid, Account_->GetNick ());
	}

	void SDSession::handleRegistrationForm (const QXmppIq& iq)
	{
		QXmppDataForm form;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
		{
			if (elem.tagName () != "query" ||
					elem.attribute ("xmlns") != "jabber:iq:register")
				continue;

			QXmppElement x = elem.firstChildElement ("x");

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

		FormBuilder builder;
		QWidget *widget = builder.CreateForm (form);
		if (!XooxUtil::RunFormDialog (widget))
			return;

		form = builder.GetForm ();
		form.setType (QXmppDataForm::Submit);

		QXmppIq regIq;
		regIq.setType (QXmppIq::Set);
		regIq.setTo (iq.from ());
		QXmppElement elem;
		elem.setTagName ("query");
		elem.setAttribute ("xmlns", "jabber:iq:register");
		elem.appendChild (XooxUtil::Form2XmppElem (form));

		regIq.setExtensions (QXmppElementList (elem));

		Account_->GetClientConnection ()->GetClient ()->sendPacket (regIq);
	}
}
}
}
