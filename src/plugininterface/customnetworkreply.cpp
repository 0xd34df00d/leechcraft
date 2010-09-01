/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "customnetworkreply.h"
#include <cstring>
#include <QTimer>

namespace LeechCraft
{
	namespace Util
	{
		CustomNetworkReply::CustomNetworkReply (QObject *parent)
		: QNetworkReply (parent)
		{
		}

		CustomNetworkReply::~CustomNetworkReply ()
		{
		}

		void CustomNetworkReply::SetError (QNetworkReply::NetworkError error, const QString& text)
		{
			setError (error, text);
		}

		void CustomNetworkReply::SetHeader (QNetworkRequest::KnownHeaders header, const QVariant& value)
		{
			setHeader (header, value);
		}

		void CustomNetworkReply::SetContentType (const QByteArray& ct)
		{
			setHeader (QNetworkRequest::ContentTypeHeader, ct);
		}

		void CustomNetworkReply::SetContent (const QString& content)
		{
			SetContent (content.toUtf8 ());
		}

		void CustomNetworkReply::SetContent (const QByteArray& content)
		{
			Content_ = content;
			Offset_ = 0;

			open (ReadOnly | Unbuffered);

			SetHeader (QNetworkRequest::ContentLengthHeader, QVariant (Content_.size ()));

			QTimer::singleShot (0,
					this,
					SIGNAL (readyRead ()));
			QTimer::singleShot (0,
					this,
					SIGNAL (finished ()));
		}

		void CustomNetworkReply::abort ()
		{
		}

		qint64 CustomNetworkReply::bytesAvailable () const
		{
			return Content_.size () - Offset_;
		}

		bool CustomNetworkReply::isSequential () const
		{
			return true;
		}

		qint64 CustomNetworkReply::readData (char *data, qint64 maxSize)
		{
			if (Offset_ >= Content_.size ())
				return -1;

			qint64 number = std::min (maxSize, bytesAvailable ());
			std::memcpy (data, Content_.constData () + Offset_, number);
			Offset_ += number;

			return number;
		}
	}
}
