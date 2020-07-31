/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vkcaptchadialog.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "ui_vkcaptchadialog.h"

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	VkCaptchaDialog::VkCaptchaDialog (const QVariantMap& errorMap,
			QNetworkAccessManager *nam, QWidget *w)
	: VkCaptchaDialog (errorMap ["captcha_img"].toString (),
			errorMap ["captcha_sid"].toString (), nam, w)
	{
	}

	VkCaptchaDialog::VkCaptchaDialog (const QUrl& url,
			const QString& cid, QNetworkAccessManager *manager, QWidget *parent)
	: QDialog (parent)
	, Ui_ (new Ui::VkCaptchaDialog)
	, Cid_ (cid)
	{
		Ui_->setupUi (this);

		auto reply = manager->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotImage ()));
	}

	void VkCaptchaDialog::SetContextName (const QString& context)
	{
		setWindowTitle (tr ("CAPTCHA required for %1").arg (context));
	}

	void VkCaptchaDialog::done (int r)
	{
		QDialog::done (r);

		if (r == DialogCode::Rejected)
			emit gotCaptcha (Cid_, {});
		else
			emit gotCaptcha (Cid_, Ui_->Text_->text ());

		deleteLater ();
	}

	void VkCaptchaDialog::handleGotImage ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QPixmap px;
		px.loadFromData (reply->readAll ());
		Ui_->ImageLabel_->setPixmap (px);
	}
}
}
}
