#include "authenticationdialog.h"

AuthenticationDialog::AuthenticationDialog (const QString& message,
		const QString& login,
		const QString& password,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.Message_->setText (message);
	Ui_.LoginEdit_->setText (login);
	Ui_.PasswordEdit_->setText (password);
}

AuthenticationDialog::~AuthenticationDialog ()
{
}

QString AuthenticationDialog::GetLogin () const
{
	return Ui_.LoginEdit_->text ();
}

QString AuthenticationDialog::GetPassword () const
{
	return Ui_.PasswordEdit_->text ();
}

bool AuthenticationDialog::ShouldSave () const
{
	return Ui_.SaveCredentials_->checkState () == Qt::Checked;
}

