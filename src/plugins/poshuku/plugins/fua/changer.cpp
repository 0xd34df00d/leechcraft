/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "changer.h"
#include <QPushButton>

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace Fua
{
	Changer::Changer (const QMap<QString, QString>& ids,
			const QString& suggestedDomain,
			const QString& selectedID,
			QWidget *parent)
	: QDialog (parent)
	, IDs_ (ids)
	{
		Ui_.setupUi (this);

		Ui_.Agent_->addItems (ids.keys ());
		Ui_.Domain_->setText (suggestedDomain);
		Ui_.IDString_->setText (selectedID);
		Ui_.Agent_->setCurrentIndex (Ui_.Agent_->findText (IDs_.key (selectedID)));
		SetEnabled ();
	}

	QString Changer::GetDomain () const
	{
		return Ui_.Domain_->text ();
	}

	QString Changer::GetID () const
	{
		return Ui_.IDString_->text ();
	}

	void Changer::on_Domain__textChanged ()
	{
		SetEnabled ();
	}

	void Changer::on_IDString__textChanged ()
	{
		SetEnabled ();
	}

	void Changer::on_Agent__currentIndexChanged (const QString& agent)
	{
		if (!agent.isEmpty ())
			Ui_.IDString_->setText (IDs_ [agent]);
	}

	void Changer::SetEnabled ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->
			setEnabled (Ui_.Domain_->text ().size () &&
					Ui_.IDString_->text ().size ());
	}
}
}
}
}
}
