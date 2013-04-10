/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "directorywidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtDebug>

namespace LeechCraft
{
namespace NetStoreManager
{
	DirectoryWidget::DirectoryWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.DirPath_,
				SIGNAL (editingFinished ()),
				this,
				SLOT (handleEditingFinished ()));
	}

	void DirectoryWidget::SetPath (const QString& path, bool byHand)
	{
		if (Path_ == path)
			return;

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
		const QString& path = QFileDialog::getExistingDirectory (this,
				tr ("Select directory"),
				Path_.isEmpty () ? QDir::homePath () : Path_);

		if (path.isEmpty ())
			return;

		SetPath (path, true);
	}

	void DirectoryWidget::handleEditingFinished ()
	{
		const QString& path = Ui_.DirPath_->text ();
		if (!QDir (path).exists ())
		{
			const int res = QMessageBox::question (this,
					"LeechCraft",
					tr ("This directory doesn't exist. Do you want to create it?"),
					QMessageBox::Ok | QMessageBox::Cancel);
			if (res == QMessageBox::Cancel)
			{
				SetPath ("");
				return;
			}
			else
			{
				if (!QDir ().mkpath (path))
				{
					QMessageBox::warning (this,
							"LeechCraft",
							tr ("Unable to create directory"));
					SetPath ("");
					return;
				}
			}
		}
		SetPath (path, true);
	}

}
}
