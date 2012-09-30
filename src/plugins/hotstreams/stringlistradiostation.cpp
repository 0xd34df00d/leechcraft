/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "stringlistradiostation.h"
#include <QTimer>
#include <QTemporaryFile>
#include <QtDebug>

namespace LeechCraft
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

	QObject* StringListRadioStation::GetObject ()
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
