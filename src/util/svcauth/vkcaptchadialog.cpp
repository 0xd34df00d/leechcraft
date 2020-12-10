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

namespace LC::Util::SvcAuth
{
	VkCaptchaDialog::VkCaptchaDialog (const QVariantMap& errorMap,
			QNetworkAccessManager *nam, QWidget *w)
	: VkCaptchaDialog
		{
			errorMap [QStringLiteral ("captcha_img")].toString (),
			errorMap [QStringLiteral ("captcha_sid")].toString (),
			nam,
			w
		}
	{
	}

	VkCaptchaDialog::VkCaptchaDialog (const QUrl& url,
			const QString& cid, QNetworkAccessManager *manager, QWidget *parent)
	: QDialog (parent)
	, Ui_ (std::make_unique<Ui::VkCaptchaDialog> ())
	, Cid_ (cid)
	{
		Ui_->setupUi (this);

		auto reply = manager->get (QNetworkRequest (url));
		connect (reply,
				&QNetworkReply::finished,
				this,
				[this, reply]
				{
					reply->deleteLater ();

					QPixmap px;
					px.loadFromData (reply->readAll ());
					Ui_->ImageLabel_->setPixmap (px);
				});
	}

	VkCaptchaDialog::~VkCaptchaDialog () = default;

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
}
