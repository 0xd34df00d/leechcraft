/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Qt>
#include <QHash>
#include <QByteArray>
#include <util/models/rolenamesmixin.h>

namespace LC
{
namespace Blasq
{
	enum CollectionRole
	{
		Name = Qt::DisplayRole,
		/** Contains a value of ItemType.
		 */
		Type = Qt::UserRole + 1,
		ID,

		/** Around 240 px.
		 */
		SmallThumb,
		SmallThumbSize,

		/** Around 640 px.
		 */
		MediumThumb,
		MediumThumbSize,

		/** Full-size original.
		 */
		Original,
		OriginalSize,

		CollectionRoleMax
	};

	template<typename T>
	class NamedModel : public Util::RoleNamesMixin<T>
	{
	public:
		NamedModel (QObject *parent)
		: Util::RoleNamesMixin<T> (parent)
		{
			QHash<int, QByteArray> result;
			result [CollectionRole::Type] = "itemType";
			result [CollectionRole::ID] = "imageId";
			result [CollectionRole::Name] = "name";
			result [CollectionRole::SmallThumb] = "smallThumb";
			result [CollectionRole::SmallThumbSize] = "smallThumbSize";
			result [CollectionRole::MediumThumb] = "mediumThumb";
			result [CollectionRole::MediumThumbSize] = "mediumThumbSize";
			result [CollectionRole::Original] = "original";
			result [CollectionRole::OriginalSize] = "originalSize";
			Util::RoleNamesMixin<T>::setRoleNames (result);
		}
	};

	enum ItemType
	{
		Collection,
		AllPhotos,
		Image
	};
}
}
