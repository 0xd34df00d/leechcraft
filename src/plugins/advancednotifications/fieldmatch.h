/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QString>
#include <QVariant>

namespace LC::AdvancedNotifications
{
	class TypedMatcherBase;

	using TypedMatcherBase_ptr = std::shared_ptr<TypedMatcherBase>;

	class FieldMatch
	{
		QString PluginID_;
		QString FieldName_;

		QVariant::Type FieldType_ = QVariant::Invalid;

		TypedMatcherBase_ptr Matcher_;
	public:
		FieldMatch () = default;
		explicit FieldMatch (QVariant::Type);
		FieldMatch (QVariant::Type, TypedMatcherBase_ptr);

		QString GetPluginID () const;
		void SetPluginID (const QString&);

		QString GetFieldName () const;
		void SetFieldName (const QString&);

		QVariant::Type GetType () const;
		void SetType (QVariant::Type);

		TypedMatcherBase_ptr GetMatcher () const;

		void Save (QDataStream&) const;
		void Load (QDataStream&);
	};

	bool operator== (const FieldMatch&, const FieldMatch&);

	using FieldMatches_t = QList<FieldMatch>;
}
