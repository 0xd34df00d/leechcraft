/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtask.h"
#include <QUrl>
#include <QFileInfo>
#include <QValidator>
#include <QMessageBox>
#include <QFileDialog>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace CSTP
{
	class URLValidator : public QValidator
	{
	public:
		URLValidator (QObject *parent = 0)
		: QValidator (parent)
		{
		}

		virtual ~URLValidator ()
		{
		}

		virtual State validate (QString& input, int&) const
		{
			if (QUrl (input).isValid () || input.isEmpty ())
				return Acceptable;
			else
				return Intermediate;
		}
	};

	AddTask::Task::Task (const QUrl& url,
			const QString& localPath,
			const QString& filename,
			const QString& comment)
	: URL_ (url)
	, LocalPath_ (localPath)
	, Filename_ (filename)
	, Comment_ (comment)
	{
	}

	AddTask::AddTask (QWidget *parent)
	: QDialog (parent)
	, UserModifiedFilename_ (false)
	{
		Ui_.setupUi (this);
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (false);
		Ui_.URL_->setValidator (new URLValidator (this));
		Ui_.LocalPath_->setText (XmlSettingsManager::Instance ().Property ("LocalPath",
					QDir::homePath ()).toString ());
	}

	AddTask::AddTask (const QUrl& url, const QString& where, QWidget *parent)
	: QDialog (parent)
	, UserModifiedFilename_ (false)
	{
		Ui_.setupUi (this);
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (false);
		Ui_.URL_->setValidator (new URLValidator (this));
		Ui_.LocalPath_->setText (where);
		Ui_.URL_->setText (url.toString ());
		on_LocalPath__textChanged ();
		on_URL__textEdited (url.toString ());
		CheckOK ();
	}

	AddTask::~AddTask ()
	{
	}

	AddTask::Task AddTask::GetTask () const
	{
		return Task (Ui_.URL_->text (),
				Ui_.LocalPath_->text (),
				Ui_.Filename_->text (),
				Ui_.Comment_->toPlainText ());
	}

	void AddTask::accept ()
	{
		QFileInfo dir (Ui_.LocalPath_->text ());
		QString message;
		if (!dir.exists ())
			message = tr ("Directory %1 doesn't exist, would you like to "
					"select another?").arg (dir.absolutePath ());
		else if (!dir.isReadable ())
			message = tr ("Directory %1 isn't readable, would you like to "
					"select another?").arg (dir.absolutePath ());
		else if (!dir.isWritable ())
			message = tr ("Directory %1 isn't writable, would you like to "
					"select another?").arg (dir.absolutePath ());
		else if (!dir.isDir ())
			message = tr ("%1 isn't a directory at all, would you like to "
					"select another?").arg (dir.absolutePath ());
		else
		{
			QDialog::accept ();
			return;
		}

		if (QMessageBox::question (this,
					"LeechCraft",
					message,
					QMessageBox::Ok | QMessageBox::Cancel) ==
				QMessageBox::Ok)
			on_BrowseButton__released ();
		else
			QDialog::reject ();
	}

	void AddTask::on_URL__textEdited (const QString& str)
	{
		CheckOK ();
		if (UserModifiedFilename_)
			return;

		Ui_.Filename_->setText (QFileInfo (QUrl (str).path ()).fileName ());
	}

	void AddTask::on_LocalPath__textChanged ()
	{
		CheckOK ();
		XmlSettingsManager::Instance ().setProperty ("LocalPath", Ui_.LocalPath_->text ());
	}

	void AddTask::on_Filename__textEdited ()
	{
		UserModifiedFilename_ = true;
		CheckOK ();
	}

	void AddTask::on_BrowseButton__released ()
	{
		QString dir = QFileDialog::getExistingDirectory (this, tr ("Select directory"),
				XmlSettingsManager::Instance ().property ("LocalPath").toString ());
		if (dir.isEmpty ())
			return;

		Ui_.LocalPath_->setText (dir);
		on_LocalPath__textChanged ();
	}

	void AddTask::CheckOK ()
	{
		bool valid = QUrl (Ui_.URL_->text ()).isValid () &&
				!Ui_.LocalPath_->text ().isEmpty () &&
				!Ui_.Filename_->text ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (valid);
	}
}
}
