/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

class QUrl;

namespace LC
{
namespace Poshuku
{
	class IStorageBackend;
	using IStorageBackend_ptr = std::shared_ptr<IStorageBackend>;

	class ILinkOpenModifier;
	using ILinkOpenModifier_ptr = std::shared_ptr<ILinkOpenModifier>;

	class IProxyObject
	{
	public:
		virtual QObject* GetHistoryModel () const = 0;
		virtual QObject* GetFavoritesModel () const = 0;
		virtual QObject* OpenInNewTab (const QUrl& url,
				bool inverted = false) const = 0;

		virtual IStorageBackend_ptr CreateStorageBackend () = 0;

		virtual QString GetUserAgent (const QUrl&) const = 0;

		virtual QVariant GetPoshukuConfigValue (const QByteArray& name) const = 0;

		virtual ILinkOpenModifier_ptr GetLinkOpenModifier () const = 0;

		virtual void RegisterHookable (QObject*) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IProxyObject,
		"org.Deviant.LeechCraft.Poshuku.IProxyObject/1.0")
