/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "privacylistsitemdialog.h"
#include "privacylistsmanager.h"

namespace LeechCraft
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

		result.SetValue (Ui_.Value_->currentText ());
		result.SetAction (Ui_.Action_->currentIndex () == ANAllow ?
					PrivacyListItem::AAllow :
					PrivacyListItem::ADeny);

		switch (Ui_.Type_->currentIndex ())
		{
		case TNJID:
			result.SetType (PrivacyListItem::TJid);
			break;
		case TNGroup:
			result.SetType (PrivacyListItem::TGroup);
			break;
		case TNSubscription:
			result.SetType (PrivacyListItem::TSubscription);
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
		Ui_.Action_->setCurrentIndex (item.GetAction () == PrivacyListItem::AAllow ?
					ANAllow :
					ANDeny);

		TypeNum index = TNJID;
		switch (item.GetType ())
		{
		case PrivacyListItem::TJid:
			index = TNJID;
			break;
		case PrivacyListItem::TGroup:
			index = TNGroup;
			break;
		case PrivacyListItem::TSubscription:
			index = TNSubscription;
			break;
		case PrivacyListItem::TNone:
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
