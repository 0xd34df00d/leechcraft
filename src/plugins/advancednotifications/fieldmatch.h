/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QMetaType>
#include <QString>

namespace LC::AdvancedNotifications
{
	class TypedMatcherBase;

	using TypedMatcherBase_ptr = std::shared_ptr<TypedMatcherBase>;

	class FieldMatch
	{
		QString PluginID_;
		QString FieldName_;

		QMetaType::Type FieldType_ = QMetaType::UnknownType;

		TypedMatcherBase_ptr Matcher_;
	public:
		FieldMatch () = default;
		explicit FieldMatch (QMetaType::Type);
		FieldMatch (QMetaType::Type, TypedMatcherBase_ptr);

		QString GetPluginID () const;
		void SetPluginID (const QString&);

		QString GetFieldName () const;
		void SetFieldName (const QString&);

		QMetaType::Type GetType () const;
		void SetType (QMetaType::Type);

		TypedMatcherBase_ptr GetMatcher () const;

		void Save (QDataStream&) const;
		void Load (QDataStream&);
	};

	bool operator== (const FieldMatch&, const FieldMatch&);

	using FieldMatches_t = QList<FieldMatch>;
}
