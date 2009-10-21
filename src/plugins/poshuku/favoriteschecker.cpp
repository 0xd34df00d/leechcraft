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

#include "favoriteschecker.h"
#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFontMetrics>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			FavoritesChecker::FavoritesChecker (QObject *parent)
			: QObject (parent)
			, Model_ (Core::Instance ().GetFavoritesModel ())
			{
				ProgressDialog_ = new QProgressDialog (tr ("Checking Favorites..."),
						tr ("Cancel"),
						0, 0);
				connect (ProgressDialog_,
						SIGNAL (canceled ()),
						this,
						SLOT (handleCanceled ()));
			}

			void FavoritesChecker::Check ()
			{
				if (Pending_.size ())
				{
					QMessageBox::critical (0,
							tr ("LeechCraft"),
							tr ("Already checking links, please wait..."));
					return;
				}

				Pending_.clear ();
				Results_.clear ();

				Items_ = Model_->GetItems ();

				Q_FOREACH (FavoritesModel::FavoritesItem item, Items_)
				{
					QUrl url = QUrl (item.URL_);
					QNetworkRequest req (url);
					QString ua = Core::Instance ().GetUserAgent (url);
					if (!ua.isEmpty ())
						req.setRawHeader ("User-Agent", ua.toLatin1 ());

					QNetworkReply *rep = Core::Instance ()
						.GetNetworkAccessManager ()->head (req);

					rep->setProperty ("SourceURL", url);

					connect (rep,
							SIGNAL (finished ()),
							this,
							SLOT (handleFinished ()));

					Pending_ << rep;
				}

				if (Pending_.size ())
				{
					ProgressDialog_->setRange (0, Pending_.size ());
					ProgressDialog_->setValue (0);
					ProgressDialog_->show ();
				}
			}

			namespace
			{
				QString BuildMessage (const QStringList& list, const QString& property, int num)
				{
					QString result;
					if (!list.size ())
						result = "";
					else if (list.size () < num)
					{
						QString merged;
						Q_FOREACH (QString url, list)
							merged.append (QString ("<li>%1</li>")
									.arg (QApplication::fontMetrics ()
											.elidedText (url, Qt::ElideMiddle, 400)));
						result = FavoritesChecker::tr ("%1 favorites are %2:<br /><ul>%3</ul>")
							.arg (list.size ())
							.arg (property)
							.arg (merged);
					}
					else
						result = FavoritesChecker::tr ("%1 favorites are %2.<br />")
							.arg (list.size ())
							.arg (property);

					return result;
				}
			}

			void FavoritesChecker::handleFinished ()
			{
				QNetworkReply *rep = qobject_cast<QNetworkReply*> (sender ());
				if (!rep)
				{
					qWarning () << Q_FUNC_INFO
						<< "sender is not a QNetworkReply*"
						<< sender ();
					return;
				}

				Pending_.removeAll (rep);
				rep->deleteLater ();

				QUrl url = rep->property ("SourceURL").value<QUrl> ();
				Result result =
				{
					rep->error (),
					rep->errorString (),
					rep->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt (),
					rep->attribute (QNetworkRequest::RedirectionTargetAttribute).value<QUrl> (),
					rep->header (QNetworkRequest::LastModifiedHeader).toDateTime (),
					rep->header (QNetworkRequest::ContentLengthHeader).value<qint64> ()
				};

				Results_ [url] = result;

				ProgressDialog_->setValue (ProgressDialog_->value () + 1);

				if (!Pending_.size ())
				{
					ProgressDialog_->setValue (ProgressDialog_->value () + 1);

					int accessible = 0,
						serverStuff = 0;
					QStringList unaccessibleList;
					QStringList redirectsList;

					QMap<QString, QString> result;

					Q_FOREACH (QUrl key, Results_.keys ())
					{
						QString mres;
						Result res = Results_ [key];
						if (res.Error_ != QNetworkReply::NoError)
						{
							unaccessibleList << key.toString ();
							mres = res.ErrorString_;
						}
						else if (res.StatusCode_ < 200 ||
								res.StatusCode_ > 399)
						{
							++serverStuff;
							mres = tr ("HTTP %1")
								.arg (res.StatusCode_);
						}
						else
						{
							++accessible;
							mres = tr ("HTTP %1")
								.arg (res.StatusCode_);
							if (res.Length_)
								mres += tr ("<br />Length: %1")
									.arg (res.Length_);
							if (res.LastModified_.isValid ())
								mres += tr ("<br />Last-modified: %1")
									.arg (res.LastModified_.toString ());

							if (res.RedirectURL_.isValid ())
							{
								redirectsList << key.toString ();
								mres += tr ("<br />Redirects to %1")
									.arg (res.RedirectURL_.toString ());
							}
						}

						result [key.toString ()] = mres;
					}

					Model_->SetCheckResults (result);

					QString message = tr ("%1 favorites total.<br />"
							"%2 favorites are accessible.<br />"
							"%3"
							"%4 are not correctly returned by the remote server.<br />"
							"%5")
						.arg (accessible + unaccessibleList.size () + serverStuff)
						.arg (accessible)
						.arg (BuildMessage (unaccessibleList, "unaccessible", 10))
						.arg (serverStuff)
						.arg (BuildMessage (redirectsList, "redirected", 10));

					QMessageBox::information (0,
							tr ("LeechCraft"),
							message);

					ProgressDialog_->reset ();
				}
			}

			void FavoritesChecker::handleCanceled ()
			{
				Q_FOREACH (QNetworkReply *rep, Pending_)
					delete rep;

				Pending_.clear ();
				Results_.clear ();
				Items_.clear ();
			}
		};
	};
};

