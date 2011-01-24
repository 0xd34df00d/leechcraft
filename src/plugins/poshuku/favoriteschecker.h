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

#ifndef PLUGINS_POSHUKU_FAVORITESCHECKER_H
#define PLUGINS_POSHUKU_FAVORITESCHECKER_H
#include <QMap>
#include <QUrl>
#include <QDateTime>
#include <QNetworkReply>
#include "favoritesmodel.h"

class QProgressDialog;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class FavoritesModel;

			class FavoritesChecker : public QObject
			{
				Q_OBJECT

				FavoritesModel *Model_;
				QList<QNetworkReply*> Pending_;
				QProgressDialog *ProgressDialog_;
				FavoritesModel::items_t Items_;
			public:
				struct Result
				{
					QNetworkReply::NetworkError Error_;
					QString ErrorString_;
					int StatusCode_;
					QUrl RedirectURL_;
					QDateTime LastModified_;
					qint64 Length_;
				};
			private:
				QMap<QUrl, Result> Results_;
			public:
				FavoritesChecker (QObject* = 0);

				void Check ();
			private slots:
				void handleFinished ();
				void handleCanceled ();
			};
		};
	};
};

#endif

