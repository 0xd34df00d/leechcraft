/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addaccountwizardfirstpage.h"
#include <QtDebug>
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iaccount.h"
#include "core.h"

namespace LC
{
namespace Blogique
{
	AddAccountWizardFirstPage::AddAccountWizardFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.BloggingPlatformBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (readdWidgets ()));

		connect (Ui_.RegisterAccount_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (readdWidgets ()));

		connect (Ui_.NameEdit_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (handleAccountNameChanged (const QString&)));
	}

	void AddAccountWizardFirstPage::initializePage ()
	{
		registerField ("AccountName*", Ui_.NameEdit_);
		registerField ("AccountBloggingPlatform", Ui_.BloggingPlatformBox_);
		registerField ("RegisterNewAccount", Ui_.RegisterAccount_);

		for (const auto platform : Core::Instance ().GetBloggingPlatforms ())
			Ui_.BloggingPlatformBox_->addItem (platform->GetBloggingPlatformIcon (),
					platform->GetBloggingPlatformName (),
					QVariant::fromValue<QObject*> (platform->GetQObject ()));

		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}

	bool AddAccountWizardFirstPage::isComplete () const
	{
		const auto& accs = Core::Instance ().GetAccounts ();
		const auto& name = Ui_.NameEdit_->text ();
		return std::none_of (accs.begin (), accs.end (),
				[&name] (const auto& acc) { return acc->GetAccountName () == name; });
	}

	void AddAccountWizardFirstPage::readdWidgets ()
	{
		const int idx = Ui_.BloggingPlatformBox_->currentIndex ();
		if (idx == -1)
			return;

		QObject *obj = Ui_.BloggingPlatformBox_->itemData (idx).value<QObject*> ();
		IBloggingPlatform *platform = qobject_cast<IBloggingPlatform*> (obj);
		if (!platform)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IBloggingPlatform";
			return;
		}

		Ui_.RegisterAccount_->setEnabled (platform->GetFeatures () &
				(IBloggingPlatform::BPFSupportsRegistration));

		const int currentId = wizard ()->currentId ();
		for (const int id : wizard ()->pageIds ())
			if (id > currentId)
				wizard ()->removePage (id);
		qDeleteAll (Widgets_);

		IBloggingPlatform::AccountAddOptions opts = IBloggingPlatform::AAONoOptions;
		if (Ui_.RegisterAccount_->isChecked ())
			opts |= IBloggingPlatform::AAORegisterNewAccount;

		Widgets_ = platform->GetAccountRegistrationWidgets (opts, Ui_.NameEdit_->text ());
		if (!Widgets_.size ())
			return;

		const auto& platformName = platform->GetBloggingPlatformName ();
		for (const auto widget : Widgets_)
		{
			QWizardPage *page = qobject_cast<QWizardPage*> (widget);
			if (!page)
			{
				page = new QWizardPage (wizard ());
				page->setTitle (tr ("%1 options")
						.arg (platformName));
				page->setLayout (new QVBoxLayout ());
				page->layout ()->addWidget (widget);
			}
			wizard ()->addPage (page);
		}

		setFinalPage (false);
	}

	void AddAccountWizardFirstPage::handleAccepted ()
	{
		QObject *obj = Ui_.BloggingPlatformBox_->
				itemData (field ("AccountBloggingPlatform")
						.toInt ()).value<QObject*> ();
		IBloggingPlatform *ibp = qobject_cast<IBloggingPlatform*> (obj);
		if (!ibp)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IBloggingPlatform";
			return;
		}

		ibp->RegisterAccount (Ui_.NameEdit_->text (), Widgets_);
	}

	void AddAccountWizardFirstPage::handleAccountNameChanged (const QString&)
	{
		emit completeChanged ();
		readdWidgets ();
	}
}
}
