/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupeditordialog.h"
#include <QStringListModel>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>

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

		Ui_.GroupsSelector_->setWindowFlags (Qt::Widget);
		Ui_.GroupsSelector_->SetPossibleSelections (allGroups);
		Ui_.GroupsSelector_->SetSelections (initial);

		const auto tc = new Util::TagsCompleter (Ui_.CategoriesLineEdit_);
		tc->OverrideModel (new QStringListModel (allGroups, this));

		Ui_.CategoriesLineEdit_->AddSelector (Ui_.GroupsSelector_);
	}

	QStringList GroupEditorDialog::GetGroups () const
	{
		const auto& text = Ui_.CategoriesLineEdit_->text ();
		return GetProxyHolder ()->GetTagsManager ()->Split (text);
	}
}
}
