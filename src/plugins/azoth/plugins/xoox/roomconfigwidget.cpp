/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "roomconfigwidget.h"
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QXmppMucManager.h>
#include "roomclentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"
#include "formbuilder.h"
#include "affiliationselectordialog.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	RoomConfigWidget::RoomConfigWidget (RoomCLEntry *room, QWidget *widget)
	: QWidget (widget)
	, FB_ (new FormBuilder)
	, Room_ (room)
	, JID_ (room->GetRoomHandler ()->GetRoomJID ())
	, RoomHandler_ (room->GetParentAccount ()->GetClientConnection ()->GetMUCManager ()->addRoom (JID_))
	, PermsModel_ (new QStandardItemModel (this))
	, Aff2Cat_ (InitModel ())
	{
		Ui_.setupUi (this);
		Ui_.PermsTree_->setModel (PermsModel_);

		connect (RoomHandler_,
				SIGNAL (configurationReceived (const QXmppDataForm&)),
				this,
				SLOT (handleConfigurationReceived (const QXmppDataForm&)));
		connect (RoomHandler_,
				SIGNAL (permissionsReceived (const QList<QXmppMucItem>&)),
				this,
				SLOT (handlePermsReceived (const QList<QXmppMucItem>&)));
		RoomHandler_->requestConfiguration ();
		RoomHandler_->requestPermissions ();
	}

	QMap<QXmppMucItem::Affiliation, QStandardItem*> RoomConfigWidget::InitModel () const
	{
		PermsModel_->clear ();
		PermsModel_->setHorizontalHeaderLabels ({ ("JID"), tr ("Reason") });
		QMap<QXmppMucItem::Affiliation, QStandardItem*> aff2cat;
		aff2cat [QXmppMucItem::OutcastAffiliation] = new QStandardItem (tr ("Banned"));
		aff2cat [QXmppMucItem::MemberAffiliation] = new QStandardItem (tr ("Members"));
		aff2cat [QXmppMucItem::AdminAffiliation] = new QStandardItem (tr ("Admins"));
		aff2cat [QXmppMucItem::OwnerAffiliation] = new QStandardItem (tr ("Owners"));
		for (auto item : aff2cat)
		{
			QList<QStandardItem*> rootItems;
			rootItems << item;
			rootItems << new QStandardItem (tr ("Reason"));
			for (auto t : rootItems)
				t->setEditable (false);
			PermsModel_->appendRow (rootItems);
		}
		return aff2cat;
	}

	void RoomConfigWidget::SendItem (const QXmppMucItem& item)
	{
		QXmppMucAdminIq iq;
		iq.setTo (JID_);
		iq.setType (QXmppIq::Set);
		iq.setItems ({ item });

		Room_->GetParentAccount ()->GetClientConnection ()->GetClient ()->sendPacket (iq);
	}

	QStandardItem* RoomConfigWidget::GetCurrentItem () const
	{
		const QModelIndex& index = Ui_.PermsTree_->currentIndex ();
		if (!index.isValid ())
			return 0;

		const QModelIndex& sibling = index.sibling (index.row (), 0);
		return PermsModel_->itemFromIndex (sibling);
	}

	void RoomConfigWidget::accept ()
	{
		auto form = FB_->GetForm ();
		form.setType (QXmppDataForm::Submit);
		RoomHandler_->setConfiguration (form);
	}

	void RoomConfigWidget::on_AddPerm__released ()
	{
		AffiliationSelectorDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& jid = dia.GetJID ();
		if (jid.isEmpty ())
			return;

		QXmppMucItem item;
		item.setJid (jid);
		item.setAffiliation (dia.GetAffiliation ());
		SendItem (item);

		handlePermsReceived ({ item });
	}

	void RoomConfigWidget::on_ModifyPerm__released ()
	{
		const auto stdItem = GetCurrentItem ();
		if (!stdItem)
			return;

		const auto parent = stdItem->parent ();

		const auto aff = Aff2Cat_.key (parent);
		if (aff == QXmppMucItem::UnspecifiedAffiliation)
		{
			qWarning () << Q_FUNC_INFO
					<< "bad parent"
					<< parent
					<< "for"
					<< stdItem;
			return;
		}

		const QString& jid = stdItem->text ();

		AffiliationSelectorDialog dia (this);
		dia.SetJID (jid);
		dia.SetAffiliation (aff);
		dia.SetReason (stdItem->data (ItemRoles::Reason).toString ());
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& newJid = dia.GetJID ();
		if (newJid.isEmpty ())
			return;

		parent->removeRow (stdItem->row ());

		QXmppMucItem item;
		item.setJid (newJid);
		item.setAffiliation (dia.GetAffiliation ());
		item.setReason (dia.GetReason ());
		SendItem (item);

		if (item.affiliation () != QXmppMucItem::NoAffiliation)
			handlePermsReceived ({ item });
	}

	void RoomConfigWidget::on_RemovePerm__released ()
	{
		QStandardItem *stdItem = GetCurrentItem ();
		if (!stdItem)
			return;

		const QString& jid = stdItem->text ();
		if (jid.isEmpty ())
			return;

		QStandardItem *parent = stdItem->parent ();
		if (!parent)
			return;

		parent->removeRow (stdItem->row ());

		QXmppMucItem item;
		item.setJid (jid);
		item.setAffiliation (QXmppMucItem::NoAffiliation);
		SendItem (item);
	}

	void RoomConfigWidget::handleConfigurationReceived (const QXmppDataForm& form)
	{
		if (sender () != RoomHandler_)
			return;

		FB_->Clear ();

		FormWidget_ = FB_->CreateForm (form);
		Ui_.ScrollArea_->setWidget (FormWidget_);
		emit dataReady ();
	}

	void RoomConfigWidget::handlePermsReceived (const QList<QXmppMucItem>& perms)
	{
		if (qobject_cast<QXmppMucRoom*> (sender ()) &&
				sender () != RoomHandler_)
			return;

		for (const auto& perm : perms)
		{
			auto parentItem = Aff2Cat_ [perm.affiliation ()];
			if (!parentItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "no parent item for"
						<< perm.affiliation ();
				continue;
			}

			auto firstItem = new QStandardItem (perm.jid ());
			firstItem->setData (perm.reason (), ItemRoles::Reason);

			QList<QStandardItem*> items
			{
				firstItem,
				new QStandardItem (perm.reason ())
			};
			for (auto item : items)
				item->setEditable (false);
			parentItem->appendRow (items);
		}

		for (auto item : Aff2Cat_.values ())
		{
			item->sortChildren (0);
			Ui_.PermsTree_->expand (item->index ());
		}
	}
}
}
}
