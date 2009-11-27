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

#include "youtubeplayer.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
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
					YoutubePlayer::YoutubePlayer (const QUrl& url,
							const QStringList& args, const QStringList& values)
					: Player (url, args, values)
					{
						Setup ();

						QString flashvars = values.at (args.indexOf ("flashvars"));
						QStringList pairs = flashvars.split ("&",
								QString::SkipEmptyParts);
						QString t;
						QString video_id;
						Q_FOREACH (QString pair, pairs)
						{
							QStringList parts = pair.split ("=",
									QString::SkipEmptyParts);
							if (parts.size () != 2)
								continue;

							if (parts.at (0) == "t")
								t = parts.at (1);
							else if (parts.at (0) == "video_id")
								video_id = parts.at (1);
						}
						QList<QPair<QByteArray, QByteArray> > query;
						query << QPair<QByteArray, QByteArray> ("video_id", video_id.toUtf8 ());
						query << QPair<QByteArray, QByteArray> ("t", t.toUtf8 ());

						QUrl vurl = QUrl::fromEncoded ("http://youtube.com/get_video");
						vurl.setEncodedQueryItems (query);

						OriginalURL_ = vurl;

						newQualityRequested (Ui_.Quality_->currentIndex ());
					}

					void YoutubePlayer::Setup ()
					{
						// TODO detect somehow available formats. Maybe the fmt_map
						// in the flashvars would help?

						Ui_.Quality_->addItem ("HD (1280x720)", "22");
						Ui_.Quality_->addItem ("HQ (640x380)", "35");
						Ui_.Quality_->addItem ("mp4 (480x360)", "18");
						Ui_.Quality_->addItem ("flv (320x180)", "34");
						Ui_.Quality_->addItem ("3gp (176x144)", "17");

						Ui_.Quality_->setCurrentIndex (Ui_.Quality_->
								findData (XmlSettingsManager::Instance ()->
									Property ("YoutubePreviousQuality", "fmt34")));

						connect (Ui_.Quality_,
								SIGNAL (currentIndexChanged (int)),
								this,
								SLOT (newQualityRequested (int)));
					}

					void YoutubePlayer::newQualityRequested (int index)
					{
						QString fmt = Ui_.Quality_->itemData (index).toString ();
						qDebug () << index << fmt;

						XmlSettingsManager::Instance ()->
							setProperty ("YoutubePreviousQuality", fmt);

						QUrl url = OriginalURL_;
						url.addQueryItem ("fmt", fmt);
						qDebug () << url.toEncoded ();
						SetVideoUrl (url);
					}

					Player* YoutubePlayerCreator::Create (const QUrl& url,
							const QStringList& args, const QStringList& values) const
					{
						if (!XmlSettingsManager::Instance ()->
								property ("YouTube").toBool ())
							return 0;

						Player *result = 0;
						QString flashvars = values.at (args.indexOf ("flashvars"));
						if (url.host () == "s.ytimg.com" &&
								args.contains ("flashvars") &&
								flashvars.contains ("&video_id=") &&
								flashvars.contains ("&t="))
							return new YoutubePlayer (url, args, values);
						return result;
					}
				};
			};
		};
	};
};


