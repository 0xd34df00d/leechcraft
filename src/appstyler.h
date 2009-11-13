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

#ifndef APPSTYLER_H
#define APPSTYLER_H
#include <QComboBox>

namespace LeechCraft
{
	/** Widget for selecting the current style in the settings dialog.
	 */
	class AppStyler : public QComboBox
	{
		Q_OBJECT
	public:
		/** Constructs and initializes the AppStyler, preparing it to
		 * be inserted into the settings dialog.
		 */
		AppStyler (QWidget* = 0);
	public slots:
		/** Accepts current style selection.
		 */
		void accept ();

		/** Rejects current styel selection and reverts the state to
		 * the previous one.
		 */
		void reject ();
	};
};

#endif

