/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QSslCertificate>
#include "ui_sslstatedialog.h"

class QSslError;
class IIconThemeManager;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class WebPageSslWatcher;

	class SslStateDialog : public QDialog
	{
		Q_OBJECT

		Ui::SslStateDialog Ui_;

		QList<QSslCertificate> Certs_;
	public:
		SslStateDialog (const WebPageSslWatcher*, IIconThemeManager*, QWidget* = nullptr);
	private:
		void FillNonSsl (const QList<QUrl>&);
		void FillErrors (const QMap<QUrl, QList<QSslError>>&);
	private slots:
		void on_CertChainBox__currentIndexChanged (int);
	};
}
}
}
