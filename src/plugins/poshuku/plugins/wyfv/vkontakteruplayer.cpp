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

#include "vkontakteruplayer.h"
#include <QUrl>
#include <QDebug>
#include "xmlsettingsmanager.h"

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
					static QString GetStringFromRX (const QString& pattern, const QString& contents)
					{
						QString result;
						QRegExp rx (pattern);
						if (rx.indexIn (contents) != -1)
							result = rx.capturedTexts ().at (1);
						else
							qWarning () << Q_FUNC_INFO
								<< "nothing captured for pattern"
								<< rx.pattern ();
						return result;
					}

					VkontakteruPlayer::VkontakteruPlayer (const QUrl& url,
						const QStringList& args, const QStringList& values)
					: Player (url, args, values)
					{
						Ui_.Quality_->hide ();
						Ui_.Related_->hide ();

						// http://'host'/assets/videos/'vtag+vkid'.vk.flv
						QString host = GetStringFromRX ("host=([0-9a-z\\.]+)", values[2]);
						QString vtag = GetStringFromRX ("vtag=([0-9a-f\\-]+)", values[2]);
						QString vkid = GetStringFromRX ("vkid=([0-9a-f]+)", values[2]);
		
						if (host.isEmpty () ||
									vtag.isEmpty () ||
									vkid.isEmpty ())
						{      
								qWarning () << Q_FUNC_INFO
									<< "one of required attrs is empty"
									<< host
									<< vtag
									<< vkid;
						} 
						else
						{
							QString source = "http://HOST/assets/videos/VTAGVKID.vk.flv";
							source.replace ("HOST", host);
							source.replace ("VTAG", vtag);
							source.replace ("VKID", vkid);
#ifdef QT_DEBUG
							qDebug () << "source" << source;
#endif
 							SetVideoUrl (source);
						}
					}

					VkontakteruPlayer::~VkontakteruPlayer ()
					{
					}

					bool VkontakteruPlayerCreator::WouldRatherPlay (const QUrl& url) const
					{
						if (!XmlSettingsManager::Instance ()->
								property ("Vkontakte.ru").toBool ())
							return false;

						return url.host () == "vkadre.ru";
					}

					Player* VkontakteruPlayerCreator::Create (const QUrl& url,
							const QStringList& args, const QStringList& values) const
					{
						if (values.size () < 3)
							return 0;

						if (!XmlSettingsManager::Instance ()->
								property ("Vkontakte.ru").toBool ())
							return 0;

						if (values [2].contains ("link=http://vkontakte.ru/video.php") &&
								 (values [2].contains ("vtag=")) &&
								 (values [2].contains ("vkid=")))
							return new VkontakteruPlayer (url, args, values);

						return 0;
					}
				};
			};
		};
	};
};

