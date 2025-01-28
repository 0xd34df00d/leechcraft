/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QMetaType>
#include <util/threads/coro/taskfwd.h>

namespace LC::Monocle
{
	struct RedirectionResult
	{
		QString TargetPath_;
	};

	class IRedirectorPlugin
	{
	public:
		static inline QByteArray PluginClass = "org.LeechCraft.Monocle.IRedirectorPlugin";

		virtual bool CanRedirectDocument (const QString& filename) const = 0;

		virtual QString GetRedirectionMime (const QString& filename) const = 0;

		/** @brief Returns the redirection proxy for the given document.
		 *
		 * This function should return a redirect proxy for the document
		 * at \em filename, or a null pointer if the document cannot be
		 * redirected (for example, if it is invalid or can be handled
		 * directly by this module). However, a null pointer can be
		 * returned only if CanLoadDocument() returned
		 * LoadCheckResult::Can or LoadCheckResult::Cannot for the same
		 * document.
		 *
		 * The default implementation simply does nothing and returns a
		 * null pointer.
		 *
		 * @param[in] filename The document to redirect.
		 * @return The redirect proxy for \em filename, or null pointer.
		 *
		 * @sa LoadDocument()
		 */
		virtual Util::Task<std::optional<RedirectionResult>> GetRedirection (const QString& filename) = 0;
	protected:
		virtual ~IRedirectorPlugin () = default;
	};
}

Q_DECLARE_INTERFACE (LC::Monocle::IRedirectorPlugin, "org.LeechCraft.Monocle.IRedirectorPlugin/1.0")
