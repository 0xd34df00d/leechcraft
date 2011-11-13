/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#ifndef PLUGINS_LAURE_CHOOSEURLDIALOG_H
#define PLUGINS_LAURE_CHOOSEURLDIALOG_H
#include <QDialog>
#include "ui_chooseurldialog.h"

namespace LeechCraft
{
namespace Laure
{
	/** @brief The ChooseURLDialog class provides a simple dialog
	 * for choosing media contents by links.
	 *  @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class ChooseURLDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::ChooseURLDialog Ui_;
	public:
		/** @brief Constructs a new ChooseURLDialog dialog
		 * with the given parent.
		 */
		ChooseURLDialog (QWidget* = 0);
		
		/** @brief Returns the media URL.
		 * 
		 * @returns QString with an URL.
		 */
		QString GetUrl () const;
		
		/** @brief This function's used to verify the string returned
		 * by GetURL.
		 *
		 * @return true if the string is valid, false otherwise.
		 * 
		 * @sa GetURL()
		 */
		bool IsUrlValid () const;
	};
}
}

#endif // PLUGINS_LAURE_CHOOSEURLDIALOG_H
