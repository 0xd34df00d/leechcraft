/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AUSCRIE_POSTER_H
#define PLUGINS_AUSCRIE_POSTER_H
#include <QObject>

class QNetworkReply;
class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Auscrie
		{
			class Poster : public QObject
			{
				Q_OBJECT

				QString Boundary_;
			public:
				Poster (const QByteArray&, const QString&,
						QNetworkAccessManager*, QObject* = 0);
			private:
				QByteArray AddPair (const QString&, const QString&);
				QByteArray AddFile (const QString&, const QString&, const QByteArray&);
			private slots:
				void handleFinished ();
				void handleError ();
			signals:
				void finished (QNetworkReply*);
				void error (QNetworkReply*);
			};
		};
	};
};

#endif

