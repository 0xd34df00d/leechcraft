/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>
#include <QFile>

namespace LC
{
namespace LMP
{
struct UnmountableFileInfo;

namespace jOS
{
	class Connection;
	typedef std::shared_ptr<Connection> Connection_ptr;

	class UploadManager : public QObject
	{
		Q_OBJECT

		QMap<QString, UnmountableFileInfo> Infos_;
		QMap<QByteArray, Connection_ptr> AvailableConnections_;
	public:
		UploadManager (QObject* = 0);

		void SetInfo (const QString&, const UnmountableFileInfo&);
		void Upload (const QString&, const QString&, const QByteArray&);
	signals:
		void uploadProgress (qint64, qint64);
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}
