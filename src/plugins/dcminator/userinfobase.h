/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_DCMINATOR_USERINFOBASE_H
#define PLUGINS_DCMINATOR_USERINFOBASE_H
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			class UserInfoBase
			{
				dcpp::UserPtr User_;
			public:
				UserInfoBase (const dcpp::UserPtr&);

				void GetList (const std::string&);
				void BrowseList (const std::string&);
				void MatchQueue (const std::string&);
				void PM (const std::string&);
				void Grant (const std::string&);
				void AddFav ();
				void RemoveFromQueue ();

				dcpp::UserPtr& GetUser ();
			};
		};
	};
};

#endif

