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
#include <util/threads/coro.h>

namespace LC::Monocle
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
		 * @return The pending font info request.
		 *
		 * @sa IPendingFontInfoRequest
		 */
		virtual Util::Task<QList<FontInfo>> RequestFontInfos () const = 0;
	};
}


Q_DECLARE_INTERFACE (LC::Monocle::IHaveFontInfo,
		"org.LeechCraft.Monocle.IHaveFontInfo/1.0")
