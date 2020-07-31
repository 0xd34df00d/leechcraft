/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "directorywidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>
#include "accountsmanager.h"
#include "remotedirectoryselectdialog.h"

namespace LC
{
namespace NetStoreManager
{
	DirectoryWidget::DirectoryWidget (Type t, const QByteArray& accId,
			AccountsManager *am, QWidget *parent)
	: QWidget (parent)
	, AccountID_ (accId)
	, AM_ (am)
	, Type_ (t)
	{
		Ui_.setupUi (this);
	}

	void DirectoryWidget::SetPath (const QString& path, bool byHand)
	{
		Path_ = path;
		Ui_.DirPath_->setText (Path_);
		if (byHand)
			emit finished (this);
	}

	QString DirectoryWidget::GetPath () const
	{
		return Path_;
	}

	void DirectoryWidget::on_OpenDir__released ()
	{
		QStringList path;
		switch (Type_)
		{
		case Type::Local:
			path.append (QFileDialog::getExistingDirectory (this,
					tr ("Select directory"),
					Path_.isEmpty () ? QDir::homePath () : Path_));
			break;
		case Type::Remote:
		{
			if (AccountID_.isEmpty ())
			{
				QMessageBox::warning (this,
					"LeechCraft",
					tr ("Account hasn't been selected.\nYou should select an account in first column."));
				break;
			}
			RemoteDirectorySelectDialog dlg (AccountID_, AM_);
			if (dlg.exec () != QDialog::Rejected)
				path = dlg.GetDirectoryPath ();
			break;
		}
		};

		if (path.isEmpty ())
			return;

		SetPath (path.join ("/"), true);
	}

	void DirectoryWidget::on_DirPath__editingFinished ()
	{
		const QString& path = Ui_.DirPath_->text ();
		QStringList pathList;
		if (!path.contains ('/'))
			pathList.append (path);
		else
			pathList = path.split ('/');

		SetPath (pathList.join ("/"), true);
	}

}
}
