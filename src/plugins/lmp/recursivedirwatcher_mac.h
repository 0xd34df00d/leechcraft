/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>

typedef struct __FSEventStream *FSEventStreamRef;

namespace LC
{
namespace LMP
{
	class RecursiveDirWatcherImpl : public QObject
	{
		Q_OBJECT

		QStringList Dirs_;
		FSEventStreamRef EvStream_ = 0;
		quint64 MaxEventId_ = 0;
		bool IsRestarting_ = false;
	public:
		RecursiveDirWatcherImpl (QObject*);
		~RecursiveDirWatcherImpl ();

		void AddRoot (const QString&);
		void RemoveRoot (const QString&);

		void Notify (quint64, const QString&);
	private:
		bool IsRunning () const;

		void Start ();
		void Stop ();
	signals:
		void directoryChanged (const QString&);
	};
}
}
