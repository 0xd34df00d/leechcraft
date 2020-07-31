/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFileInfo>

class QFileSystemWatcher;

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class FilesWatcher : public QObject
	{
		Q_OBJECT

		QFileSystemWatcher *Watcher_;
	public:
		FilesWatcher (QObject* = 0);

		void Clear ();
		void AddFiles (const QList<QFileInfo>&);
	signals:
		void rereadFiles ();
	};
}
}
}
