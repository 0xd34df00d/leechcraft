/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>
#include <QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QTimer>

namespace LC::Monocle
{
	class FileWatcher : public QObject
	{
		Q_OBJECT

		QString CurrentFile_;
		QFileSystemWatcher Watcher_;

		QTimer ReloadTimer_;

		struct FileIdentity;
		using MaybeFileIdentity = std::optional<FileIdentity>;

		std::unique_ptr<MaybeFileIdentity> LastIdentityHolder_;
		MaybeFileIdentity& LastIdentity_;

		static MaybeFileIdentity MakeIdentity (const QString&);
	public:
		explicit FileWatcher (QObject* = nullptr);
		~FileWatcher () override;

		void SetWatchedFile (const QString&);
	private:
		void CheckReload ();
	signals:
		void reloadNeeded (const QString&);
	};
}
