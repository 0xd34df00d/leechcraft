/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cookieseditdialog.h"
#include <QPushButton>
#include "cookieseditmodel.h"
#include "cookiesfilter.h"

namespace LC
{
namespace Poshuku
{
	CookiesEditDialog::CookiesEditDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.ButtonBox_->button (QDialogButtonBox::Apply)->setEnabled (false);
	
		Filter_ = new CookiesFilter (this);
		Model_ = new CookiesEditModel (this);
	
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (const QString&)),
				Filter_,
				SLOT (setFilterWildcard (const QString&)));
	
		Filter_->setSourceModel (Model_);
		Ui_.CookiesView_->setModel (Filter_);
		connect (Ui_.CookiesView_,
				SIGNAL (clicked (const QModelIndex&)),
				this,
				SLOT (handleClicked (const QModelIndex&)));
	
		connect (Ui_.ButtonBox_->button (QDialogButtonBox::Apply),
				SIGNAL (released ()),
				this,
				SLOT (handleAccepted ()));
	}
	
	void CookiesEditDialog::handleClicked (const QModelIndex& si)
	{
		QModelIndex index = Filter_->mapToSource (si);
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
	
			Model_->SetCookie (Filter_->mapToSource (Ui_.CookiesView_->currentIndex ()),
					cookie);
		}
		else
		{
			for (const auto& cookie : QNetworkCookie::parseCookies (Ui_.RawCookieEdit_->toPlainText ().toUtf8 ()))
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
	
	void CookiesEditDialog::on_Delete__released ()
	{
		Model_->RemoveCookie (Filter_->mapToSource (Ui_.CookiesView_->currentIndex ()));
	}
}
}
