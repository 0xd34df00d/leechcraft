/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "settings.h"
#include <QStandardItemModel>
#include "fua.h"
#include "changer.h"

using namespace LeechCraft::Plugins::Poshuku::Plugins::Fua;

Settings::Settings (QStandardItemModel *model, FUA *parent)
: Fua_ (parent)
, Model_ (model)
{
	Ui_.setupUi (this);
	Ui_.Items_->setModel (model);
}

void Settings::on_Add__released ()
{
	Changer changer (Fua_->GetBrowser2ID ());
	if (changer.exec () != QDialog::Accepted)
		return;

	QString domain = changer.GetDomain ();
	QString identification = changer.GetID ();
	QList<QStandardItem*> items;
	items << new QStandardItem (domain)
		<< new QStandardItem (Fua_->GetBrowser2ID ().key (identification))
		<< new QStandardItem (identification);
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

	Changer changer (Fua_->GetBrowser2ID (), domain, identification);
	if (changer.exec () != QDialog::Accepted)
		return;

	domain = changer.GetDomain ();
	identification = changer.GetID ();
	QList<QStandardItem*> items;
	Model_->item (cur.row (), 0)->setText (domain);
	Model_->item (cur.row (), 1)->setText (Fua_->GetBrowser2ID ().key (identification));
	Model_->item (cur.row (), 2)->setText (identification);
	Fua_->Save ();
}

void Settings::on_Remove__released ()
{
	QModelIndex cur = Ui_.Items_->currentIndex ();
	if (!cur.isValid ())
		return;

	qDeleteAll (Model_->takeRow (cur.row ()));
	Fua_->Save ();
}

