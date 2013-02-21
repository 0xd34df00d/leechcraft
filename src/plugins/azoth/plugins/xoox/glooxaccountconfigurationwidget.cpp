/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "glooxaccountconfigurationwidget.h"
#include <QInputDialog>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	GlooxAccountConfigurationWidget::GlooxAccountConfigurationWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		SetFTMethods (QXmppTransferJob::AnyMethod);
	}

	QString GlooxAccountConfigurationWidget::GetJID () const
	{
		return Ui_.JID_->text ();
	}

	void GlooxAccountConfigurationWidget::SetJID (const QString& jid)
	{
		Ui_.JID_->setText (jid);
	}

	QString GlooxAccountConfigurationWidget::GetNick () const
	{
		return Ui_.Nick_->text ();
	}

	void GlooxAccountConfigurationWidget::SetNick (const QString& nick)
	{
		Ui_.Nick_->setText (nick);
	}

	QString GlooxAccountConfigurationWidget::GetResource () const
	{
		return Ui_.Resource_->text ();
	}

	void GlooxAccountConfigurationWidget::SetResource (const QString& res)
	{
		Ui_.Resource_->setText (res);
	}

	short GlooxAccountConfigurationWidget::GetPriority () const
	{
		return Ui_.Priority_->value ();
	}

	void GlooxAccountConfigurationWidget::SetPriority (short priority)
	{
		Ui_.Priority_->setValue (priority);
	}

	QString GlooxAccountConfigurationWidget::GetHost () const
	{
		return Ui_.CustomAddress_->isChecked () ?
			Ui_.ConnectionHost_->text () :
			QString ();
	}

	void GlooxAccountConfigurationWidget::SetHost (const QString& host)
	{
		Ui_.CustomAddress_->setChecked (host.size ());
		Ui_.ConnectionHost_->setText (host);
	}

	int GlooxAccountConfigurationWidget::GetPort () const
	{
		return Ui_.CustomAddress_->isChecked () ?
			Ui_.ConnectionPort_->value () :
			-1;
	}

	void GlooxAccountConfigurationWidget::SetPort (int port)
	{
		Ui_.CustomAddress_->setChecked (port > 0);
		Ui_.ConnectionPort_->setValue (port);
	}

	int GlooxAccountConfigurationWidget::GetKAInterval () const
	{
		return Ui_.KeepAliveInterval_->value ();
	}

	void GlooxAccountConfigurationWidget::SetKAInterval (int val)
	{
		Ui_.KeepAliveInterval_->setValue (val);
	}

	int GlooxAccountConfigurationWidget::GetKATimeout () const
	{
		return Ui_.KeepAliveTimeout_->value ();
	}

	void GlooxAccountConfigurationWidget::SetKATimeout (int val)
	{
		Ui_.KeepAliveTimeout_->setValue (val);
	}

	bool GlooxAccountConfigurationWidget::GetFileLogEnabled () const
	{
		return Ui_.FileLogCheckbox_->checkState () == Qt::Checked;
	}

	void GlooxAccountConfigurationWidget::SetFileLogEnabled (bool log)
	{
		Ui_.FileLogCheckbox_->setCheckState (log ? Qt::Checked : Qt::Unchecked);
	}

	QXmppConfiguration::StreamSecurityMode GlooxAccountConfigurationWidget::GetTLSMode () const
	{
		return static_cast<QXmppConfiguration::StreamSecurityMode> (Ui_.TLSMode_->currentIndex ());
	}

	void GlooxAccountConfigurationWidget::SetTLSMode (QXmppConfiguration::StreamSecurityMode mode)
	{
		Ui_.TLSMode_->setCurrentIndex (mode);
	}

	QXmppTransferJob::Methods GlooxAccountConfigurationWidget::GetFTMethods () const
	{
		QXmppTransferJob::Methods result;
		if (Ui_.AllowFileTransferIBB_->checkState () == Qt::Checked)
			result |= QXmppTransferJob::InBandMethod;
		if (Ui_.AllowFileTransferSOCKS5_->checkState () == Qt::Checked)
			result |= QXmppTransferJob::SocksMethod;
		return result;
	}

	void GlooxAccountConfigurationWidget::SetFTMethods (QXmppTransferJob::Methods methods)
	{
		const bool ibb = methods & QXmppTransferJob::InBandMethod;
		Ui_.AllowFileTransferIBB_->setCheckState (ibb ? Qt::Checked : Qt::Unchecked);

		const bool socks = methods & QXmppTransferJob::SocksMethod;
		Ui_.AllowFileTransferSOCKS5_->setCheckState (socks ? Qt::Checked : Qt::Unchecked);
	}

	bool GlooxAccountConfigurationWidget::GetUseSOCKS5Proxy () const
	{
		return Ui_.UseSOCKS5Proxy_->isChecked ();
	}

	void GlooxAccountConfigurationWidget::SetUseSOCKS5Proxy (bool use)
	{
		Ui_.UseSOCKS5Proxy_->setChecked (use);
	}

	QString GlooxAccountConfigurationWidget::GetSOCKS5Proxy () const
	{
		return Ui_.SOCKS5ProxyAddress_->text ();
	}

	void GlooxAccountConfigurationWidget::SetSOCKS5Proxy (const QString& proxy)
	{
		Ui_.SOCKS5ProxyAddress_->setText (proxy);
	}

	QString GlooxAccountConfigurationWidget::GetPassword () const
	{
		return Password_;
	}

	void GlooxAccountConfigurationWidget::on_UpdatePassword__released ()
	{
		bool ok = false;
		Password_ = QInputDialog::getText (this,
				tr ("Password update"),
				tr ("Enter new password:"),
				QLineEdit::Password,
				QString (),
				&ok);

		if (Password_.isEmpty ())
			Password_ = ok ? "" : QString ();
	}
}
}
}
