/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QDialog>
#include <QVariantMap>
#include "svcauthconfig.h"

class QNetworkAccessManager;
class QUrl;

namespace Ui
{
	class VkCaptchaDialog;
}

namespace LC::Util::SvcAuth
{
	class UTIL_SVCAUTH_API VkCaptchaDialog : public QDialog
	{
		Q_OBJECT

		const std::unique_ptr<Ui::VkCaptchaDialog> Ui_;

		const QString Cid_;
	public:
		VkCaptchaDialog (const QVariantMap& errorMap, QNetworkAccessManager*, QWidget* = nullptr);
		VkCaptchaDialog (const QUrl&, const QString&, QNetworkAccessManager*, QWidget* = nullptr);
		~VkCaptchaDialog () override;

		void SetContextName (const QString&);

		void done (int) override;
	private slots:
		void handleGotImage ();
	signals:
		void gotCaptcha (const QString& cid, const QString& value);
	};
}
