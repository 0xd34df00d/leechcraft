/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "selfavatarfetcher.h"
#include <QTimer>
#include <QStringList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	SelfAvatarFetcher::SelfAvatarFetcher (QObject *parent)
	: QObject (parent)
	, Timer_ (new QTimer (this))
	{
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (refetch ()));
		Timer_->setInterval (120 * 60 * 1000);
	}

	void SelfAvatarFetcher::Restart (const QString& full)
	{
		const QStringList& split = full.split ('@', QString::SkipEmptyParts);

		Name_ = split.value (0);
		Domain_ = split.value (1);
		if (Domain_.endsWith (".ru"))
			Domain_.chop (3);

		if (Timer_->isActive ())
			Timer_->stop ();
		Timer_->start ();

		QTimer::singleShot (2000,
				this,
				SLOT (refetch ()));
	}

	QUrl SelfAvatarFetcher::GetReqURL () const
	{
		QString urlStr = "http://obraz.foto.mail.ru/" + Domain_ + "/" + Name_ + "/_mrimavatarsmall";
		return QUrl (urlStr);
	}

	void SelfAvatarFetcher::refetch ()
	{
		auto nam = Core::Instance ().GetCoreProxy ()->GetNetworkAccessManager ();
		QNetworkReply *reply = nam->head (QNetworkRequest (GetReqURL ()));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleHeadFinished ()));
	}

	void SelfAvatarFetcher::handleHeadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		if (reply->error () == QNetworkReply::ContentNotFoundError)
		{
			qDebug () << Q_FUNC_INFO
					<< "avatar not found for"
					<< Name_
					<< Domain_;
			return;
		}

		const auto& dt = reply->header (QNetworkRequest::LastModifiedHeader).toDateTime ();
		if (dt <= PreviousDateTime_)
			return;

		PreviousDateTime_ = dt;

		auto nam = Core::Instance ().GetCoreProxy ()->GetNetworkAccessManager ();
		QNetworkReply *getReply = nam->get (QNetworkRequest (GetReqURL ()));
		connect (getReply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetFinished ()));
	}

	void SelfAvatarFetcher::handleGetFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		const QImage& image = QImage::fromData (reply->readAll ());
		emit gotImage (image);
	}
}
}
}
