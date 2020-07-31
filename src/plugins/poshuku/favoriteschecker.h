/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_FAVORITESCHECKER_H
#define PLUGINS_POSHUKU_FAVORITESCHECKER_H
#include <QMap>
#include <QUrl>
#include <QDateTime>
#include <QNetworkReply>
#include "favoritesmodel.h"

class QProgressDialog;

namespace LC
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
	private:
		void HandleAllDone ();
	private slots:
		void handleFinished ();
		void handleCanceled ();
	};
}
}

#endif
