/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Monocle
{
	/** @brief Interface for redirecting document opening requests.
	 *
	 * This interface is used when a backend can't open a document, but
	 * can convert it to a format probably openable by another Monocle
	 * plugin.
	 *
	 * The GetRedirectedMime() method returns the MIME of the converted
	 * document.
	 *
	 * GetRedirectSource() returns the filename of the source document,
	 * and GetRedirectTarget() returns the filename of the converted
	 * document.
	 *
	 * The redirect proxy should start converting after a small delay
	 * after construction (like after spinning the main event loop via
	 * QTimer with a zero timeout). This is because temporary
	 * IRedirectProxy objects can be requested by Monocle to get the
	 * MIME type of the redirected document without actually converting
	 * it.
	 *
	 * @sa IBackendPlugin
	 * @sa IBackendPlugin::GetRedirection()
	 */
	class IRedirectProxy
	{
	public:
		virtual ~IRedirectProxy () {}

		/** @brief Returns this object as a QObject.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the source filename of the document.
		 *
		 * The source filename is what's been passed to the
		 * IBackendPlugin::GetRedirection() method.
		 *
		 * This function should return valid data even before ready() is
		 * emitted.
		 *
		 * @return The file name of the source document being converted.
		 */
		virtual QString GetRedirectSource () const = 0;

		/** @brief Returns the filename of the converted document.
		 *
		 * This function should return valid data even before ready() is
		 * emitted.
		 *
		 * @return The file name of the converted document.
		 */
		virtual QString GetRedirectTarget () const = 0;

		/** @brief Returns the MIME type of the converted document.
		 *
		 * This function should return valid data even before ready() is
		 * emitted.
		 *
		 * @return The MIME type of the converted document.
		 */
		virtual QString GetRedirectedMime () const = 0;
	protected:
		/** @brief Emitted when the document has finished converting.
		 *
		 * This signal should be emitted both when document is converted
		 * successfully and when it failed to be converted.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] target The target document.
		 */
		virtual void ready (const QString& target) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IRedirectProxy,
		"org.LeechCraft.Monocle.IRedirectProxy/1.0")
