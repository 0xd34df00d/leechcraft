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

#ifndef PLUGINS_POSHUKU_PLUGINS_WYFV_PLAYERFACTORY_H
#define PLUGINS_POSHUKU_PLUGINS_WYFV_PLAYERFACTORY_H
#include <boost/function.hpp>
#include <QList>
#include "abstractplayercreator.h"

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
					class PlayerFactory
					{
						typedef boost::function<Player*
							(const QUrl&, const QStringList&, const QStringList&)> PlayerCreator_f;
						static QList<PlayerCreator_f> Creators_;

						typedef boost::function<bool (const QUrl&)> SuitablePlayerChecker_f;
						static QList<SuitablePlayerChecker_f> Checkers_;

						static QList<AbstractPlayerCreator*> AllocatedCreators_;

						PlayerFactory ();
					public:
						static void Init ();
						static Player* Create (const QUrl&,
								const QStringList&,
								const QStringList&);
						static bool HasPlayerFor (const QUrl&);
					};
				};
			};
		};
	};
};

#endif

