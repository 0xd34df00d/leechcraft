/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settings.h"
#include <QStandardItemModel>
#include "fua.h"
#include "changer.h"

namespace LC
{
namespace Poshuku
{
namespace Fua
{
	Settings::Settings (QStandardItemModel *model, FUA *parent)
	: Fua_ (parent)
	, Model_ (model)
	{
		Ui_.setupUi (this);
		Ui_.Items_->setModel (model);
	}

	void Settings::on_Add__released ()
	{
		Changer changer (Fua_->GetBrowser2ID (), Fua_->GetBackLookupMap ());
		if (changer.exec () != QDialog::Accepted)
			return;

		const auto& identification = changer.GetID ();
		const QList<QStandardItem*> items
		{
			new QStandardItem (changer.GetDomain ()),
			new QStandardItem (Fua_->GetBackLookupMap () [identification]),
			new QStandardItem (identification)
		};
		Model_->appendRow (items);
		Fua_->Save ();
	}

	void Settings::on_Modify__released ()
	{
		QModelIndex cur = Ui_.Items_->currentIndex ();
		if (!cur.isValid ())
			return;

		QString domain = Model_->item (cur.row (), 0)->text ();
		QString identification = Model_->item (cur.row (), 2)->text ();

		Changer changer (Fua_->GetBrowser2ID (), Fua_->GetBackLookupMap (), domain, identification);
		if (changer.exec () != QDialog::Accepted)
			return;

		domain = changer.GetDomain ();
		identification = changer.GetID ();
		Model_->item (cur.row (), 0)->setText (domain);
		Model_->item (cur.row (), 1)->setText (Fua_->GetBackLookupMap () [identification]);
		Model_->item (cur.row (), 2)->setText (identification);
		Fua_->Save ();
	}

	void Settings::on_Remove__released ()
	{
		const auto& cur = Ui_.Items_->currentIndex ();
		if (!cur.isValid ())
			return;

		Model_->removeRow (cur.row ());
		Fua_->Save ();
	}
}
}
}
