/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "spacepasteservicebase.h"
#include <QNetworkRequest>
#include <QNetworkReply>

namespace LC::Azoth::Autopaste
{
	void SpacePasteServiceBase::PasteImpl (const PasteParams& params,
			QByteArray baseUrl, const QByteArray& postData)
	{
		if (!baseUrl.endsWith ('/'))
			baseUrl += '/';

		QNetworkRequest req { QUrl { baseUrl } };
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		req.setRawHeader ("Origin", baseUrl);
		req.setRawHeader ("Referer", baseUrl);
		req.setHeader (QNetworkRequest::ContentLengthHeader, postData.size ());
		req.setAttribute (QNetworkRequest::FollowRedirectsAttribute, true);
		InitReply (params.NAM_->post (req, postData));
	}
}

