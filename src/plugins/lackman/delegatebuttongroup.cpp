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

#include "delegatebuttongroup.h"
#include <QAbstractButton>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			DelegateButtonGroup::DelegateButtonGroup (QObject *parent)
			: QObject (parent)
			{
			}

			void DelegateButtonGroup::AddButton (QAbstractButton *button)
			{
				Buttons_ << button;
				connect (button,
						SIGNAL (toggled (bool)),
						this,
						SLOT (handleButtonToggled (bool)));
			}

			void DelegateButtonGroup::handleButtonToggled (bool toggled)
			{
				QAbstractButton *button = qobject_cast<QAbstractButton*> (sender ());
				if (!button)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not a QAbstractButton*"
							<< sender ();
					return;
				}

				if (toggled)
					Q_FOREACH (QAbstractButton *otherButton, Buttons_)
						if (otherButton != button &&
								otherButton->isChecked ())
							otherButton->setChecked (false);
			}
		}
	}
}
