/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "tunesourcebase.h"
#include <QFileSystemWatcher>

namespace LC
{
namespace Xtazy
{
	class FileSource : public TuneSourceBase
	{
		Q_OBJECT

		QFileSystemWatcher Watcher_;
	public:
		FileSource (QObject* = nullptr);
	private slots:
		void handleFileChanged (const QString&);
		void handleFilePathChanged ();
	};
}
}
