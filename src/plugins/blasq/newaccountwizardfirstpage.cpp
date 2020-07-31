/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newaccountwizardfirstpage.h"
#include <interfaces/blasq/iservice.h>
#include "servicesmanager.h"
#include <QVBoxLayout>
#include <QtDebug>

namespace LC
{
namespace Blasq
{
	NewAccountWizardFirstPage::NewAccountWizardFirstPage (const ServicesManager *svcMgr,
			QWidget *parent)
	: QWizardPage (parent)
	, ServicesMgr_ (svcMgr)
	{
		Ui_.setupUi (this);
		connect (Ui_.Service_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (updatePages ()));
	}

	void NewAccountWizardFirstPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));

		for (auto service : ServicesMgr_->GetServices ())
			Ui_.Service_->addItem (service->GetServiceIcon (), service->GetServiceName (),
					QVariant::fromValue (service->GetQObject ()));

		updatePages ();
	}

	void NewAccountWizardFirstPage::updatePages ()
	{
		const int currentId = wizard ()->currentId ();
		for (const int id : wizard ()->pageIds ())
			if (id > currentId)
				wizard ()->removePage (id);

		Widgets_.clear ();
		Service_ = 0;

		const auto idx = Ui_.Service_->currentIndex ();
		if (idx < 0)
			return;

		const auto serviceObj = Ui_.Service_->itemData (idx).value<QObject*> ();
		Service_ = qobject_cast<IService*> (serviceObj);

		Widgets_ = Service_->GetAccountRegistrationWidgets ();
		for (auto w : Widgets_)
		{
			auto page = qobject_cast<QWizardPage*> (w);
			if (!page)
			{
				page = new QWizardPage ();
				page->setTitle (tr ("%1 options")
						.arg (Service_->GetServiceName ()));
				page->setLayout (new QVBoxLayout);
				page->layout ()->addWidget (w);
			}
			wizard ()->addPage (page);
		}
		setFinalPage (false);
	}

	void NewAccountWizardFirstPage::handleAccepted ()
	{
		if (!Service_)
			return;

		Service_->RegisterAccount (Ui_.AccName_->text (), Widgets_);
	}
}
}
