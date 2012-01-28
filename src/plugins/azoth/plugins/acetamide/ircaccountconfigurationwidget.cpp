/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#include "ircaccountconfigurationwidget.h"
#include <QTextCodec>


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAccountConfigurationWidget::
			IrcAccountConfigurationWidget (QWidget* parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Q_FOREACH (const QByteArray& codec, QTextCodec::availableCodecs ())
			Ui_.DefaultEncoding_->addItem (QString::fromUtf8 (codec));
		Ui_.DefaultEncoding_->model ()->sort (0);
		Ui_.DefaultEncoding_->
				setCurrentIndex (Ui_.DefaultEncoding_->findText ("UTF-8"));
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

	void IrcAccountConfigurationWidget::
			SetNickNames (const QStringList& nicks)
	{
		Ui_.NickNames_->setPlainText (nicks.join ("\n"));
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
		Ui_.DefaultEncoding_->
				setCurrentIndex (Ui_.DefaultEncoding_->findText (encoding));
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

};
};
};
