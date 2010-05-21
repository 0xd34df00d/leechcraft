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

#ifndef COMMONJOBADDER_H
#define COMMONJOBADDER_H
#include <QDialog>
#include "ui_commonjobadder.h"

namespace LeechCraft
{
	/** Dialog for adding tasks directly via LeechCraft. Has two fields,
	 * What and Where, corresponding to Entity_ and Location_ fields of
	 * Entity respectively.
	 */
	class CommonJobAdder : public QDialog,
						   private Ui::CommonJobAdder
	{
		Q_OBJECT
	public:
		/** Creates the dialog and sets the what/where values to
		 * previous ones.
		 *
		 * @param[in] parent The parent widget.
		 */
		CommonJobAdder (QWidget *parent = 0);

		/** Returns the value of What field.
		 *
		 * @return The What.
		 */
		QString GetString () const;
	private slots:
		/** Handles clicking the Browse button near What field. Pops up
		 * the QFileDialog::getOpenFileName dialog.
		 */
		void on_Browse__released ();

		/** Handles clicking the Paste button. Pastes text to the What
		 * field from the clipboard.
		 */
		void on_Paste__released ();
	};
};

#endif

