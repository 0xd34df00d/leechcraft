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

#include "videofindproxy.h"
#include <QAction>
#include <QTextCodec>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			VideoFindProxy::VideoFindProxy (const Request& r)
			: FindProxy (r)
			, Type_ (PTInvalid)
			{
				SetError (tr ("Searching for %1...")
						.arg (r.String_));
			}

			QVariant VideoFindProxy::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				switch (role)
				{
					case Qt::DisplayRole:
						if (Error_)
						{
							switch (index.column ())
							{
								case 0:
									return *Error_;
								case 1:
									return tr ("Video vkontakte.ru");
								default:
									return QString ();
							}
						}
						else
						{
							const VideoResult& res = VideoResults_ [index.row ()];
							switch (index.column ())
							{
								case 0:
									return res.Title_;
								case 1:
									return tr ("Video");
								case 2:
									return res.URL_.toString ();
								default:
									return QString ();
							}
						}
					case LeechCraft::RoleControls:
						{
							QUrl url;
							if (VideoResults_.size () > index.row ())
								url = VideoResults_ [index.row ()].URL_;
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
			
			int VideoFindProxy::rowCount (const QModelIndex& parent) const
			{
				if (parent.isValid ())
					return 0;

				if (Error_)
					return 1;
				else
					return VideoResults_.size ();
			}

			QUrl VideoFindProxy::GetURL () const
			{
				QByteArray urlStr = "http://vkontakte.ru/gsearch.php?q=FIND&section=video";
				return QUrl (urlStr.replace ("FIND",
							QTextCodec::codecForName ("Windows-1251")->fromUnicode (R_.String_)));
			}

			void VideoFindProxy::Handle (const QString& contents)
			{
				if (Type_ == PTInvalid)
					HandleSearchResults (contents);
				else
					HandleVideoPage (contents);
			}

			void VideoFindProxy::HandleSearchResults (const QString& contents)
			{
				QRegExp upt (".*><a href=\"video([0-9\\_]*)\">(.*)</a></div>.*",
						Qt::CaseSensitive,
						QRegExp::RegExp2);
				upt.setMinimal (true);
				int pos = 0;
				while (pos >= 0)
				{
					if (contents.mid (pos).contains ("<a href=\"video"))
						pos = upt.indexIn (contents, pos);
					else
						pos = -1;

					if (pos >= 0)
					{
						QStringList captured = upt.capturedTexts ();
						captured.removeFirst ();
						QUrl url = QUrl (QString ("http://vkontakte.ru/video%1")
								.arg (captured.at (0)));
						QString title = captured.at (1);
						title.replace ("<span class=\"match\">", "").replace ("</span>", "");
						/*
						QString descr = captured.at (2);
						QString length = captured.at (3);
						QString date = captured.at (4);
						*/

						VideoResult vr =
						{
							url,
							title
							/*
							length,
							date,
							descr
							*/
						};

						VideoResults_ << vr;
						pos += upt.matchedLength ();
					}
				}

				if (VideoResults_.size ())
				{
					SetError (QString ());

					beginInsertRows (QModelIndex (), 0, VideoResults_.size () - 1);
					endInsertRows ();
				}
				else
					SetError (tr ("Nothing found for %1")
							.arg (R_.String_));
			}

			namespace
			{
				QString GetStringFromRX (const QString& pattern, const QString& contents)
				{
					QString result;
					QRegExp rx (pattern);
					rx.setMinimal (true);
					if (rx.indexIn (contents) != -1)
						result = rx.capturedTexts ().at (1);
					else
						qWarning () << Q_FUNC_INFO
							<< "nothing captured for pattern"
							<< rx.pattern ();
					return result;
				}
			};

			void VideoFindProxy::HandleVideoPage (const QString& contents)
			{
				// http://'host'/assets/videos/'vtag+vkid'.vk.flv
				QString host = GetStringFromRX (".*host:'([0-9a-z\\.]*)'.*", contents);
				QString vtag = GetStringFromRX (".*vtag:'([0-9a-f\\-]*)'.*", contents);
				QString vkid = GetStringFromRX (".*vkid:'([0-9a-f]*)'.*", contents);

				if (host.isEmpty () ||
						vtag.isEmpty () ||
						vkid.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO
						<< "one of required attrs is empty"
						<< host
						<< vtag
						<< vkid
						<< "for"
						<< contents;
					return;
				}

				QString source = "http://HOST/assets/videos/VTAGVKID.vk.flv";
				source.replace ("HOST", host);
				source.replace ("VTAG", vtag);
				source.replace ("VKID", vkid);

				LeechCraft::TaskParameter hd;
				switch (Type_)
				{
					case PTInvalid:
						qWarning () << Q_FUNC_INFO
							<< "invalid Type_, assuming both Download and Handle";
						break;
					case PTHandle:
						hd = OnlyHandle;
						break;
					case PTDownload:
						hd = OnlyDownload;
						break;
				}

				EmitWith (hd, QUrl (source));
			}

			void VideoFindProxy::handleDownload ()
			{
				Type_ = PTDownload;
				HandleAction ();
			}

			void VideoFindProxy::handleHandle ()
			{
				Type_ = PTHandle;
				HandleAction ();
			}

			void VideoFindProxy::HandleAction ()
			{
				QUrl url = qobject_cast<QAction*> (sender ())->data ().value<QUrl> ();

				QString fname = Util::GetTemporaryName ();
				DownloadEntity e =
					Util::MakeEntity (url,
						fname,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);

				int id = -1;
				QObject *pr = 0;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit error (tr ("Job for request<br />%1<br />wasn't delegated.")
							.arg (url.toString ()));
					return;
				}

				Jobs_ [id] = fname;
				HandleProvider (pr);
			}
		};
	};
};

