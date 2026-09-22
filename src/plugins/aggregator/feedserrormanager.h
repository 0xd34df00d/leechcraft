/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "common.h"

namespace LC::Aggregator
{
	class FeedsErrorManager : public QObject
	{
		Q_OBJECT
	public:
		struct Error
		{
			QString Message_;

			bool operator== (const Error&) const = default;
		};
	private:
		QHash<IDType_t, QList<Error>> Errors_;
	public:
		FeedsErrorManager () = default;

		void AddFeedError (IDType_t, const QString&, const Error&);
		void ClearFeedErrors (IDType_t);
		QList<Error> GetFeedErrors (IDType_t) const;
	signals:
		void gotErrors (IDType_t);
		void clearedErrors (IDType_t);
	};
}
