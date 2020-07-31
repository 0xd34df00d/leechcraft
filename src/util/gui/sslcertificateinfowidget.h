/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <QtNetwork/QSslCertificate>
#include "guiconfig.h"

class QSslCertificate;

namespace Ui
{
	class SslCertificateInfoWidget;
}

namespace LC
{
namespace Util
{
	class UTIL_GUI_API SslCertificateInfoWidget : public QWidget
	{
		std::shared_ptr<Ui::SslCertificateInfoWidget> Ui_;
	public:
		SslCertificateInfoWidget (QWidget* = nullptr);

		void SetCertificate (const QSslCertificate&);
	};

	UTIL_GUI_API QDialog* MakeCertificateViewerDialog (const QSslCertificate&, QWidget* = nullptr);
}
}
