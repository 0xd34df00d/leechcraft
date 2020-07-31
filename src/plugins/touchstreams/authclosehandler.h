/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}
}
namespace TouchStreams
{
	class AuthCloseHandler : public QObject
	{
		Q_OBJECT

		Util::SvcAuth::VkAuthManager * const Manager_;
	public:
		AuthCloseHandler (Util::SvcAuth::VkAuthManager*, QObject* = 0);
	private slots:
		void handleAuthCanceled ();
	};
}
}
