/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QFile>

namespace LC
{
namespace LMP
{
	enum class SyncConfLevel
	{
		None,
		Medium,
		High
	};

	class ISyncPlugin
	{
	public:
		virtual ~ISyncPlugin () {}

		virtual QObject* GetQObject () = 0;

		virtual QString GetSyncSystemName () const = 0;

		virtual SyncConfLevel CouldSync (const QString& path) = 0;

		virtual void Upload (const QString& localPath, const QString& origLocalPath,
				const QString& to, const QString& relPath) = 0;
	protected:
		virtual void uploadFinished (const QString& localPath,
				QFile::FileError error, const QString& errorStr) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ISyncPlugin, "org.LeechCraft.LMP.ISyncPlugin/1.0")
