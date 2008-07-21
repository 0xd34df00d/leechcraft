#include "addjob.h"
#include <QUrl>
#include <QFileInfo>
#include <QValidator>
#include <QMessageBox>
#include <QFileDialog>

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

	virtual State validate (QString& input, int& pos) const
	{
		if (QUrl (input).isValid () || input.isEmpty ())
			return Acceptable;
		else
			return Intermediate;
	}
};

AddJob::Job::Job (const QString& url,
		const QString& localPath,
		const QString& filename,
		const QString& comment)
: URL_ (url)
, LocalPath_ (localPath)
, Filename_ (filename)
, Comment_ (comment)
{
}

AddJob::AddJob (QWidget *parent)
: QDialog (parent)
, UserModifiedFilename_ (false)
{
	Ui_.setupUi (this);
	Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (false);
	Ui_.URL_->setValidator (new URLValidator (this));
}

AddJob::~AddJob ()
{
}

AddJob::Job AddJob::GetJob () const
{
	return Job (Ui_.URL_->text (),
			Ui_.LocalPath_->text (),
			Ui_.Filename_->text (),
			Ui_.Comment_->toPlainText ());
}

void AddJob::accept ()
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
				tr ("Question"),
				message,
				QMessageBox::Ok | QMessageBox::Cancel) ==
			QMessageBox::Ok)
		on_BrowseButton__released ();
	else
		QDialog::reject ();
}

void AddJob::on_URL__textEdited (const QString& str)
{
	if (UserModifiedFilename_)
		return;

	Ui_.Filename_->setText (QFileInfo (QUrl (str).path ()).fileName ());

	CheckOK ();
}

void AddJob::on_LocalPath__textEdited ()
{
	CheckOK ();
}

void AddJob::on_Filename__textEdited ()
{
	UserModifiedFilename_ = true;
	CheckOK ();
}

void AddJob::on_BrowseButton__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this,
			tr ("Select directory"));							// FIXME show old directory
	if (dir.isEmpty ())
		return;

	Ui_.LocalPath_->setText (dir);
	on_LocalPath__textEdited ();
	// FIXME save selected directory
}

void AddJob::CheckOK ()
{
	bool valid = QUrl (Ui_.URL_->text ()).isValid () &&
			!Ui_.LocalPath_->text ().isEmpty () &&
			!Ui_.Filename_->text ().isEmpty ();
	Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (valid);
}

