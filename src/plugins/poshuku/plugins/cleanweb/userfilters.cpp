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

#include "userfilters.h"
#include "userfiltersmodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					UserFilters::UserFilters (QWidget *parent)
					: QWidget (parent)
					{
						Ui_.setupUi (this);
						Ui_.View_->setModel (Core::Instance ()
								.GetUserFiltersModel ());
					}

					void UserFilters::on_Add__released ()
					{
						Core::Instance ().GetUserFiltersModel ()->InitiateAdd ();
					}

					void UserFilters::on_Modify__released ()
					{
						QModelIndex current = Ui_.View_->currentIndex ();
						if (!current.isValid ())
							return;

						Core::Instance ()
							.GetUserFiltersModel ()->Modify (current.row ());
					}

					void UserFilters::on_Remove__released ()
					{
						QModelIndex current = Ui_.View_->currentIndex ();
						if (!current.isValid ())
							return;

						Core::Instance ()
							.GetUserFiltersModel ()->Remove (current.row ());
					}
				};
			};
		};
	};
};
