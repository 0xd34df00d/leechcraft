/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customnetworkreply.h"
#include <cstring>
#include <QTimer>

namespace LC
{
namespace Util
{
	CustomNetworkReply::CustomNetworkReply (const QUrl& url, QObject *parent)
	: QNetworkReply (parent)
	{
		setUrl (url);
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

		setHeader (QNetworkRequest::ContentLengthHeader, Content_.size ());

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
