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

#include "relateditem.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "related.h"
#include "core.h"

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
					RelatedItem::RelatedItem (QWidget *parent)
					: QWidget (parent)
					{
						Ui_.setupUi (this);
					}

					void RelatedItem::SetRelated (const Related& related)
					{
						Ui_.Title_->setText (related.Title_);
						Ui_.Rating_->setValue (related.Rating_ * 100);

						QNetworkReply *reply = Core::Instance ().GetProxy ()->
							GetNetworkAccessManager ()->get (QNetworkRequest (related.Thumbnail_));
						connect (reply,
								SIGNAL (readyRead ()),
								this,
								SLOT (addToPixmap ()));
						connect (reply,
								SIGNAL (finished ()),
								this,
								SLOT (handlePixmapFinished ()));
					}

					void RelatedItem::addToPixmap ()
					{
						QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
						if (!reply)
						{
							qWarning () << Q_FUNC_INFO
								<< "sender is not a QNetworkReply*"
								<< sender ();
							return;
						}

						PixmapData_.buffer ().append (reply->readAll ());
					}

					void RelatedItem::handlePixmapFinished ()
					{
						QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
						if (!reply)
						{
							qWarning () << Q_FUNC_INFO
								<< "sender is not a QNetworkReply*"
								<< sender ();
							return;
						}

						addToPixmap ();

						reply->deleteLater ();

						QPixmap px;
						if (!px.loadFromData (PixmapData_.buffer ()))
						{
							qWarning () << Q_FUNC_INFO
								<< "failed to create pixmap from loaded data";
							Ui_.Thumbnail_->setText (tr ("Failed to load"));
						}
						else
							Ui_.Thumbnail_->setPixmap (px);
					}
				};
			};
		};
	};
};

