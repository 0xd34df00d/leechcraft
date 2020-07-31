/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <QObject>
#include <QFileInfo>
#include <interfaces/lmp/ilmpplugin.h>

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class RecIterator : public QObject
	{
		Q_OBJECT

		const ILMPProxy_ptr LMPProxy_;
		std::atomic<bool> StopFlag_;

		QList<QFileInfo> Result_;
	public:
		RecIterator (ILMPProxy_ptr, QObject* = 0);

		void Start (const QString&);
		QList<QFileInfo> GetResult () const;
	public slots:
		void cancel ();
	private slots:
		void handleImplFinished ();
	signals:
		void finished ();
		void canceled ();
	};
}
}
}
