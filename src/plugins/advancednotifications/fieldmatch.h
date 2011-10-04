/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_FIELDMATCH_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_FIELDMATCH_H
#include <boost/shared_ptr.hpp>
#include <QString>
#include <QVariant>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class TypedMatcherBase;

	typedef boost::shared_ptr<TypedMatcherBase> TypedMatcherBase_ptr;

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
