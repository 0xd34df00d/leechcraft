/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <variant>
#include <optional>
#include <QUrl>
#include <QtPlugin>

namespace LC
{
namespace Poshuku
{
	class IWebView;

	class IInterceptableRequests
	{
	protected:
		virtual ~IInterceptableRequests () = default;
	public:
		enum class NavigationType
		{
			Unknown,
			Link,
			Typed,
			FormSubmitted,
			BackForward,
			Reload,
			Other
		};

		enum class ResourceType
		{
			Unknown,
			MainFrame,
			SubFrame,
			Stylesheet,
			Script,
			Image,
			FontResource,
			SubResource,
			Object,
			Media,
			Worker,
			SharedWorker,
			Prefetch,
			Favicon,
			Xhr,
			Ping,
			ServiceWorker,
			CspReport,
			PluginResource,
			Other
		};

		struct RequestInfo
		{
			QUrl RequestUrl_;

			QUrl PageUrl_;
			NavigationType NavType_;
			ResourceType ResourceType_;

			std::optional<IWebView*> View_;
		};

		struct Block {};
		struct Allow {};

		struct Redirect
		{
			QUrl NewUrl_;
		};

		using Result_t = std::variant<Block, Allow, Redirect>;

		using Interceptor_t = std::function<Result_t (RequestInfo)>;

		virtual void AddInterceptor (const Interceptor_t&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IInterceptableRequests,
		"org.LeechCraft.Poshuku.IInterceptableRequests/1.0")
