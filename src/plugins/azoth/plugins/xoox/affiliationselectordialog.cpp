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

#include "affiliationselectordialog.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AffiliationSelectorDialog::AffiliationSelectorDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}
	
	QString AffiliationSelectorDialog::GetJID () const
	{
		return Ui_.JIDEdit_->text ();
	}
	
	void AffiliationSelectorDialog::SetJID (const QString& jid)
	{
		Ui_.JIDEdit_->setText (jid);
	}
	
	QXmppMucAdminIq::Item::Affiliation AffiliationSelectorDialog::GetAffiliation () const
	{
		if (Ui_.Owner_->isChecked ())
			return QXmppMucAdminIq::Item::OwnerAffiliation;
		else if (Ui_.Admin_->isChecked ())
			return QXmppMucAdminIq::Item::AdminAffiliation;
		else if (Ui_.Member_->isChecked ())
			return QXmppMucAdminIq::Item::MemberAffiliation;
		else if (Ui_.Banned_->isChecked ())
			return QXmppMucAdminIq::Item::OutcastAffiliation;
		else
			return QXmppMucAdminIq::Item::NoAffiliation;
	}
	
	void AffiliationSelectorDialog::SetAffiliation (QXmppMucAdminIq::Item::Affiliation aff)
	{
		switch (aff)
		{
		case QXmppMucAdminIq::Item::OwnerAffiliation:
			Ui_.Owner_->setChecked (true);
			break;
		case QXmppMucAdminIq::Item::AdminAffiliation:
			Ui_.Admin_->setChecked (true);
			break;
		case QXmppMucAdminIq::Item::MemberAffiliation:
			Ui_.Admin_->setChecked (true);
			break;
		case QXmppMucAdminIq::Item::NoAffiliation:
			Ui_.None_->setChecked (true);
			break;
		case QXmppMucAdminIq::Item::OutcastAffiliation:
			Ui_.Banned_->setChecked (true);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown affiliation"
					<< aff;
			break;
		}
	}
}
}
}
