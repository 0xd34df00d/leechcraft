/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWebEnginePage>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/poshuku/ilinkopenmodifier.h>

namespace LC::Poshuku
{
class IProxyObject;

namespace WebEngineView
{
	class CustomWebView;

	class CustomWebPage : public QWebEnginePage
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		IProxyObject * const PoshukuProxy_;
		const ILinkOpenModifier_ptr LinkOpenModifier_;
	public:
		CustomWebPage (const ICoreProxy_ptr&, IProxyObject*, QWidget*);
	protected:
		bool acceptNavigationRequest (const QUrl&, NavigationType, bool) override;
	signals:
		void webViewCreated (const std::shared_ptr<CustomWebView>&, bool);
	};
}
}
