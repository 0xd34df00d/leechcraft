/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "advancedpermchangedialog.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucperms.h"

namespace LC
{
namespace Azoth
{
	AdvancedPermChangeDialog::AdvancedPermChangeDialog (const QList<ICLEntry*>& entries,
			const QByteArray& permClass, const QByteArray& perm, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		QStringList names;
		for (const auto entry : entries)
			names << entry->GetEntryName ();

		auto perms = qobject_cast<IMUCPerms*> (entries.front ()->GetParentCLEntryObject ());
		Ui_.NameLabel_->setText (tr ("Set %1 to %2 for %3")
					.arg (perms->GetUserString (permClass))
					.arg (perms->GetUserString (perm))
					.arg ("<em>" + names.join ("</em>; <em>") + "</em>"));
	}

	QString AdvancedPermChangeDialog::GetReason () const
	{
		return Ui_.Reason_->text ();
	}

	bool AdvancedPermChangeDialog::IsGlobal () const
	{
		return Ui_.ChangeGlobally_->checkState () == Qt::Checked;
	}
}
}
