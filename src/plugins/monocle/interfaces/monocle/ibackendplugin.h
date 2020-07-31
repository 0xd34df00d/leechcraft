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
#include "idocument.h"

namespace LC
{
namespace Monocle
{
	class IRedirectProxy;
	typedef std::shared_ptr<IRedirectProxy> IRedirectProxy_ptr;

	/** @brief Basic interface for plugins providing support for various
	 * document formats for Monocle.
	 *
	 * This interface should be implemented by plugins that provide
	 * format backends for Monocle document reader — that is, for those
	 * plugins that can load documents or convert them.
	 *
	 * Some backends only convert a document from their format to another
	 * format, probably supported by another Monocle plugin. This is
	 * called a redirection, and the backend should return
	 * LoadCheckResult::Redirect from the CanLoadDocument() method in
	 * this case. The backend should also return a valid redirect proxy
	 * from the GetRedirection() method in this case.
	 *
	 * @sa IDocument
	 */
	class IBackendPlugin
	{
	public:
		/** @brief Virtual destructor.
		 */
		virtual ~IBackendPlugin () {}

		/** @brief Describes the result of checking whether a file can be
		 * loaded.
		 *
		 * @sa CanLoadDocument()
		 */
		enum class LoadCheckResult
		{
			/** @brief The file cannot be loaded by this backend.
			 */
			Cannot,

			/** @brief The file can be loaded by this backend.
			 */
			Can,

			/** @brief The file cannot be loaded by this backend, but can
			 * be converted to another format.
			 */
			Redirect
		};

		/** @brief Checks whether the given document can be loaded.
		 *
		 * This method should return LoadCheckResult::Can if the document
		 * can possibly be loaded, LoadCheckResult::Cannot if it can't be
		 * loaded at all, and LoadCheckResult::Redirect if the document
		 * can be preprocessed and converted to some other format probably
		 * loadable by another Monocle plugin.
		 *
		 * The cheaper this function is, the better. It is discouraged to
		 * just check by document extension, though.
		 *
		 * It is OK to return nullptr or invalid document from
		 * LoadDocument() even if this method returns
		 * LoadCheckResult::Can for a given \em filename.
		 *
		 * If this function returns LoadCheckResult::Redirect, then
		 * the GetRedirection() method should return a non-null redirect
		 * proxy (which can fail to convert the document, though).
		 *
		 * @param[in] filename Path to the document to check.
		 * @return Whether the document at \em filename can be loaded.
		 *
		 * @sa LoadDocument()
		 * @sa GetRedirection()
		 */
		virtual LoadCheckResult CanLoadDocument (const QString& filename) = 0;

		/** @brief Loads the given document.
		 *
		 * This method should load the document at \em filename and
		 * return a pointer to it, or a null pointer if the document is
		 * invalid.
		 *
		 * The ownership is passed to the caller: that is, this backend
		 * plugin should keep no strong owning references to the returned
		 * document.
		 *
		 * It is OK for this method to return a null or invalid document
		 * even if CanLoadDocument() returned <code>true</code> for this
		 * \em filename.
		 *
		 * @param[in] filename The document to load.
		 * @return The document object for \em filename, or null pointer,
		 * or invalid document if an error has occurred.
		 *
		 * @sa CanLoadDocument()
		 * @sa GetRedirection()
		 * @sa IDocument
		 */
		virtual IDocument_ptr LoadDocument (const QString& filename) = 0;

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
		 * @sa IRedirectProxy
		 */
		virtual IRedirectProxy_ptr GetRedirection (const QString& filename)
		{
			Q_UNUSED (filename)
			return {};
		}

		/** @brief Returns the MIME types supported by the backend.
		 *
		 * Only those MIMEs that can be handled directly (by the
		 * LoadDocument() method) should be returned.
		 *
		 * The returned MIME types are only considered when dealing with
		 * redirections. CanLoadDocument() and LoadDocument() methods can
		 * still be called on a file whose MIME isn't contained in the
		 * returned list. The reverse is also true: CanLoadDocument() and
		 * LoadDocument() can reject loading a document even if its MIME
		 * is contained in the list returned by this method.
		 *
		 * @return The list of MIMEs the backend supports.
		 */
		virtual QStringList GetSupportedMimes () const = 0;

		/** @brief Returns whether the backend supports threads.
		 *
		 * This function returns true if the implementation supports
		 * threaded pages rendering.
		 *
		 * The default implementation simply returns false.
		 *
		 * @return Whether threaded rendering is supported by this
		 * backend.
		 */
		virtual bool IsThreaded () const
		{
			return false;
		}
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IBackendPlugin,
		"org.LeechCraft.Monocle.IBackendPlugin/1.0")
