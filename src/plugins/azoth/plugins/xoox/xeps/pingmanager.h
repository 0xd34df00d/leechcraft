/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QObject>
#include <QString>
#include <QHash>
#include <QXmppClientExtension.h>

class QElapsedTimer;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PingManager : public QXmppClientExtension
	{
	public:
		typedef std::function<void (int)> ReplyHandler_f;
	private:
		struct PingInfo
		{
			std::shared_ptr<QElapsedTimer> Timer_;
			ReplyHandler_f Handler_;
		};

		QHash<QString, PingInfo> Stanza2Info_;
	public:
		bool handleStanza (const QDomElement&);

		void Ping (const QString& jid, const ReplyHandler_f& cb);
	};
}
}
}
