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
	
	QXmppMucItem::Affiliation AffiliationSelectorDialog::GetAffiliation () const
	{
		if (Ui_.Owner_->isChecked ())
			return QXmppMucItem::OwnerAffiliation;
		else if (Ui_.Admin_->isChecked ())
			return QXmppMucItem::AdminAffiliation;
		else if (Ui_.Member_->isChecked ())
			return QXmppMucItem::MemberAffiliation;
		else if (Ui_.Banned_->isChecked ())
			return QXmppMucItem::OutcastAffiliation;
		else
			return QXmppMucItem::NoAffiliation;
	}
	
	void AffiliationSelectorDialog::SetAffiliation (QXmppMucItem::Affiliation aff)
	{
		switch (aff)
		{
		case QXmppMucItem::OwnerAffiliation:
			Ui_.Owner_->setChecked (true);
			break;
		case QXmppMucItem::AdminAffiliation:
			Ui_.Admin_->setChecked (true);
			break;
		case QXmppMucItem::MemberAffiliation:
			Ui_.Member_->setChecked (true);
			break;
		case QXmppMucItem::NoAffiliation:
			Ui_.None_->setChecked (true);
			break;
		case QXmppMucItem::OutcastAffiliation:
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
