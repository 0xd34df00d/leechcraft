/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QIcon>
#include <QtPlugin>

namespace LC::Poshuku
{
	class IWebView;
	using IWebView_ptr = std::shared_ptr<IWebView>;

	namespace NewWebViewBehavior
	{
		enum Enum
		{
			None = 0x00,
			Background = 0x01,
		};
	}

	class IWebViewProvider
	{
	protected:
		virtual ~IWebViewProvider () = default;
	public:
		virtual IWebView_ptr CreateWebView () = 0;

		virtual QIcon GetIconForUrl (const QUrl&) const = 0;

		virtual QIcon GetDefaultUrlIcon () const = 0;
	protected:
		virtual void webViewCreated (const IWebView_ptr& view, NewWebViewBehavior::Enum behavior = NewWebViewBehavior::None) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Poshuku::IWebViewProvider,
		"org.LeechCraft.Poshuku.IWebViewProvider/1.0")
