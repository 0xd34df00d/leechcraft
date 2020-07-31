/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QtPlugin>

class QObject;

namespace LC
{
namespace Monocle
{
	/** @brief Describes a single font.
	 */
	struct FontInfo
	{
		/** @brief The name of the font as it appears in the document.
		 *
		 * This name does not have to be equal to some commonly used name
		 * like "Droid Sans Mono".
		 */
		QString FontName_;

		/** @brief The path to the local font file used.
		 *
		 * This variable makes no sense if the font is embedded.
		 */
		QString LocalPath_;

		/** @brief Whether the font is embedded into the document.
		 */
		bool IsEmbedded_;
	};

	/** @brief A list of FontInfo structures.
	 */
	typedef QList<FontInfo> FontInfos_t;

	/** @brief A proxy object for a pending font info request.
	 *
	 * The IPendingFontInfoRequest object is returned from the
	 * IHaveFontInfo::RequestFontInfos() method. The object is initially
	 * in pending state until the ready() signal is emitted, after which
	 * the font info can be obtained via the GetFontInfos() method.
	 * The GetQObject() method can be used to get a <code>QObject*</code>
	 * to pass in <code>connect()</code>.
	 *
	 * The object is used to support asynchronous font info fetching,
	 * though some single-threaded format implementations may block.
	 *
	 * The IPendingFontInfoRequest objects are self-owning, that is,
	 * they schedule their own destruction shortly after emitting the
	 * ready() signal. The object can be used no later than in slots
	 * connected to the ready() signal via <code>Qt::DirectConnection</code>.
	 */
	class IPendingFontInfoRequest
	{
	public:
		virtual ~IPendingFontInfoRequest () {}

		/** @brief Returns this object as a QObject.
		 *
		 * @return This object as a QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the font information list for the document.
		 *
		 * If the request object is ready, then the list of FontInfo
		 * structures is returned, otherwise an empty list is returned.
		 *
		 * @return The list of font information structures, or an empty
		 * list if the object is not ready.
		 *
		 * @sa ready()
		 */
		virtual QList<FontInfo> GetFontInfos () const = 0;
	protected:
		/** @brief Notifies that the request is completed.
		 *
		 * This signal is emitted to notify that the
		 * IPendingFontInfoRequest object became ready and can now be
		 * queried via the GetFontInfos() method.
		 *
		 * The IPendingFontInfoRequest will schedule the self deletion
		 * after emitting this signal.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @sa GetFontInfos()
		 */
		virtual void ready () = 0;
	};

	/** @brief Interface for querying font information in a document.
	 *
	 * This interface can be implemented by documents supporting querying
	 * font information, like PDF.
	 */
	class IHaveFontInfo
	{
	public:
		virtual ~IHaveFontInfo () {}

		/** @brief Requests the font information for the document.
		 *
		 * The returned object is self-owned, that is, it is destroyed
		 * shortly after becoming ready (and emitting
		 * IPendingFontInfoRequest::ready()).
		 *
		 * This function may or may not block.
		 *
		 * @return The pending font info request.
		 *
		 * @sa IPendingFontInfoRequest
		 */
		virtual IPendingFontInfoRequest* RequestFontInfos () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::IPendingFontInfoRequest,
		"org.LeechCraft.Monocle.IPendingFontInfoRequest/1.0")
Q_DECLARE_INTERFACE (LC::Monocle::IHaveFontInfo,
		"org.LeechCraft.Monocle.IHaveFontInfo/1.0")
