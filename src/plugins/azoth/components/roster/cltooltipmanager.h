/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QImage>
#include <util/threads/coro/corocontext.h>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/azoth/azothcommon.h"

namespace LC::Azoth
{
	class ICLEntry;

	class AvatarsManager;

	class CLTooltipManager : public QObject
	{
		Q_OBJECT

		AvatarsManager& Avatars_;
	public:
		explicit CLTooltipManager (AvatarsManager&);

		void AddEntry (ICLEntry*);
		QString GetTooltip (ICLEntry*);
	private:
		void RedisplayTooltip (ICLEntry*);
	private slots:
		void redisplayTooltipForSender ();
	signals:
		void hookTooltipBeforeVariants (LC::IHookProxy_ptr proxy,
				QObject *entry) const;

		void redisplayTooltip (const GlobalStrongestId&);

	LC_CORO_CONTEXT
	};
}
