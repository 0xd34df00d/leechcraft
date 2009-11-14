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

#include "audiofindproxy.h"
#include <QAction>
#include <QTextCodec>
#include <QTime>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			namespace
			{
				QString Filter (QString str)
				{
					if (str.contains ("<a href='javascript"))
					{
						QRegExp unJS (".*<a href='javascript: showLyrics\\([0-9]*,[0-9]*\\);'>(.*)</a>");
						unJS.setMinimal (true);
						if (unJS.indexIn (str, 0) >= 0)
							str = unJS.cap (1);
					}

					str.replace ("&amp;", "&");
					str.replace ("&#39;", "'");

					return str;
				}
			};

			AudioFindProxy::AudioFindProxy (const Request& r)
			: FindProxy (r)
			{
			}

			QVariant AudioFindProxy::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				switch (role)
				{
					case Qt::DisplayRole:
						if (!AudioResults_.size ())
							return tr ("Searching for audio \"%1\" on vkontakte.ru...")
								.arg (R_.String_);
						else
						{

							const AudioResult& res = AudioResults_ [index.row ()];
							switch (index.column ())
							{
								case 0:
									return QString ("%1 - %2")
										.arg (res.Performer_)
										.arg (res.Title_);
								case 1:
									return QTime (0, 0, 0).addSecs (res.Length_ - 1).toString ();
								case 2:
									return res.URL_.toString ();
								default:
									return QString ();
							}
						}
					case LeechCraft::RoleControls:
						{
							QUrl url = AudioResults_ [index.row ()].URL_;
							if (!url.isEmpty ())
							{
								ActionDownload_->setData (url);
								ActionHandle_->setData (url);
							}
							ActionDownload_->setEnabled (!url.isEmpty ());
							ActionHandle_->setEnabled (!url.isEmpty ());
							return QVariant::fromValue<QToolBar*> (Toolbar_);
						}
					default:
						return QVariant ();
				}
			}
			
			int AudioFindProxy::rowCount (const QModelIndex& parent) const
			{
				if (parent.isValid ())
					return 0;

				int count = AudioResults_.size ();
				if (!count)
					count = 1;

				return count;
			}

			QUrl AudioFindProxy::GetURL () const
			{
				QByteArray urlStr = "http://vkontakte.ru/gsearch.php?q=FIND&section=audio";
				return QUrl (urlStr.replace ("FIND",
							QTextCodec::codecForName ("Windows-1251")->fromUnicode (R_.String_)));
			}

			void AudioFindProxy::Handle (const QString& contents)
			{
				QList<QUrl> urls;
				QList<int> lengths;

				QRegExp links (".*onclick=\"return operate\\([0-9]*,([0-9]*),([0-9]*),'([0-9a-f]*)',([0-9]*)\\);\".*");
				links.setMinimal (true);
				int pos = 0;
				while (pos >= 0)
				{
					if (contents.mid (pos).contains ("return operate"))
						pos = links.indexIn (contents, pos);
					else
						pos = -1;

					if (pos >= 0)
					{
						QStringList captured = links.capturedTexts ();
						captured.removeFirst ();
						urls << QUrl (QString ("http://cs%1.vkontakte.ru/u%2/audio/%3.mp3")
								.arg (captured.at (0))
								.arg (captured.at (1))
								.arg (captured.at (2)));
						lengths << captured.at (3).toInt ();
						pos += links.matchedLength ();
					}
				}

				QList<QPair<QString, QString> > infos;
				QRegExp names (".*performer[0-9]*\">(.*)</b> - <span id=\"title[0-9]*\">(.*)</spa.*");
				names.setMinimal (true);
				pos = 0;
				while (pos >= 0)
				{
					if (contents.mid (pos).contains ("return operate"))
						pos = names.indexIn (contents, pos);
					else
						pos = -1;

					if (pos >= 0)
					{
						QStringList captured = names.capturedTexts ();
						captured.removeFirst ();
						infos << qMakePair<QString, QString> (captured.at (0), captured.at (1));
						pos += names.matchedLength ();
					}
				}

				if (AudioResults_.size ())
				{
					beginRemoveRows (QModelIndex (), 1, AudioResults_.size () - 1);
					AudioResults_.clear ();
					endRemoveRows ();
				}

				int size = urls.size ();
				if (size)
				{
					if (size > 1)
						beginInsertRows (QModelIndex (), 1, size - 1);

					for (int i = 0; i < size; ++i)
					{
						AudioResult r =
						{
							urls.at (i),
							lengths.at (i),
							Filter (infos.at (i).first),
							Filter (infos.at (i).second)
						};
						AudioResults_ << r;
					}

					if (size > 1)
						endInsertRows ();

					emit dataChanged (index (0, 0), index (0, columnCount () - 1));
				}
			}

			void AudioFindProxy::handleDownload ()
			{
				EmitWith (LeechCraft::OnlyDownload,
						qobject_cast<QAction*> (sender ())->data ().value<QUrl> ());
			}

			void AudioFindProxy::handleHandle ()
			{
				EmitWith (LeechCraft::OnlyHandle,
						qobject_cast<QAction*> (sender ())->data ().value<QUrl> ());
			}
		};
	};
};

