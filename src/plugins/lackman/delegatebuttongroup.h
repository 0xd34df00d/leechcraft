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

#ifndef PLUGINS_LACKMAN_DELEGATEBUTTONGROUP_H
#define PLUGINS_LACKMAN_DELEGATEBUTTONGROUP_H
#include <QObject>

class QAbstractButton;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			/** @brief Button group with 0 or 1 checked buttons.
			 *
			 * Usual QButtonGroup doesn't allow to have no checked
			 * buttons at all. This class is used to work around this
			 * and allows to have no checked buttons.
			 *
			 * Used in PackagesDelegate to represent the update or
			 * (install|remove) action group.
			 */
			class DelegateButtonGroup : public QObject
			{
				Q_OBJECT

				QList<QAbstractButton*> Buttons_;
			public:
				DelegateButtonGroup(QObject* = 0);

				void AddButton (QAbstractButton*);
			private slots:
				void handleButtonToggled (bool);
			};
		}
	}
}

#endif
