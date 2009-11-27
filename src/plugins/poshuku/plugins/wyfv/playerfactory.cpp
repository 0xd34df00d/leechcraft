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
					QList<PlayerFactory::PlayerCreator_f> PlayerFactory::Players_;
					QList<AbstractPlayerCreator*> PlayerFactory::AllocatedCreators_;

					void PlayerFactory::Init ()
					{
						Players_.clear ();
						qDeleteAll (AllocatedCreators_);
						AllocatedCreators_.clear ();

						AllocatedCreators_ << new YoutubePlayerCreator;
						Q_FOREACH (AbstractPlayerCreator *apc, AllocatedCreators_)
							Players_ << PlayerCreator_f (boost::bind (&AbstractPlayerCreator::Create,
										apc,
										_1,
										_2,
										_3));
					}

					Player* PlayerFactory::Create (const QUrl& url,
							const QStringList& args, const QStringList& values)
					{
						Player *result = 0;
						Q_FOREACH (PlayerCreator_f c, Players_)
							if ((result = c (url, args, values)))
								break;
						return result;
					}
				};
			};
		};
	};
};

