/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupeditordialog.h"
#include <QStringListModel>

namespace LC
{
namespace Azoth
{
	GroupEditorDialog::GroupEditorDialog (const QStringList& initial,
			const QStringList& allGroups,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		const auto model = new QStringListModel { allGroups, this };
		Groups_ = new Util::TagsEditorAddon { *Ui_.CategoriesEdit_, *model, Util::TagsEditorAddon::Flag::NoSelectorButton };
		Groups_->BindSelector (*Ui_.GroupsSelector_);
		Groups_->SetTags (initial);
	}

	QStringList GroupEditorDialog::GetGroups () const
	{
		return Groups_->GetTags ();
	}
}
}
