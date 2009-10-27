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

#include "flashonclickplugin.h"
#include <QDebug>
#include "flashplaceholder.h"
#include "xmlsettingsmanager.h"

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
					FlashOnClickPlugin::FlashOnClickPlugin (QObject *parent)
					: QObject (parent)
					{
					}

					QWebPluginFactory::Plugin FlashOnClickPlugin::Plugin () const
					{
						QWebPluginFactory::Plugin result;
						result.name = "FlashOnClickPlugin";
						QWebPluginFactory::MimeType mime;
						mime.fileExtensions << "swf";
						mime.name = "application/x-shockwave-flash";
						result.mimeTypes << mime;
						return result;
					}

					QWidget* FlashOnClickPlugin::Create (const QString& mime,
							const QUrl& url,
							const QStringList& arguments,
							const QStringList& values)
					{
						if (!XmlSettingsManager::Instance ()->
								property ("EnableFlashOnClick").toBool ())
							return 0;

						return new FlashPlaceHolder (url);
					}
				};
			};
		};
	};
};
