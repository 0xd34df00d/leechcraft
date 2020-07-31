/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "privacylistsitemdialog.h"
#include "xeps/privacylistsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	PrivacyListsItemDialog::PrivacyListsItemDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	PrivacyListItem PrivacyListsItemDialog::GetItem () const
	{
		PrivacyListItem result;

		if (Ui_.Type_->currentIndex () == TNSubscription)
			result.SetValue (Ui_.Value_->itemData (Ui_.Value_->currentIndex ()).toString ());
		else
			result.SetValue (Ui_.Value_->currentText ());

		result.SetAction (Ui_.Action_->currentIndex () == ANAllow ?
					PrivacyListItem::Action::Allow :
					PrivacyListItem::Action::Deny);

		switch (Ui_.Type_->currentIndex ())
		{
		case TNJID:
			result.SetType (PrivacyListItem::Type::Jid);
			break;
		case TNGroup:
			result.SetType (PrivacyListItem::Type::Group);
			break;
		case TNSubscription:
			result.SetType (PrivacyListItem::Type::Subscription);
			break;
		}

		PrivacyListItem::StanzaTypes stanzas = PrivacyListItem::STNone;
		if (Ui_.StanzaMessage_->checkState () == Qt::Checked)
			stanzas |= PrivacyListItem::STMessage;
		if (Ui_.StanzaInPres_->checkState () == Qt::Checked)
			stanzas |= PrivacyListItem::STPresenceIn;
		if (Ui_.StanzaOutPres_->checkState () == Qt::Checked)
			stanzas |= PrivacyListItem::STPresenceOut;
		if (Ui_.StanzaIQ_->checkState () == Qt::Checked)
			stanzas |= PrivacyListItem::STIq;

		if (stanzas == PrivacyListItem::STNone)
			stanzas = PrivacyListItem::STAll;

		result.SetStanzaTypes (stanzas);

		return result;
	}

	void PrivacyListsItemDialog::SetItem (const PrivacyListItem& item)
	{
		Ui_.Action_->setCurrentIndex (item.GetAction () == PrivacyListItem::Action::Allow ?
					ANAllow :
					ANDeny);

		TypeNum index = TNJID;
		switch (item.GetType ())
		{
		case PrivacyListItem::Type::Jid:
			index = TNJID;
			break;
		case PrivacyListItem::Type::Group:
			index = TNGroup;
			break;
		case PrivacyListItem::Type::Subscription:
			index = TNSubscription;
			break;
		case PrivacyListItem::Type::None:
			break;
		}
		Ui_.Type_->setCurrentIndex (index);
		on_Type__currentIndexChanged (index);

		if (index == TNSubscription)
		{
			const int idx = Ui_.Value_->findData (item.GetValue ());
			if (idx >= 0)
				Ui_.Value_->setCurrentIndex (idx);
		}
		else
			Ui_.Value_->setEditText (item.GetValue ());

		const PrivacyListItem::StanzaTypes stanzas = item.GetStanzaTypes ();
		if (stanzas != PrivacyListItem::STAll)
		{
			if (stanzas & PrivacyListItem::STMessage)
				Ui_.StanzaMessage_->setCheckState (Qt::Checked);
			if (stanzas & PrivacyListItem::STIq)
				Ui_.StanzaIQ_->setCheckState (Qt::Checked);
			if (stanzas & PrivacyListItem::STPresenceIn)
				Ui_.StanzaInPres_->setCheckState (Qt::Checked);
			if (stanzas & PrivacyListItem::STPresenceOut)
				Ui_.StanzaOutPres_->setCheckState (Qt::Checked);
		}
	}

	void PrivacyListsItemDialog::on_Type__currentIndexChanged (int type)
	{
		Ui_.Value_->clear ();
		if (type == TNSubscription)
		{
			Ui_.Value_->setEditable (false);

			Ui_.Value_->addItem (tr ("Both"), "both");
			Ui_.Value_->addItem (tr ("To"), "to");
			Ui_.Value_->addItem (tr ("From"), "from");
			Ui_.Value_->addItem (tr ("None"), "none");
		}
		else
			Ui_.Value_->setEditable (true);
	}
}
}
}
