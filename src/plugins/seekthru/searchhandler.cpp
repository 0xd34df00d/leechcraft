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

#include "searchhandler.h"
#include <QToolBar>
#include <QAction>
#include <QUrl>
#include <QFile>
#include <QDomDocument>
#include <QTextCodec>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/util.h>
#include <plugininterface/selectablebrowser.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			const QString SearchHandler::OS_ = "http://a9.com/-/spec/opensearch/1.1/";
			
			SearchHandler::SearchHandler (const Description& d)
			: D_ (d)
			, Viewer_ (new Util::SelectableBrowser)
			, Toolbar_ (new QToolBar)
			{
				setObjectName ("SeekThru SearchHandler");
				Viewer_->Construct (Core::Instance ().GetWebBrowser ());
			
				Action_.reset (Toolbar_->addAction (tr ("Subscribe"),
						this, SLOT (subscribe ())));
				Action_->setProperty ("ActionIcon", "seekthru_subscribe");
			}
			
			int SearchHandler::columnCount (const QModelIndex&) const
			{
				return 3;
			}
			
			QVariant SearchHandler::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				int r = index.row ();
				switch (role)
				{
					case Qt::DisplayRole:
						switch (index.column ())
						{
							case 0:
								return SearchString_;
							case 1:
								if (Results_.at (r).TotalResults_ >= 0)
									return tr ("%n total result(s)", "", Results_.at (r).TotalResults_);
								else
									return tr ("Unknown number of results");
							case 2:
								{
									QString result = D_.ShortName_;
									switch (Results_.at (r).Type_)
									{
										case Result::TypeRSS:
											result += " (RSS)";
											break;
										case Result::TypeAtom:
											result += " (Atom)";
											break;
										case Result::TypeHTML:
											result += " (HTML)";
											break;
									}
									return result;
								}
							default:
								return QString ("");
						}
					case LeechCraft::RoleAdditionalInfo:
						if (Results_.at (r).Type_ == Result::TypeHTML)
						{
							Viewer_->SetNavBarVisible (XmlSettingsManager::Instance ()
									.property ("NavBarVisible").toBool ());
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
					default:
						return QVariant ();
				}
			}
			
			Qt::ItemFlags SearchHandler::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}
			
			QVariant SearchHandler::headerData (int, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return QString ("");
				else
					return QVariant ();
			}
			
			QModelIndex SearchHandler::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex SearchHandler::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int SearchHandler::rowCount (const QModelIndex& parent) const
			{
				return parent.isValid () ? 0 : Results_.size ();
			}
			
			void SearchHandler::Start (const LeechCraft::Request& r)
			{
				SearchString_ = r.String_;
				Q_FOREACH (UrlDescription u, D_.URLs_)
				{
					QUrl url (u.Template_);
					QList<QPair<QString, QString> > items = url.queryItems (),
						newItems;
					QPair<QString, QString> item;
					Q_FOREACH (item, items)
					{
						// Currently skips optional parameters
						if (item.second.size () >= 3 &&
								item.second.at (0) == '{' &&
								item.second.at (item.second.size () - 1) == '}' &&
								item.second.at (item.second.size () - 2) == '?')
							continue;
			
						if (item.second == "{searchTerms}")
							item.second = SearchString_;
						else if (item.second.size () > 2 &&
								*item.second.begin () == '{' &&
								*(item.second.end () - 1) == '}')
						{
							QString key = item.second.mid (1,
									item.second.size () - 2);
							// To the correct string if Params_ has this key or to
							// empty string otherwise.
							item.second = r.Params_ [key].toString ();
						}
						else
							item.second = "";
			
						newItems << item;
					}
					url.setQueryItems (newItems);
			
					QString fname = LeechCraft::Util::GetTemporaryName ();
					LeechCraft::Entity e =
						LeechCraft::Util::MakeEntity (url,
							fname,
							LeechCraft::Internal |
								LeechCraft::DoNotNotifyUser |
								LeechCraft::DoNotSaveInHistory |
								LeechCraft::NotPersistent |
								LeechCraft::DoNotAnnounceEntity,
							u.Type_);
			
					Result job;
					if (u.Type_ == "application/rss+xml")
						job.Type_ = Result::TypeRSS;
					else if (u.Type_ == "application/atom+xml")
						job.Type_ = Result::TypeAtom;
					else if (u.Type_.startsWith ("text/"))
						job.Type_ = Result::TypeHTML;
					else
						continue;
			
					int id = -1;
					QObject *pr;
					emit delegateEntity (e, &id, &pr);
					if (id == -1)
					{
						emit error (tr ("Job for request<br />%1<br />wasn't delegated.")
								.arg (url.toString ()));
						continue;
					}
			
					HandleProvider (pr);
			
					job.Filename_ = fname;
					job.RequestURL_ = url;
					Jobs_ [id] = job;
				}
			}
			
			void SearchHandler::handleJobFinished (int id)
			{
				if (!Jobs_.contains (id))
					return;
			
				Result result = Jobs_ [id];
				Jobs_.remove (id);
			
				QFile file (result.Filename_);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1.")
							.arg (result.Filename_));
					return;
				}
			
				result.Response_ = QTextCodec::codecForName ("UTF-8")->
					toUnicode (file.readAll ());
				result.TotalResults_ = -1;
			
				file.close ();
				if (!file.remove ())
					emit warning (tr ("Could not remove temporary file %1.")
							.arg (result.Filename_));
			
				QDomDocument doc;
				if (doc.setContent (result.Response_, true))
				{
					if (result.Type_ == Result::TypeHTML)
					{
						QDomNodeList nodes = doc.elementsByTagName ("meta");
						for (int i = 0; i < nodes.size (); ++i)
						{
							QDomElement meta = nodes.at (i).toElement ();
							if (meta.isNull ())
								continue;
			
							if (meta.attribute ("name") == "totalResults")
							{
								result.TotalResults_ = meta.attribute ("content").toInt ();
								break;
							}
						}
					}
					else
					{
						QDomNodeList nodes = doc.elementsByTagNameNS (OS_, "totalResults");
						if (nodes.size ())
						{
							QDomElement tr = nodes.at (0).toElement ();
							if (!tr.isNull ())
								result.TotalResults_ = tr.text ().toInt ();
						}
					}
				}
			
				beginInsertRows (QModelIndex (), Results_.size (), Results_.size ());
				Results_ << result;
				endInsertRows ();
			}
			
			void SearchHandler::handleJobError (int id)
			{
				if (!Jobs_.contains (id))
					return;
			
				emit error (tr ("Search request for URL<br />%1<br />was delegated, but it failed.")
						.arg (Jobs_ [id].RequestURL_.toString ()));
				Jobs_.remove (id);
			}
			
			void SearchHandler::subscribe ()
			{
				int r = qobject_cast<QAction*> (sender ())->data ().toInt ();
			
				QString mime;
				if (Results_.at (r).Type_ == Result::TypeAtom)
					mime = "application/atom+xml";
				else if (Results_.at (r).Type_ == Result::TypeRSS)
					mime = "application/rss+xml";
			
				LeechCraft::Entity e =
					LeechCraft::Util::MakeEntity (Results_.at (r).RequestURL_,
							QString (),
							LeechCraft::FromUserInitiated,
							mime);
				emit gotEntity (e);
			}
			
			void SearchHandler::HandleProvider (QObject *provider)
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
			
		};
	};
};

