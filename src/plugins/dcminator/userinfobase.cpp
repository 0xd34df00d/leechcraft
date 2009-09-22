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

#include "userinfobase.h"
#include "dcpp/LogManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/UploadManager.h"
#include "dcpp/FavoriteManager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DCminator
		{
			UserInfoBase::UserInfoBase (const dcpp::UserPtr& u)
			: User_ (u)
			{
			}

			void UserInfoBase::GetList (const std::string& hubHint)
			{
				try
				{
					dcpp::QueueManager::getInstance ()->
						addList (User_, hubHint, dcpp::QueueItem::FLAG_CLIENT_VIEW);
				}
				catch (const dcpp::Exception& e)
				{
					dcpp::LogManager::getInstance ()->message (e.getError ());
				}
			}

			void UserInfoBase::BrowseList (const std::string& hubHint)
			{
				if (User_->getCID ().isZero ())
					return;

				try
				{
					dcpp::QueueManager::getInstance ()->
						addList (User_, hubHint,
								dcpp::QueueItem::FLAG_CLIENT_VIEW |
								dcpp::QueueItem::FLAG_PARTIAL_LIST);
				}
				catch (const dcpp::Exception& e)
				{
					dcpp::LogManager::getInstance ()->message (e.getError ());
				}
			}

			void UserInfoBase::MatchQueue (const std::string& hubHint)
			{
				try
				{
					dcpp::QueueManager::getInstance ()->
						addList (User_, hubHint, dcpp::QueueItem::FLAG_MATCH_QUEUE);
				}
				catch (const dcpp::Exception& e)
				{
					dcpp::LogManager::getInstance ()->message (e.getError ());
				}
			}

			void UserInfoBase::PM (const std::string& hubHint)
			{
				// TODO implement
			}

			void UserInfoBase::Grant (const std::string& hubHint)
			{
				dcpp::UploadManager::getInstance ()->reserveSlot (User_, hubHint);
			}

			void UserInfoBase::AddFav ()
			{
				dcpp::FavoriteManager::getInstance ()->addFavoriteUser (User_);
			}

			void UserInfoBase::RemoveFromQueue ()
			{
				dcpp::QueueManager::getInstance ()->
					removeSource (User_, dcpp::QueueItem::Source::FLAG_REMOVED);
			}

			dcpp::UserPtr& UserInfoBase::GetUser ()
			{
				return User_;
			}
		};
	};
};

