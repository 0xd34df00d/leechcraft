/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QList>
#include <QPair>
#include <QVariant>

namespace LC
{
namespace Azoth
{
	class IMetaInfoEntry
	{
	public:
		enum class DataField
		{
			BirthDate
		};

		virtual ~IMetaInfoEntry () {}

		virtual QVariant GetMetaInfo (DataField) const = 0;

		virtual QList<QPair<QString, QVariant>> GetVCardRepresentation () const = 0;
	protected:
		virtual void vcardUpdated () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IMetaInfoEntry, "org.Deviant.LeechCraft.Azoth.IMetaInfoEntry/1.0")
