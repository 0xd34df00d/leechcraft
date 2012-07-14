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

#include "setstatusdialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	namespace
	{
		QByteArray BuildSettingName (const QString& context, State st)
		{
			return QString ("FastStatusText_%1_%2")
					.arg (context)
					.arg (st)
					.toUtf8 ();
		}
	}

	SetStatusDialog::SetStatusDialog (const QString& context, QWidget *parent)
	: QDialog (parent)
	, Context_ (context)
	{
		Ui_.setupUi (this);
		on_StatusBox__currentIndexChanged ();

		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (save ()));
	}

	State SetStatusDialog::GetState () const
	{
		switch (Ui_.StatusBox_->currentIndex ())
		{
		case 1:
			return SChat;
		case 2:
			return SAway;
		case 3:
			return SDND;
		case 4:
			return SXA;
		case 5:
			return SOffline;
		default:
			return SOnline;
		}
	}

	QString SetStatusDialog::GetStatusText () const
	{
		return Ui_.StatusText_->toPlainText ();
	}

	void SetStatusDialog::save ()
	{
		const auto& name = BuildSettingName (Context_, GetState ());
		XmlSettingsManager::Instance ().setProperty (name, GetStatusText ());
	}

	void SetStatusDialog::on_StatusBox__currentIndexChanged ()
	{
		const auto& name = BuildSettingName (Context_, GetState ());
		const auto& str = XmlSettingsManager::Instance ().property (name).toString ();
		Ui_.StatusText_->setText (str);
	}
}
}
