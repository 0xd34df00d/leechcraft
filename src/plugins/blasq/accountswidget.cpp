/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountswidget.h"
#include <QWizard>
#include <QtDebug>
#include "newaccountwizardfirstpage.h"
#include "accountsmanager.h"

namespace LC
{
namespace Blasq
{
	AccountsWidget::AccountsWidget (ServicesManager *svcMgr,
			AccountsManager *accMgr, QWidget *parent)
	: QWidget (parent)
	, ServicesMgr_ (svcMgr)
	, AccountsMgr_ (accMgr)
	{
		Ui_.setupUi (this);
		Ui_.View_->setModel (AccountsMgr_->GetModel ());
	}

	void AccountsWidget::on_Add__released ()
	{
		auto wizard = new QWizard (this);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle ("Add account");
		wizard->addPage (new NewAccountWizardFirstPage (ServicesMgr_));
		wizard->show ();
	}

	void AccountsWidget::on_Remove__released ()
	{
		const auto& idx = Ui_.View_->currentIndex ();
		if (!idx.isValid ())
			return;

		AccountsMgr_->RemoveAccount (idx);
	}
}
}
