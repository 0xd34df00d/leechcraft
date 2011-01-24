/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGININTERFACE_CUSTOMNETWORKREPLY_H
#define PLUGININTERFACE_CUSTOMNETWORKREPLY_H
#include <QNetworkReply>
#include "piconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		class PLUGININTERFACE_API CustomNetworkReply : public QNetworkReply
		{
			Q_OBJECT

			QByteArray Content_;
			qint64 Offset_;
		public:
			CustomNetworkReply (QObject* = 0);
			virtual ~CustomNetworkReply ();

			void SetError (NetworkError, const QString& = QString ());
			void SetHeader (QNetworkRequest::KnownHeaders, const QVariant&);
			void SetContentType (const QByteArray&);
			void SetContent (const QString&);
			void SetContent (const QByteArray&);

			void abort ();
			qint64 bytesAvailable () const;
			bool isSequential () const;
		protected:
			qint64 readData (char*, qint64);
		};
	}
}

#endif
