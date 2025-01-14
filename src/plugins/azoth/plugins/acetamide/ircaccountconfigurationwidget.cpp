/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircaccountconfigurationwidget.h"

namespace LC::Azoth::Acetamide
{
	IrcAccountConfigurationWidget::IrcAccountConfigurationWidget (QWidget* parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		Ui_.DefaultEncoding_->addItems (QStringConverter::availableCodecs ());
		Ui_.DefaultEncoding_->model ()->sort (0);
		Ui_.DefaultEncoding_->setCurrentIndex (Ui_.DefaultEncoding_->findText (QStringLiteral ("UTF-8")));
	}

	void IrcAccountConfigurationWidget::SetRealName (const QString& real)
	{
		Ui_.RealName_->setText (real);
	}

	QString IrcAccountConfigurationWidget::GetRealName () const
	{
		return Ui_.RealName_->text ();
	}

	void IrcAccountConfigurationWidget::SetUserName (const QString& user)
	{
		Ui_.UserName_->setText (user);
	}

	QString IrcAccountConfigurationWidget::GetUserName () const
	{
		return Ui_.UserName_->text ();
	}

	void IrcAccountConfigurationWidget::SetNickNames (const QStringList& nicks)
	{
		Ui_.NickNames_->setPlainText (nicks.join ('\n'));
	}

	QStringList IrcAccountConfigurationWidget::GetNickNames () const
	{
		return Ui_.NickNames_->toPlainText ().split ('\n');
	}

	void IrcAccountConfigurationWidget::SetDefaultServer (const QString& server)
	{
		Ui_.DefaultIrcServer_->setText (server);
	}

	QString IrcAccountConfigurationWidget::GetDefaultServer () const
	{
		return Ui_.DefaultIrcServer_->text ();
	}

	void IrcAccountConfigurationWidget::SetDefaultPort (int port)
	{
		Ui_.DefaultPort_->setValue (port);
	}

	int IrcAccountConfigurationWidget::GetDefaultPort () const
	{
		return Ui_.DefaultPort_->value ();
	}

	void IrcAccountConfigurationWidget::SetDefaultEncoding (const QString& encoding)
	{
		Ui_.DefaultEncoding_->setCurrentIndex (Ui_.DefaultEncoding_->findText (encoding));
	}

	QString IrcAccountConfigurationWidget::GetDefaultEncoding () const
	{
		return Ui_.DefaultEncoding_->currentText ();
	}

	void IrcAccountConfigurationWidget::SetDefaultChannel (const QString& channel)
	{
		Ui_.DefaultChannel_->setText (channel);
	}

	QString IrcAccountConfigurationWidget::GetDefaultChannel () const
	{
		return Ui_.DefaultChannel_->text ();
	}
}
