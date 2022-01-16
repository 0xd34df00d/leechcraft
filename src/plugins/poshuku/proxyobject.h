/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/poshuku/iproxyobject.h"

namespace LC
{
namespace Poshuku
{
	class ProxyObject : public QObject
					  , public IProxyObject
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IProxyObject)
	public slots:
		QObject* GetHistoryModel () const override;
		QObject* GetFavoritesModel () const override;
		QObject* OpenInNewTab (const QUrl&, bool) const override;

		std::unique_ptr<IBrowserWidget> CreateBrowserWidget () const override;

		IStorageBackend_ptr CreateStorageBackend () override;

		QString GetUserAgent (const QUrl&) const override;

		QVariant GetPoshukuConfigValue (const QByteArray&) const override;

		ILinkOpenModifier_ptr GetLinkOpenModifier () const override;

		void RegisterHookable (QObject*) const override;
	};
}
}
