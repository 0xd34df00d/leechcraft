/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/
#include "keywordsmanagerwidget.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>
#include "keywords.h"
#include "editkeyworddialog.h"

namespace LC
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
		EditKeywordDialog addDialog { {}, {} };

		if (addDialog.exec () != QDialog::Accepted)
			return;

		const auto& keyword = addDialog.GetKeyword ();
		const auto& url = addDialog.GetUrl ();

		if (url.isEmpty () || keyword.isEmpty ())
			return;

		bool alreadyExists = Keywords_.allKeys ().contains (keyword);

		if (alreadyExists &&
				QMessageBox::question (this,
						tr ("Keyword already exists"),
						tr ("The keyword %1 already exists. Do you want to update the URL for this keyword?")
							.arg ("<em>" + keyword + "</em>"),
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::Yes) == QMessageBox::No)
				return;

		Keywords_.setValue (keyword, url);

		if (alreadyExists)
		{
			for (int i = 0; i < Model_->rowCount (); i++)
				if (Model_->item (i, 0)->text () == keyword)
				{
					Model_->item (i, 1)->setText (url);
					break;
				}
		}
		else
		{
			const QList<QStandardItem*> items
			{
				new QStandardItem (keyword),
				new QStandardItem (url)
			};
			for (const auto item : items)
				item->setEditable (false);
			Model_->appendRow (items);
		}
		Plugin_->UpdateKeywords (keyword, url);
	}

	void KeywordsManagerWidget::on_Modify__released ()
	{
		const auto& selected = Ui_.Items_->currentIndex ();
		if (!selected.isValid ())
			return;

		const auto row = selected.row ();

		const auto& keyword = Model_->item (row, 0)->text ();
		const auto& url = Model_->item (row, 1)->text ();
		EditKeywordDialog editDialog { url, keyword };

		if (editDialog.exec () != QDialog::Accepted)
			return;

		if (keyword == editDialog.GetKeyword () && url == editDialog.GetUrl ())
			return;

		if (keyword != editDialog.GetKeyword ())
			Keywords_.remove (keyword);

		Keywords_.setValue (editDialog.GetKeyword (), editDialog.GetUrl ());
		Model_->item (row, 0)->setText (editDialog.GetKeyword ());
		Model_->item (row, 1)->setText (editDialog.GetUrl ());
		Plugin_->UpdateKeywords (editDialog.GetKeyword (), editDialog.GetUrl ());
	}

	void KeywordsManagerWidget::on_Remove__released ()
	{
		const auto& selected = Ui_.Items_->currentIndex ();

		if (!selected.isValid ())
			return;

		const auto& keyword = Model_->item (selected.row (), 0)->text ();

		Keywords_.remove (keyword);
		Model_->removeRow (selected.row ());
		Plugin_->RemoveKeyword (keyword);
	}
}
}
}
