/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fieldmatch.h"
#include <QDataStream>
#include <QtDebug>
#include "typedmatchers.h"

namespace LC::AdvancedNotifications
{
	FieldMatch::FieldMatch (QMetaType::Type type)
	: FieldType_ (type)
	, Matcher_ (TypedMatcherBase::Create (type))
	{
	}

	FieldMatch::FieldMatch (QMetaType::Type type, TypedMatcherBase_ptr matcher)
	: FieldType_ (type)
	, Matcher_ (std::move (matcher))
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

	QMetaType::Type FieldMatch::GetType () const
	{
		return FieldType_;
	}

	void FieldMatch::SetType (QMetaType::Type type)
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
