/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
}
}
}
