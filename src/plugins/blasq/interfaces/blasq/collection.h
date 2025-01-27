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

namespace LC::Blasq
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
		explicit NamedModel (QObject *parent, const QHash<int, QByteArray>& extraNames = {})
		: Util::RoleNamesMixin<T> { parent }
		{
			QHash<int, QByteArray> names
			{
				{ CollectionRole::Type, "itemType" },
				{ CollectionRole::ID, "imageId" },
				{ CollectionRole::Name, "name" },
				{ CollectionRole::SmallThumb, "smallThumb" },
				{ CollectionRole::SmallThumbSize, "smallThumbSize" },
				{ CollectionRole::MediumThumb, "mediumThumb" },
				{ CollectionRole::MediumThumbSize, "mediumThumbSize" },
				{ CollectionRole::Original, "original" },
				{ CollectionRole::OriginalSize, "originalSize" },
			};
			names.insert (extraNames);
			Util::RoleNamesMixin<T>::setRoleNames (names);
		}
	};

	enum ItemType
	{
		Collection,
		AllPhotos,
		Image
	};
}
