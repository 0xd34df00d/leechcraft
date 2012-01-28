/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include "keywordsmanagerwidget.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>
#include "keywords.h"
#include "editkeyworddialog.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace Keywords
{
	KeywordsManagerWidget::KeywordsManagerWidget (QStandardItemModel *model, Plugin *plugin)
	: Model_ (model)
	, Plugin_ (plugin)
	, Keywords_ (QCoreApplication::organizationName (),	
			QCoreApplication::applicationName () + "_Poshuku_Keywords")
	{
		Ui_.setupUi (this);
		Ui_.Items_->setModel (Model_); 
	}

	void KeywordsManagerWidget::on_Add__released ()
	{
		EditKeywordDialog addDialog("", "");

		if (addDialog.exec () == QDialog::Accepted)
		{
			const QString& keyword = addDialog.GetKeyword ();
			const QString& url = addDialog.GetUrl ();

			if (url.isEmpty () || keyword.isEmpty ())
				return;

			bool alreadyExists = Keywords_.allKeys ().contains (keyword);

			if (alreadyExists)
				if (QMessageBox::question (this, 
						tr ("Keyword already exists"), 
						tr ("Entered keyword already exists. Change url for this keyword?"),
						QMessageBox::Yes || QMessageBox::No,
						QMessageBox::Yes) == QMessageBox::No)
							return;

			Keywords_.setValue (keyword, url);
			
			if (alreadyExists) 
			{
				for (int i = 0; i < Model_->rowCount (); i++)
				{
					if (Model_->item (i, 0)->text () == keyword)
					{
						Model_->item (i, 1)->setText (url);
						break;
					}
				}
			}
			else 
			{
				QStandardItem *keywordItem = new QStandardItem (keyword);
				QStandardItem *urlItem = new QStandardItem (url);
				QList<QStandardItem*> items; 
				
				items << keywordItem << urlItem;
				Model_->appendRow (items);
			}
			Plugin_->UpdateKeywords (keyword, url);
		}
	}

	void KeywordsManagerWidget::on_Modify__released ()
	{
		const QModelIndex& selected = Ui_.Items_->currentIndex ();

		if (!selected.isValid ())
			return;

		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Ui_.Items_->model ());


		if (!model)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable cast "
				<< Ui_.Items_->model ()
				<< "to QStandardItemModel";
			return;
		}

		const QString& keyword = model->item (selected.row (), 0)->text ();
		const QString& url = model->item (selected.row (), 1)->text ();
		EditKeywordDialog editDialog(url, keyword);

		if (editDialog.exec () == QDialog::Accepted && 
				(keyword != editDialog.GetKeyword () || url != editDialog.GetUrl ()))
		{
			if (keyword != editDialog.GetKeyword ())
				Keywords_.remove (keyword);
			Keywords_.setValue (editDialog.GetKeyword (), editDialog.GetUrl ());
			model->item (selected.row (), 0)->setText (editDialog.GetKeyword ());
			model->item (selected.row (), 1)->setText (editDialog.GetUrl ());
			Plugin_->UpdateKeywords (editDialog.GetKeyword (), editDialog.GetUrl ());
		}
	}

	void KeywordsManagerWidget::on_Remove__released ()
	{
		const QModelIndex& selected = Ui_.Items_->currentIndex ();

		if (!selected.isValid ())
			return;

		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Ui_.Items_->model ());


		if (!model)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable cast "
				<< Ui_.Items_->model ()
				<< "to QStandardItemModel";
			return;
		}

		const QString& keyword = model->item (selected.row (), 0)->text ();

		Keywords_.remove (keyword);
		Model_->removeRow (selected.row ());
		Plugin_->RemoveKeyword (keyword);
	}
}
}
}
