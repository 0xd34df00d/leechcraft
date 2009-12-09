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

#include "playerfactory.h"
#include <boost/bind.hpp>
#include "youtubeplayer.h"
#include "vkontakteruplayer.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace WYFV
				{
					QList<PlayerFactory::PlayerCreator_f> PlayerFactory::Creators_;
					QList<PlayerFactory::SuitablePlayerChecker_f> PlayerFactory::Checkers_;
					QList<AbstractPlayerCreator*> PlayerFactory::AllocatedCreators_;

					void PlayerFactory::Init ()
					{
						Creators_.clear ();
						qDeleteAll (AllocatedCreators_);
						AllocatedCreators_.clear ();

						AllocatedCreators_ << new YoutubePlayerCreator;
						AllocatedCreators_ << new VkontakteruPlayerCreator;

						Q_FOREACH (AbstractPlayerCreator *apc, AllocatedCreators_)
						{
							Creators_ << PlayerCreator_f (boost::bind (&AbstractPlayerCreator::Create,
										apc,
										_1,
										_2,
										_3));
							Checkers_ << SuitablePlayerChecker_f (
									boost::bind (&AbstractPlayerCreator::WouldRatherPlay,
										apc,
										_1)
									);
						}
					}

					Player* PlayerFactory::Create (const QUrl& url,
							const QStringList& args, const QStringList& values)
					{
						Player *result = 0;
						Q_FOREACH (PlayerCreator_f c, Creators_)
							if ((result = c (url, args, values)))
								break;
						return result;
					}

					bool PlayerFactory::HasPlayerFor (const QUrl& url)
					{
						Q_FOREACH (SuitablePlayerChecker_f c, Checkers_)
							if (c (url))
								return true;
						return false;
					}
				};
			};
		};
	};
};

