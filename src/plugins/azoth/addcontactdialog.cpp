/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addcontactdialog.h"
#include <QStringListModel>
#include <util/tags/tagscompleter.h>
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/iaccount.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	AddContactDialog::AddContactDialog (IAccount *focusAcc, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		const auto tc = new Util::TagsCompleter (Ui_.Groups_);
		tc->OverrideModel (new QStringListModel (Core::Instance ().GetChatGroups (), this));
		Ui_.Groups_->AddSelector ();

		for (const auto proto : Core::Instance ().GetProtocols ())
			Ui_.Protocol_->addItem (proto->GetProtocolName (), QVariant::fromValue<IProtocol*> (proto));

		if (focusAcc)
			FocusAccount (focusAcc);

		checkComplete ();
		connect (Ui_.ContactID_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkComplete ()));
	}

	void AddContactDialog::SetContactID (const QString& id)
	{
		Ui_.ContactID_->setText (id);
	}

	void AddContactDialog::SetNick (const QString& nick)
	{
		Ui_.Nick_->setText (nick);
	}

	IAccount* AddContactDialog::GetSelectedAccount () const
	{
		int idx = Ui_.Account_->currentIndex ();
		return idx >= 0 ?
			Ui_.Account_->itemData (idx).value<IAccount*> () :
			0;
	}

	QString AddContactDialog::GetContactID () const
	{
		return Ui_.ContactID_->text ();
	}

	QString AddContactDialog::GetNick () const
	{
		return Ui_.Nick_->text ();
	}

	QString AddContactDialog::GetReason () const
	{
		return Ui_.Reason_->toPlainText ();
	}

	QStringList AddContactDialog::GetGroups () const
	{
		QStringList result;
		for (const auto& str : Ui_.Groups_->text ().split (';'))
			result << str.trimmed ();
		return result;
	}

	void AddContactDialog::on_Protocol__currentIndexChanged (int idx)
	{
		Ui_.Account_->clear ();
		if (idx < 0)
			return;

		const auto proto = Ui_.Protocol_->itemData (idx).value<IProtocol*> ();
		for (const auto accObj : proto->GetRegisteredAccounts ())
		{
			const auto acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< accObj
						<< "to IAccount";
				continue;
			}

			if (!acc->IsShownInRoster ())
				continue;

			if (acc->GetState ().State_ == SOffline &&
					!(acc->GetAccountFeatures () & IAccount::FCanAddContactsInOffline))
				continue;

			Ui_.Account_->addItem (QString ("%1 (%2)")
						.arg (acc->GetAccountName ())
						.arg (acc->GetOurNick ()),
					QVariant::fromValue<IAccount*> (acc));
		}
	}

	void AddContactDialog::FocusAccount (IAccount *focusAcc)
	{
		QObject *protoObj = focusAcc->GetParentProtocol ();
		IProtocol *focusProto = qobject_cast<IProtocol*> (protoObj);
		if (!focusProto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< protoObj
					<< "to IProtocol";
			return;
		}

		for (int i = 0; i < Ui_.Protocol_->count (); ++i)
			if (Ui_.Protocol_->itemData (i).value<IProtocol*> () == focusProto)
			{
				Ui_.Protocol_->setCurrentIndex (i);
				break;
			}

		for (int i = 0; i < Ui_.Account_->count (); ++i)
			if (Ui_.Account_->itemData (i).value<IAccount*> () == focusAcc)
			{
				Ui_.Account_->setCurrentIndex (i);
				break;
			}
	}

	void AddContactDialog::checkComplete ()
	{
		const bool isComplete = GetSelectedAccount () &&
				!Ui_.ContactID_->text ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isComplete);
	}
}
}
