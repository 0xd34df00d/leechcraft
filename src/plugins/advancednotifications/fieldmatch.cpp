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

#include "fieldmatch.h"
#include <QtDebug>
#include "typedmatchers.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	FieldMatch::FieldMatch ()
	: FieldType_ (QVariant::Invalid)
	{
	}

	FieldMatch::FieldMatch (QVariant::Type type)
	: FieldType_ (type)
	, Matcher_ (TypedMatcherBase::Create (type))
	{
	}

	FieldMatch::FieldMatch (QVariant::Type type,
			TypedMatcherBase_ptr matcher)
	: FieldType_ (type)
	, Matcher_ (matcher)
	{
	}

	QString FieldMatch::GetPluginID () const
	{
		return PluginID_;
	}

	void FieldMatch::SetPluginID (const QString& id)
	{
		PluginID_ = id;
	}

	QString FieldMatch::GetFieldName () const
	{
		return FieldName_;
	}

	void FieldMatch::SetFieldName (const QString& name)
	{
		FieldName_ = name;
	}

	QVariant::Type FieldMatch::GetType () const
	{
		return FieldType_;
	}

	void FieldMatch::SetType (QVariant::Type type)
	{
		FieldType_ = type;
		Matcher_ = TypedMatcherBase::Create (type);
	}

	TypedMatcherBase_ptr FieldMatch::GetMatcher () const
	{
		return Matcher_;
	}

	void FieldMatch::Save (QDataStream& out) const
	{
		out << static_cast<quint8> (1)
				<< PluginID_
				<< FieldName_
				<< FieldType_
				<< (Matcher_ ? Matcher_->Save () : QVariantMap ());
	}

	void FieldMatch::Load (QDataStream& in)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return;
		}

		QVariantMap map;
		in >> PluginID_
			>> FieldName_
			>> FieldType_
			>> map;
		Matcher_ = TypedMatcherBase::Create (FieldType_);
		if (Matcher_)
			Matcher_->Load (map);
	}

	bool operator== (const FieldMatch& f1, const FieldMatch& f2)
	{
		return f1.GetType () == f2.GetType () &&
			f1.GetPluginID () == f2.GetPluginID () &&
			f1.GetFieldName () == f2.GetFieldName ();
	}
}
}
