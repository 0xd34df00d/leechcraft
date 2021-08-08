/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEngineUrlSchemeHandler>

namespace LC::Azoth
{
	class AvatarsManager;

	class AzothSchemeHandler final : public QWebEngineUrlSchemeHandler
	{
		AvatarsManager * const AM_;
	public:
		explicit AzothSchemeHandler (AvatarsManager*, QObject* = nullptr);

		void requestStarted (QWebEngineUrlRequestJob *request) override;
	private:
		void LoadAvatar (const QString& path, QWebEngineUrlRequestJob *request);
	};
}
