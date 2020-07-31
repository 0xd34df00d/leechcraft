/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QImage>
#include <QDateTime>
#include <QUrl>
#include <interfaces/azoth/ihaveavatars.h>

class QTimer;
class QNetworkAccessManager;

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class SelfAvatarFetcher : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager * const NAM_;

		QTimer * const Timer_;

		const QString FullAddress_;

		struct Urls
		{
			QUrl SmallUrl_;
			QUrl BigUrl_;

			Urls (const QString&);
		} const Urls_;

		QDateTime PreviousDateTime_;
	public:
		SelfAvatarFetcher (QNetworkAccessManager*, const QString&, QObject* = nullptr);

		bool IsValid () const;

		QFuture<QImage> FetchAvatar (IHaveAvatars::Size) const;
	private slots:
		void refetch ();
		void handleHeadFinished ();
	signals:
		void avatarChanged ();
	};
}
}
}
