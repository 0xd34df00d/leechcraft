#include "finddialog.h"

using namespace LeechCraft::Plugins::Poshuku;

FindDialog::FindDialog (QWidget *parent)
: Notification (parent)
{
	Ui_.setupUi (this);
	Ui_.Pattern_->setFocus ();
}

FindDialog::~FindDialog ()
{
}

void FindDialog::on_Pattern__textChanged (const QString& newText)
{
	Ui_.FindButton_->setEnabled (!newText.isEmpty ());
}

void FindDialog::on_FindButton__released ()
{
	QWebPage::FindFlags flags;
	if (Ui_.SearchBackwards_->checkState () == Qt::Checked)
		flags |= QWebPage::FindBackward;
	if (Ui_.MatchCase_->checkState () == Qt::Checked)
		flags |= QWebPage::FindCaseSensitively;
	if (Ui_.WrapAround_->checkState () == Qt::Checked)
		flags |= QWebPage::FindWrapsAroundDocument;

	emit next (Ui_.Pattern_->text (), flags);
}

void FindDialog::SetSuccessful (bool success)
{
	QString ss = QString ("QLineEdit {"
			"background-color:rgb(");
	if (success)
		ss.append ("0,255");
	else
		ss.append ("255,0");
	ss.append (",0) }");
	Ui_.Pattern_->setStyleSheet (ss);
}

void FindDialog::reject ()
{
	hide ();
}


