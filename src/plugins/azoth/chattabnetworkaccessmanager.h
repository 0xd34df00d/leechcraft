/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkAccessManager>

namespace LC
{
namespace Azoth
{
	class AvatarsManager;

	class ChatTabNetworkAccessManager : public QNetworkAccessManager
	{
		AvatarsManager * const AvatarsMgr_;
	public:
		ChatTabNetworkAccessManager (AvatarsManager*, QObject* = nullptr);
	protected:
		QNetworkReply* createRequest (Operation, const QNetworkRequest&, QIODevice*) override;
	};
}
}
