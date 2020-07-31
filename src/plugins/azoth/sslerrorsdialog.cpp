/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorsdialog.h"
#include <QSslError>
#include <util/network/sslerror2treeitem.h>
#include <util/sll/visitor.h>

namespace LC
{
namespace Azoth
{
	SslErrorsDialog::SslErrorsDialog (const SslErrorsHandler::Context_t& context,
			const QList<QSslError>& errors, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		const auto& contextDescr = Util::Visit (context,
				[] (SslErrorsHandler::AccountRegistration)
				{
					return tr ("SSL errors occured during account registration.");
				},
				[] (const SslErrorsHandler::Account& acc)
				{
					return tr ("SSL errors occured for account %1.")
							.arg ("<em>" + acc.Name_ + "</em>");
				});
		Ui_.ContextText_->setText (contextDescr);

		for (const auto& error : errors)
			Ui_.ErrorsTree_->addTopLevelItem (Util::SslError2TreeItem (error));

		Ui_.ErrorsTree_->expandAll ();
		Ui_.ErrorsTree_->resizeColumnToContents (0);
	}

	bool SslErrorsDialog::ShouldRememberChoice () const
	{
		return Ui_.RememberChoice_->checkState () == Qt::Checked;
	}
}
}
