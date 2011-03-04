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
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QMap>

class QNetworkReply;
class QNetworkAccessManager;

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace Auscrie
		{
			struct Worker
			{
				virtual ~Worker () {}

				virtual QNetworkReply* Post (const QByteArray& imageData,
						const QString& format, QNetworkAccessManager *am) const = 0;
				virtual QString GetLink (const QString& contents, QNetworkReply *reply) const = 0;
			};

			typedef boost::shared_ptr<Worker> Worker_ptr;

			class Poster : public QObject
			{
				Q_OBJECT

				QNetworkReply *Reply_;
			public:
				enum HostingService
				{
					DumpBitcheeseNet,
					SavepicRu,
					ImagebinCa
				};
			private:
				const HostingService Service_;
				QMap<HostingService, Worker_ptr> Workers_;
			public:
				Poster (HostingService,
						const QByteArray&, const QString&,
						QNetworkAccessManager*, QObject* = 0);
			private slots:
				void handleFinished ();
				void handleError ();
			signals:
				void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};

#endif

