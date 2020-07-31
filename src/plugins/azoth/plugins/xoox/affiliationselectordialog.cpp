/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "affiliationselectordialog.h"
#include <QtDebug>

namespace LC
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

	QString AffiliationSelectorDialog::GetReason () const
	{
		return Ui_.ReasonEdit_->text ();
	}

	void AffiliationSelectorDialog::SetReason (const QString& reason)
	{
		Ui_.ReasonEdit_->setText (reason);
	}
}
}
}
