/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userfilters.h"
#include <QPlainTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include "userfiltersmodel.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	UserFilters::UserFilters (UserFiltersModel *model, QWidget *parent)
	: QWidget { parent }
	, Model_ { model }
	{
		Ui_.setupUi (this);
		Ui_.View_->setModel (model);

		QShortcut *sh = new QShortcut (Qt::Key_Delete, Ui_.View_);
		connect (sh,
				SIGNAL (activated ()),
				this,
				SLOT (on_Remove__released ()));
		sh->setContext (Qt::WidgetWithChildrenShortcut);
	}

	void UserFilters::on_Add__released ()
	{
		Model_->InitiateAdd ();
	}

	void UserFilters::on_Modify__released ()
	{
		const auto& current = Ui_.View_->currentIndex ();
		if (!current.isValid ())
			return;

		Model_->Modify (current.row ());
	}

	void UserFilters::on_Remove__released ()
	{
		const auto& current = Ui_.View_->currentIndex ();
		if (!current.isValid ())
			return;

		Model_->Remove (current.row ());
	}

	namespace
	{
		void AddMulti (UserFiltersModel *model, const QString& str)
		{
			model->AddMultiFilters (str.split ("\n", Qt::SkipEmptyParts));
		}
	}

	void UserFilters::on_Paste__released ()
	{
		auto edit = new QPlainTextEdit ();

		QDialog dia (this);
		dia.setWindowTitle (tr ("Paste rules"));
		dia.resize (600, 400);
		dia.setLayout (new QVBoxLayout ());
		dia.layout ()->addWidget (new QLabel (tr ("Paste your custom rules here:")));
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

		AddMulti (Model_, edit->toPlainText ());
	}

	void UserFilters::on_Load__released ()
	{
		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Load rules"),
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

		AddMulti (Model_, file.readAll ());
	}

	void UserFilters::accept ()
	{
		Model_->WriteSettings ();
	}

	void UserFilters::reject ()
	{
		Model_->ReadSettings ();
	}
}
}
}
