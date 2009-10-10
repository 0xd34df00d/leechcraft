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

#include "findproxy.h"
#include <QTextCodec>
#include <QTime>
#include <interfaces/structures.h>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			FindProxy::FindProxy (const Request& r)
			: R_ (r)
			{
			}
			
			FindProxy::~FindProxy ()
			{
			}

			void FindProxy::Start ()
			{
				QUrl url = GetURL ();

				QString fname = LeechCraft::Util::GetTemporaryName ();
				LeechCraft::DownloadEntity e =
					LeechCraft::Util::MakeEntity (url,
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
			
			QAbstractItemModel* FindProxy::GetModel ()
			{
				return this;
			}

			int FindProxy::columnCount (const QModelIndex&) const
			{
				return 3;
			}
			
			QVariant FindProxy::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				int r = index.row ();
				const Result& res = Results_ [index.row ()];
				switch (role)
				{
					case Qt::DisplayRole:
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
					/*
					case LeechCraft::RoleAdditionalInfo:
						if (Results_.at (r).Type_ == Result::TypeHTML)
						{
							Viewer_->SetHtml (Results_.at (r).Response_,
									Results_.at (r).RequestURL_.toString ()); 
							return QVariant::fromValue<QWidget*> (Viewer_.get ());
						}
						else
							return 0;
					case LeechCraft::RoleControls:
						if (Results_.at (r).Type_ != Result::TypeHTML)
						{
							Action_->setData (r);
							return QVariant::fromValue<QToolBar*> (Toolbar_.get ());
						}
						else
							return 0;
				*/
					default:
						return QVariant ();
				}
			}
			
			Qt::ItemFlags FindProxy::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}
			
			QVariant FindProxy::headerData (int, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return QString ("");
				else
					return QVariant ();
			}
			
			QModelIndex FindProxy::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex FindProxy::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int FindProxy::rowCount (const QModelIndex& parent) const
			{
				return parent.isValid () ? 0 : Results_.size ();
			}

			void FindProxy::handleJobFinished (int id)
			{
				if (!Jobs_.contains (id))
					return;

				QString filename = Jobs_ [id];
				Jobs_.remove (id);

				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1.")
							.arg (filename));
					return;
				}

				QString contents = QTextCodec::codecForName ("Windows-1251")->toUnicode (file.readAll ());

				QList<QUrl> urls;
				QList<int> lengths;
//				"http://cs\\1.vkontakte.ru/u\\2/audio/\\3.mp3"
//				\\4 — length in seconds
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

				if (Results_.size ())
				{
					beginRemoveRows (QModelIndex (), 0, Results_.size () - 1);
					Results_.clear ();
					endRemoveRows ();
				}

				int size = urls.size ();
				if (size)
				{
					beginInsertRows (QModelIndex (), 0, urls.size () - 1);
					for (int i = 0; i < size; ++i)
					{
						Result r =
						{
							urls.at (i),
							lengths.at (i),
							infos.at (i).first,
							infos.at (i).second
						};
						Results_ << r;
					}
					endInsertRows ();
				}
			}

			void FindProxy::handleJobError (int id)
			{
				if (!Jobs_.contains (id))
					return;
			
				emit error (tr ("Search request for URL<br />%1<br />was delegated, but it failed.")
						.arg (GetURL ().toString ()));
				Jobs_.remove (id);
			}

			void FindProxy::HandleProvider (QObject *provider)
			{
				if (Downloaders_.contains (provider))
					return;
				
				Downloaders_ << provider;
				connect (provider,
						SIGNAL (jobFinished (int)),
						this,
						SLOT (handleJobFinished (int)));
				connect (provider,
						SIGNAL (jobError (int, IDownload::Error)),
						this,
						SLOT (handleJobError (int)));
			}

			QUrl FindProxy::GetURL () const
			{
				QByteArray urlStr;
				if (R_.Category_.endsWith ("music"))
					urlStr = "http://vkontakte.ru/gsearch.php?q=FIND&section=audio";
				else
				{
					qWarning () << Q_FUNC_INFO
						<< "unknown category"
						<< R_.Category_;
					return QUrl ();
				}

				return QUrl (urlStr.replace ("FIND",
							QTextCodec::codecForName ("Windows-1251")->fromUnicode (R_.String_)));
			}
		};
	};
};

