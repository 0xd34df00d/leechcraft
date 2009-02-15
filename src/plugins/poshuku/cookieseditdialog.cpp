#include "cookieseditdialog.h"
#include <QPushButton>
#include "cookieseditmodel.h"

CookiesEditDialog::CookiesEditDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (false);

	Model_ = new CookiesEditModel (this);

	Ui_.CookiesView_->setModel (Model_);
	connect (Ui_.CookiesView_,
			SIGNAL (clicked (const QModelIndex&)),
			this,
			SLOT (handleClicked (const QModelIndex&)));

	connect (Ui_.ButtonBox_->button (QDialogButtonBox::Apply),
			SIGNAL (released ()),
			this,
			SLOT (handleAccepted ()));
}

void CookiesEditDialog::handleClicked (const QModelIndex& index)
{
	QNetworkCookie cookie;
	try
	{
		cookie = Model_->GetCookie (index);
	}
	catch (...)
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (false);
		return;
	}

	Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
	Ui_.DomainEdit_->setEnabled (true);
	Ui_.DomainEdit_->setText (cookie.domain ());
	Ui_.NameEdit_->setEnabled (true);
	Ui_.NameEdit_->setText (cookie.name ());
	Ui_.ExpirationEdit_->setEnabled (true);
	Ui_.ExpirationEdit_->setDateTime (cookie.expirationDate ());
	Ui_.PathEdit_->setEnabled (true);
	Ui_.PathEdit_->setText (cookie.path ());
	Ui_.ValueEdit_->setEnabled (true);
	Ui_.ValueEdit_->setText (cookie.value ());
	Ui_.SecureEdit_->setEnabled (true);
	Ui_.SecureEdit_->setCheckState (cookie.isSecure () ?
			Qt::Checked : Qt::Unchecked);
}

void CookiesEditDialog::handleAccepted ()
{
	if (Ui_.RawCookieEdit_->toPlainText ().isEmpty ())
	{
		QNetworkCookie cookie (Ui_.NameEdit_->text ().toUtf8 (),
				Ui_.ValueEdit_->text ().toUtf8 ());
		cookie.setDomain (Ui_.DomainEdit_->text ());
		cookie.setExpirationDate (Ui_.ExpirationEdit_->dateTime ());
		cookie.setPath (Ui_.PathEdit_->text ());
		cookie.setSecure (Ui_.SecureEdit_->checkState () == Qt::Checked);

		Model_->SetCookie (Ui_.CookiesView_->currentIndex (), cookie);
	}
	else
	{
		foreach (QNetworkCookie cookie,
				QNetworkCookie::parseCookies (Ui_.RawCookieEdit_->
					toPlainText ().toUtf8 ()))
			Model_->SetCookie (QModelIndex (), cookie);

		Ui_.RawCookieEdit_->clear ();
	}
}

void CookiesEditDialog::handleDomainChanged ()
{
	Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
}

void CookiesEditDialog::handleNameChanged ()
{
	Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (true);
}

