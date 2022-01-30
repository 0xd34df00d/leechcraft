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

namespace LC::LMP::Graffiti
{
	class RecIterator : public QObject
	{
		Q_OBJECT

		const ILMPProxy_ptr LMPProxy_;
		std::atomic<bool> StopFlag_ { false };

		QList<QFileInfo> Result_;
	public:
		explicit RecIterator (ILMPProxy_ptr, QObject* = nullptr);

		void Start (const QString&);
		QList<QFileInfo> GetResult () const;

		void Cancel ();
	signals:
		void finished ();
		void canceled ();
	};
}
