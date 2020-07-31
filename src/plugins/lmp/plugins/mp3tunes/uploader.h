/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QNetworkAccessManager;

namespace LC
{
struct Entity;

namespace LMP
{
enum class CloudStorageError;

namespace MP3Tunes
{
	class AuthManager;

	class Uploader : public QObject
	{
		Q_OBJECT

		const QString Login_;
		QNetworkAccessManager *NAM_;
		AuthManager *AuthMgr_;

		QString UpAfterAuth_;
	public:
		Uploader (const QString&, QNetworkAccessManager*, AuthManager*, QObject* = 0);

		void Upload (const QString&);
	private slots:
		void handleSidReady (const QString&);
		void handleSidError (const QString&, const QString&);
		void handleUploadFinished ();
	signals:
		void removeThis (const QString&);

		void uploadFinished (const QString&,
				LC::LMP::CloudStorageError, const QString&);
	};
}
}
}
