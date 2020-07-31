/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "audiostructs.h"

template<typename>
class QFuture;

namespace Media
{
	/** @brief Interface for plugins fetching tags for untagged files.
	 *
	 * If a plugin is able to fetch tags from a tags database by, for
	 * example, an audio fingerprint (like AcoustID/MusicBrainz database)
	 * it should implement this interface.
	 */
	class Q_DECL_EXPORT ITagsFetcher
	{
	public:
		virtual ~ITagsFetcher () {}

		/** @brief Requests fetching tags for the given file.
		 *
		 * This function initiates a search for tags for the given file
		 * and returns a QFuture that can be used to obtain the results.
		 *
		 * @param[in] filename The name of the file to search tags for.
		 * @return The QFuture with the tags corresponding to \em filename.
		 */
		virtual QFuture<AudioInfo> FetchTags (const QString& filename) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ITagsFetcher, "org.LeechCraft.Media.ITagsFetcher/1.0")
