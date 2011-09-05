/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "nickservidentifywidget.h"
#include <QStandardItemModel>
#include "core.h"
#include "newnickservidentifydialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	NickServIdentifyWidget::NickServIdentifyWidget (QStandardItemModel *model,
			QWidget *parent , Qt::WindowFlags f)
	: QWidget (parent, f)
	, Model_ (model)
	{
		Ui_.setupUi (this);
		Ui_.NickServIdentifyView_->setModel (Model_);
		Model_->setHorizontalHeaderLabels (QStringList () << tr ("Nickname")
				<< tr ("NickServ nickname")
				<< tr ("NickServ auth string")
				<< tr ("Auth message"));
		Ui_.NickServIdentifyView_->horizontalHeader ()->setStretchLastSection (true);

		ReadSettings ();
	}

	void NickServIdentifyWidget::ReadSettings ()
	{
		//TODO restore settings
	}

	void NickServIdentifyWidget::accept ()
	{
		//TODO save settings
	}

	void NickServIdentifyWidget::on_Add__clicked ()
	{
		std::auto_ptr<NewNickServIdentifyDialog> nns (new NewNickServIdentifyDialog (0));
		if (nns->exec () == QDialog::Rejected)
			return;

		QList<QStandardItem*> identify;
		QStandardItem *nickItem = new QStandardItem (nns->GetNickName ());
		QStandardItem *nickServNickItem = new QStandardItem (nns->GetNickServNickName ());
		QStandardItem *authStringItem = new QStandardItem (nns->GetAuthString ());
		QStandardItem *authMessageItem = new QStandardItem (nns->GetAuthMessage ());
		identify << nickItem
				<< nickServNickItem
				<< authStringItem
				<< authMessageItem;
		Model_->appendRow (identify);
	}

	void NickServIdentifyWidget::on_Edit__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;

		std::auto_ptr<NewNickServIdentifyDialog> nns (new NewNickServIdentifyDialog (0));
		nns->SetNickName (Model_->item(index.row (), 0)->text ());
		nns->SetNickServNickName (Model_->item(index.row (), 1)->text ());
		nns->SetAuthString (Model_->item(index.row (), 2)->text ());
		nns->SetAuthMessage (Model_->item(index.row (), 3)->text ());
		
		if (nns->exec () == QDialog::Rejected)
			return;

		Model_->item(index.row (), 0)->setText (nns->GetNickName ());
		Model_->item(index.row (), 1)->setText (nns->GetNickServNickName ());
		Model_->item(index.row (), 2)->setText (nns->GetAuthString ());
		Model_->item(index.row (), 3)->setText (nns->GetAuthMessage ());
	}

	void NickServIdentifyWidget::on_Delete__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;
		Model_->removeRow (index.row ());
	}

}
}
}