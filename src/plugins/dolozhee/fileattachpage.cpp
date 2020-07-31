/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fileattachpage.h"
#include "reportwizard.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <util/util.h>

namespace LC
{
namespace Dolozhee
{
	FileAttachPage::FileAttachPage (QWidget *page)
	: QWizardPage (page)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Path"), tr ("Size"), tr ("Description") });
		Ui_.FilesView_->setModel (Model_);
	}

	int FileAttachPage::nextId () const
	{
		return ReportWizard::PageID::PreviewRequestPage;
	}

	void FileAttachPage::AddFile (const QString& path)
	{
		auto pathItem = new QStandardItem (path);
		pathItem->setEditable (false);

		auto sizeItem = new QStandardItem (Util::MakePrettySize (QFileInfo (path).size ()));
		sizeItem->setEditable (false);

		auto descrItem = new QStandardItem ();
		Model_->appendRow ({ pathItem, sizeItem, descrItem });
	}

	QStringList FileAttachPage::GetFiles () const
	{
		QStringList result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << Model_->item (i)->text ();
		return result;
	}

	void FileAttachPage::on_AddFile__released ()
	{
		const auto& paths = QFileDialog::getOpenFileNames (this,
				tr ("Select files to attach"),
				QDir::homePath ());

		for (auto path : paths)
			AddFile (path);
	}

	void FileAttachPage::on_RemoveFile__released ()
	{
		const auto& idx = Ui_.FilesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		Model_->removeRow (idx.row ());
	}
}
}
