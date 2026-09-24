/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

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

		/** @brief Creates a new collection, asking the user for its details.
		 *
		 * @param[in] parentId The CollectionRole::ID of the collection to
		 * create the new one in, or an empty string for the top level.
		 */
		virtual void CreateCollection (const QString& parentId) = 0;

		/** @brief Uploads the given items to the collection.
		 *
		 * @param[in] collectionId The CollectionRole::ID of the collection.
		 * @param[in] items The items to upload.
		 */
		virtual void UploadImages (const QString& collectionId, const QList<UploadItem>& items) = 0;
	protected:
		virtual void itemUploaded (const UploadItem&, const QUrl&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::ISupportUploads, "org.LeechCraft.Blasq.ISupportUploads/1.0")
