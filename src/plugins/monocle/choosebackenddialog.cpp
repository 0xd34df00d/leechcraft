/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "choosebackenddialog.h"
#include <interfaces/iinfo.h>

namespace LC
{
namespace Monocle
{
	ChooseBackendDialog::ChooseBackendDialog (const QList<QObject*>& backends, QWidget *parent)
	: QDialog (parent)
	, Backends_ (backends)
	{
		Ui_.setupUi (this);
		for (auto backend : backends)
		{
			auto ii = qobject_cast<IInfo*> (backend);
			Ui_.BackendSelector_->addItem (ii->GetIcon (),
					QString ("%1 (%2)")
						.arg (ii->GetName ())
						.arg (ii->GetInfo ()));
		}
	}

	QObject* ChooseBackendDialog::GetSelectedBackend () const
	{
		return Backends_.value (Ui_.BackendSelector_->currentIndex ());
	}

	bool ChooseBackendDialog::GetRememberChoice () const
	{
		return Ui_.RememberBox_->checkState () == Qt::Checked;
	}
}
}
