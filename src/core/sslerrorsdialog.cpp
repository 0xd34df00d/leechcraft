/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorsdialog.h"
#include <QPushButton>
#include <util/network/sslerror2treeitem.h>

namespace LC
{
SslErrorsDialog::SslErrorsDialog (const QString& host,
		const QList<QSslError>& errors,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);

	Ui_.Host_->setText (host);

	for (const auto& err : errors)
		PopulateTree (err);
	Ui_.Errors_->expandAll ();

	Ui_.ButtonBox_->button (QDialogButtonBox::No)->setDefault (true);
	connect (Ui_.ButtonBox_,
			&QDialogButtonBox::clicked,
			this,
			[this] (QAbstractButton *button)
			{
				const bool ignore = Ui_.ButtonBox_->standardButton (button) == QDialogButtonBox::Yes;
				Answer_ = Answer { .IgnoreErrors_ = ignore, .Permanently_ = Ui_.Permanently_->isChecked () };
				done (ignore ? Accepted : Rejected);
			});
}

std::optional<SslErrorsDialog::Answer> SslErrorsDialog::GetAnswer () const
{
	return Answer_;
}

void SslErrorsDialog::PopulateTree (const QSslError& error)
{
	Ui_.Errors_->addTopLevelItem (Util::SslError2TreeItem (error));
}
}
