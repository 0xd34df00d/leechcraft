/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QModelIndex;
class QUrl;

namespace LC
{
namespace Blasq
{
	struct UploadItem
	{
		QString FilePath_;
		QString Description_;
	};

	class ISupportUploads
	{
	public:
		/** @brief Describes various pecularities of upload services.
		 */
		enum class Feature
		{
			/** @brief Requires an album to be selected on upload.
			 *
			 * Lacking this feature means that photos can be uploaded
			 * without choosing the album to upload to.
			 */
			RequiresAlbumOnUpload,

			/** @brief Supports putting descriptions alongside the photos.
			 */
			SupportsDescriptions
		};

		virtual ~ISupportUploads () {}

		virtual bool HasUploadFeature (Feature) const = 0;

		virtual void CreateCollection (const QModelIndex& parent) = 0;

		virtual void UploadImages (const QModelIndex& collection, const QList<UploadItem>& paths) = 0;
	protected:
		virtual void itemUploaded (const UploadItem&, const QUrl&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::ISupportUploads, "org.LeechCraft.Blasq.ISupportUploads/1.0")
