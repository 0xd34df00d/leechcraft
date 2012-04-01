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

#include "userfilters.h"
#include <QPlainTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include "userfiltersmodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	UserFilters::UserFilters (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.View_->setModel (Core::Instance ()
				.GetUserFiltersModel ());

		QShortcut *sh = new QShortcut (Qt::Key_Delete, Ui_.View_);
		connect (sh,
				SIGNAL (activated ()),
				this,
				SLOT (on_Remove__released ()));
		sh->setContext (Qt::WidgetWithChildrenShortcut);
	}

	void UserFilters::on_Add__released ()
	{
		Core::Instance ().GetUserFiltersModel ()->InitiateAdd ();
	}

	void UserFilters::on_Modify__released ()
	{
		QModelIndex current = Ui_.View_->currentIndex ();
		if (!current.isValid ())
			return;

		Core::Instance ()
			.GetUserFiltersModel ()->Modify (current.row ());
	}

	void UserFilters::on_Remove__released ()
	{
		QModelIndex current = Ui_.View_->currentIndex ();
		if (!current.isValid ())
			return;

		Core::Instance ()
			.GetUserFiltersModel ()->Remove (current.row ());
	}

	namespace
	{
		void AddMulti (const QString& str)
		{
			const auto& list = str.split ("\n", QString::SkipEmptyParts);
			Core::Instance ().GetUserFiltersModel ()->AddMultiFilters (list);
		}
	}

	void UserFilters::on_Paste__released ()
	{
		auto edit = new QPlainTextEdit ();

		QDialog dia (this);
		dia.setWindowTitle (tr ("Paste filters"));
		dia.resize (600, 400);
		dia.setLayout (new QVBoxLayout ());
		dia.layout ()->addWidget (new QLineEdit (tr ("Paste your filter strings here:")));
		dia.layout ()->addWidget (edit);
		auto box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		dia.layout ()->addWidget (box);
		connect (box,
				SIGNAL (accepted ()),
				&dia,
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				&dia,
				SLOT (reject ()));

		if (dia.exec () != QDialog::Accepted)
			return;

		AddMulti (edit->toPlainText ());
	}

	void UserFilters::on_Load__released ()
	{
		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Load filters"),
				QDir::homePath ());
		if (filename.isEmpty ())
			return;

		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("Error opening file %1: %2.")
						.arg (filename)
						.arg (file.errorString ()));
			return;
		}

		AddMulti (file.readAll ());
	}
}
}
}
