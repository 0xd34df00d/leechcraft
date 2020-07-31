/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addaccountwizardfirstpage.h"
#include <interfaces/azoth/icanhavesslerrors.h>
#include "core.h"
#include "sslerrorshandler.h"

namespace LC
{
namespace Azoth
{
	AddAccountWizardFirstPage::AddAccountWizardFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.ProtoBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (readdWidgets ()));
		connect (Ui_.RegisterAccount_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (readdWidgets ()));
	}
	
	void AddAccountWizardFirstPage::initializePage ()
	{
		registerField ("AccountName*", Ui_.NameEdit_);
		registerField ("AccountProto", Ui_.ProtoBox_);
		registerField ("RegisterNewAccount", Ui_.RegisterAccount_);

		for (const auto proto : Core::Instance ().GetProtocols ())
		{
			if (proto->GetFeatures () & IProtocol::PFNoAccountRegistration)
				continue;

			Ui_.ProtoBox_->addItem (proto->GetProtocolIcon (),
					proto->GetProtocolName (),
					QVariant::fromValue<QObject*> (proto->GetQObject ()));
		}
			
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}

	void AddAccountWizardFirstPage::CleanupWidgets ()
	{
		const int currentId = wizard ()->currentId ();
		for (const int id : wizard ()->pageIds ())
			if (id > currentId)
				wizard ()->removePage (id);
		qDeleteAll (Widgets_);
		Widgets_.clear ();
	}

	void AddAccountWizardFirstPage::readdWidgets ()
	{
		const int idx = Ui_.ProtoBox_->currentIndex ();
		if (idx == -1)
			return;

		const auto obj = Ui_.ProtoBox_->itemData (idx).value<QObject*> ();
		const auto proto = qobject_cast<IProtocol*> (obj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IProtocol";
			return;
		}
		
		Ui_.RegisterAccount_->setEnabled (proto->GetFeatures () & IProtocol::PFSupportsInBandRegistration);

		CleanupWidgets ();

		IProtocol::AccountAddOptions options = IProtocol::AAONoOptions;
		if (Ui_.RegisterAccount_->isChecked ())
			options |= IProtocol::AAORegisterNewAccount;
		Widgets_ = proto->GetAccountRegistrationWidgets (options);
		if (Widgets_.isEmpty ())
		{
			setFinalPage (true);
			return;
		}
		
		const auto& protoName = proto->GetProtocolName ();
		for (const auto widget : Widgets_)
		{
			auto page = qobject_cast<QWizardPage*> (widget);
			if (!page)
			{
				page = new QWizardPage (wizard ());
				page->setTitle (tr ("%1 options")
						.arg (protoName));
				page->setLayout (new QVBoxLayout ());
				page->layout ()->addWidget (widget);
			}
			wizard ()->addPage (page);

			if (const auto ichse = qobject_cast<ICanHaveSslErrors*> (widget))
				new SslErrorsHandler { SslErrorsHandler::AccountRegistration {}, ichse };
		}
		setFinalPage (false);
	}
	
	void AddAccountWizardFirstPage::handleAccepted ()
	{
		const auto obj = Ui_.ProtoBox_->itemData (field ("AccountProto").toInt ()).value<QObject*> ();
		const auto proto = qobject_cast<IProtocol*> (obj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IProtocol";
			return;
		}
		
		proto->RegisterAccount (Ui_.NameEdit_->text (), Widgets_);
	}
}
}
