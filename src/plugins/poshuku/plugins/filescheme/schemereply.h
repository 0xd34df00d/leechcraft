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

#ifndef PLUGINS_POSHUKU_PLUGINS_FILESCHEME_SCHEMEREPLY_H
#define PLUGINS_POSHUKU_PLUGINS_FILESCHEME_SCHEMEREPLY_H
#include <QNetworkReply>
#include <QBuffer>

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace FileScheme
{
	class SchemeReply : public QNetworkReply
	{
		Q_OBJECT

		QBuffer Buffer_;
	public:
		SchemeReply (const QNetworkRequest&, QObject* = 0);
		virtual ~SchemeReply ();

		virtual qint64 bytesAvailable () const;
		virtual void abort ();
		virtual void close ();
	protected:
		virtual qint64 readData (char*, qint64);
	private slots:
		void list ();
	};
}
}
}
}
}

#endif

