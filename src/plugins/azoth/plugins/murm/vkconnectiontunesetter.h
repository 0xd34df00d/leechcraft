/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkConnection;

	class VkConnectionTuneSetter : public QObject
	{
		Q_OBJECT

		VkConnection * const Conn_;
		const ICoreProxy_ptr Proxy_;

		QString LastQuery_;
	public:
		VkConnectionTuneSetter (VkConnection*, const ICoreProxy_ptr&);

		void SetTune (const QVariantMap&);
	private:
		void HandleAudioSearchResults (QNetworkReply*, const QVariantMap&);
		void PublishDumbStatus (const QVariantMap&);
	};
}
}
}
