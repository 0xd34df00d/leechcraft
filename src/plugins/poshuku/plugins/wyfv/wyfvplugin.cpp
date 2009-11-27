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

#include "wyfvplugin.h"
#include <QtDebug>
#include "player.h"
#include "playerfactory.h"

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
					WYFVPlugin::WYFVPlugin (QObject *parent)
					: QObject (parent)
					{
						PlayerFactory::Init ();
					}

					QWebPluginFactory::Plugin WYFVPlugin::Plugin () const
					{
						QWebPluginFactory::Plugin result;
						result.name = "WYFVPlugin";
						QWebPluginFactory::MimeType mime;
						mime.fileExtensions << "swf";
						mime.name = "application/x-shockwave-flash";
						result.mimeTypes << mime;
						return result;
					}

					QWidget* WYFVPlugin::Create (const QString&,
							const QUrl& url,
							const QStringList& args,
							const QStringList& values)
					{
						Player *p = PlayerFactory::Create (url, args, values);
						return p;
					}
				};
			};
		};
	};
};

