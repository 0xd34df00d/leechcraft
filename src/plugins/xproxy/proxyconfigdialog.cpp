/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxyconfigdialog.h"
#include "structures.h"

namespace LC
{
namespace XProxy
{
	ProxyConfigDialog::ProxyConfigDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
	}

	Proxy ProxyConfigDialog::GetProxy () const
	{
		auto type = QNetworkProxy::ProxyType::NoProxy;
		switch (Ui_.ProxyType_->currentIndex ())
		{
		case 0:
			type = QNetworkProxy::ProxyType::Socks5Proxy;
			break;
		case 1:
			type = QNetworkProxy::ProxyType::HttpProxy;
			break;
		case 2:
			type = QNetworkProxy::ProxyType::HttpCachingProxy;
			break;
		case 3:
			type = QNetworkProxy::ProxyType::FtpCachingProxy;
			break;
		case 4:
			type = QNetworkProxy::ProxyType::NoProxy;
			break;
		}

		return
		{
			type,
			Ui_.ProxyHost_->text (),
			Ui_.ProxyPort_->value (),
			Ui_.ProxyUser_->text (),
			Ui_.ProxyPassword_->text ()
		};
	}

	void ProxyConfigDialog::SetProxy (const Proxy& proxy)
	{
		Ui_.ProxyHost_->setText (proxy.Host_);
		Ui_.ProxyPort_->setValue (proxy.Port_);
		Ui_.ProxyUser_->setText (proxy.User_);
		Ui_.ProxyPassword_->setText (proxy.Pass_);
		switch (proxy.Type_)
		{
		case QNetworkProxy::ProxyType::Socks5Proxy:
			Ui_.ProxyType_->setCurrentIndex (0);
			break;
		case QNetworkProxy::ProxyType::HttpProxy:
			Ui_.ProxyType_->setCurrentIndex (1);
			break;
		case QNetworkProxy::ProxyType::HttpCachingProxy:
			Ui_.ProxyType_->setCurrentIndex (2);
			break;
		case QNetworkProxy::ProxyType::FtpCachingProxy:
			Ui_.ProxyType_->setCurrentIndex (3);
			break;
		case QNetworkProxy::ProxyType::NoProxy:
			Ui_.ProxyType_->setCurrentIndex (4);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown proxy type"
					<< proxy.Type_;
			break;
		}
	}
}
}
