/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <QHash>
#include <interfaces/idownload.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "common.h"

namespace LC::Aggregator
{
	class FeedsErrorManager : public QObject
	{
		Q_OBJECT
	public:
		struct ParseError
		{
			QString Error_;

			bool operator== (const ParseError&) const;
		};
		using Error = std::variant<IDownload::Error, ParseError>;
	private:
		const ICoreProxy_ptr Proxy_;
		QHash<IDType_t, QList<Error>> Errors_;
	public:
		FeedsErrorManager () = default;

		void AddFeedError (IDType_t, const Error&);

		void ClearFeedErrors (IDType_t);

		QList<Error> GetFeedErrors (IDType_t) const;
	signals:
		void gotErrors (IDType_t);
		void clearedErrors (IDType_t);
	};
}
