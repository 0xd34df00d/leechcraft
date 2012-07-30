/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QObject>

class QNetworkAccessManager;

namespace LeechCraft
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
				LeechCraft::LMP::CloudStorageError, const QString&);
	};
}
}
}
