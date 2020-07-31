/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>
#include <QPointer>
#include <QObject>
#include <vmime/utility/progressListener.hpp>

namespace LC
{
namespace Snails
{
	class ProgressListener : public QObject
						   , public vmime::utility::progressListener
	{
		Q_OBJECT

		size_t LastProgress_ = 0;
		size_t LastTotal_ = 0;
	public:
		ProgressListener (QObject* = nullptr);

		void Increment ();

		bool cancel () const;

		void start (const size_t) override;
		void progress (const size_t, const size_t) override;
		void stop (const size_t) override;
	signals:
		void started (quint64);
		void gotProgress (quint64, quint64);
	};

	using ProgressListener_ptr = std::shared_ptr<ProgressListener>;
	using ProgressListener_wptr = std::weak_ptr<ProgressListener>;

	bool operator< (const ProgressListener_wptr&, const ProgressListener_wptr&);
}
}
