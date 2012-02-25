/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#pragma once
#include <QDialog>
#include "ui_chooseurldialog.h"

namespace LeechCraft
{
namespace Laure
{
	/** @brief Provides a simple dialog for entering links to media content.
	 * 
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
		 * @return String with an URL.
		 * 
		 * @sa IsUrlValid()
		 */
		QString GetUrl () const;
		
		/** @brief Verifies the string returned by GetUrl().
		 * 
		 * The Url is run through a conformance test. Every part of the
		 * Url must conform to the standard encoding rules of the URI
		 * standard for the URL to be reported as valid.
		 * 
		 * @return True if the string is valid, false otherwise.
		 * 
		 * @sa GetUrl()
		 */
		bool IsUrlValid () const;
	};
}
}
