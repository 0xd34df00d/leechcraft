/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selectgroupsdialog.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "ljprofile.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	SelectGroupsDialog::SelectGroupsDialog (LJProfile *profile, quint32 allowMask,
			QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.Groups_->setModel (Model_);
		Model_->setHorizontalHeaderLabels ({ tr ("Group") });

		for (const auto& group : profile->GetProfileData ().FriendGroups_)
		{
			QStandardItem *item = new QStandardItem (group.Name_);
			item->setData (group.Id_);
			item->setCheckable (true);
			if (allowMask & 1 << group.Id_)
				item->setCheckState (Qt::Checked);
			Model_->appendRow (item);
		}
	}

	QList<uint> SelectGroupsDialog::GetSelectedGroupsIds () const
	{
		QList<uint> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->checkState () == Qt::Checked)
				result << Model_->item (i)->data ().toUInt ();

		return result;
	}

	void SelectGroupsDialog::SetHeaderLabel (const QString& text)
	{
		Ui_.DialogHeaderLabel_->setText (text);
	}

}
}
}

