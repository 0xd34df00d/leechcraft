/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "handlenetworkreply.h"

namespace LC::Util
{
	ReplyWithHeaders::ReplyWithHeaders (QNetworkReply *reply)
	: Code_ { reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt () }
	, Data_ { reply->readAll () }
	{
		const auto& raws = reply->rawHeaderPairs ();

		Headers_.reserve (raws.size ());
		for (const auto& [header, value] : raws)
			Headers_ [header] << value;
	}

	ReplyError::ReplyError (QNetworkReply *reply)
	: Error_ { reply->error () }
	, ErrorString_ { reply->errorString () }
	, HttpStatusCode_ { reply->attribute (QNetworkRequest::HttpStatusCodeAttribute) }
	{
	}
}
