/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stringlistradiostation.h"
#include <QTimer>
#include <QTemporaryFile>
#include <QtDebug>

namespace LC
{
namespace HotStreams
{
	StringListRadioStation::StringListRadioStation (const QList<QUrl>& urls, const QString& name)
	: Name_ (name)
	, URLs_ (urls)
	{
		QTimer::singleShot (0,
				this,
				SLOT (emitPlaylist ()));
	}

	QObject* StringListRadioStation::GetQObject ()
	{
		return this;
	}

	QString StringListRadioStation::GetRadioName () const
	{
		return Name_;
	}

	void StringListRadioStation::RequestNewStream ()
	{
	}

	void StringListRadioStation::emitPlaylist ()
	{
		QTemporaryFile file;
		file.setAutoRemove (false);
		if (!file.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open temporary file";
			return;
		}

		for (const auto& url : URLs_)
			file.write (url.toEncoded () + '\n');

		file.close ();
		emit gotPlaylist (file.fileName (), "m3u8");
	}
}
}
