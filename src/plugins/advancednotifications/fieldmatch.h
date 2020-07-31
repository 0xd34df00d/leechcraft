/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_FIELDMATCH_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_FIELDMATCH_H
#include <memory>
#include <QString>
#include <QVariant>

namespace LC
{
namespace AdvancedNotifications
{
	class TypedMatcherBase;

	typedef std::shared_ptr<TypedMatcherBase> TypedMatcherBase_ptr;

	class FieldMatch
	{
		QString PluginID_;
		QString FieldName_;

		QVariant::Type FieldType_;

		TypedMatcherBase_ptr Matcher_;
	public:
		FieldMatch ();
		FieldMatch (QVariant::Type);
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

	typedef QList<FieldMatch> FieldMatches_t;
}
}

#endif
