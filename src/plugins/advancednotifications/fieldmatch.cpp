/**********************************************************************
 *
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fieldmatch.h"
#include <QDataStream>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/anutil.h>

namespace LC::AdvancedNotifications
{
	namespace
	{
		namespace Keys
		{
			const QString IsSet = "IsSet"_qs;

			const QString Boundary = "Bd"_qs;
			const QString Ops = "Ops"_qs;

			const QString Rx = "Rx"_qs;
			const QString Contains = "Cont"_qs;
		}

		QVariantMap ToMap (const AN::ValueMatcher& matcher)
		{
			return Util::Visit (matcher,
				[] (const AN::BoolValueMatcher& bm) { return QVariantMap { { Keys::IsSet, bm.Value_ } }; },
				[] (const AN::IntValueMatcher& im)
				{
					return QVariantMap
					{
						{ Keys::Boundary, im.Boundary_ },
						{ Keys::Ops, static_cast<quint16> (im.Ops_) },
					};
				},
				[] (const AN::StringValueMatcher& sm)
				{
					return QVariantMap
					{
						{ Keys::Rx, Util::AN::ToVariant (sm.Pattern_) },
						{ Keys::Contains, sm.Positive_ }
					};
				});
		}

		template<typename>
		struct TypeWitness {};

		template<typename T>
		TypeWitness<T> Type = {};

		std::optional<AN::ValueMatcher> FromMap (QMetaType::Type type, const QVariantMap& map)
		{
			struct InvalidKey : std::exception {};

			auto key = [&map]<typename T> (TypeWitness<T>, const QString& key)
			{
				const auto pos = map.find (key);
				if (pos == map.end () || !pos->canConvert<T> ())
				{
					qWarning () << "invalid key" << key << "in" << map;
					throw InvalidKey {};
				}
				return pos->value<T> ();
			};

			try
			{
				switch (type)
				{
				case QMetaType::Bool:
					return AN::BoolValueMatcher { key (Type<bool>, Keys::IsSet) };
				case QMetaType::Int:
					return AN::IntValueMatcher
					{
						key (Type<int>, Keys::Boundary),
						static_cast<AN::IntValueMatcher::Operations> (key (Type<quint16>, Keys::Ops))
					};
				case QMetaType::QString:
				case QMetaType::QStringList:
				case QMetaType::QUrl:
				{
					const auto positive = key (Type<bool>, Keys::Contains);
					return Util::AN::StringPatternFromVariant (map.value (Keys::Rx))
							.transform ([&] (const auto& pattern) { return AN::StringValueMatcher { pattern, positive }; });
				}
				default:
					qWarning () << "unknown type" << type;
					return {};
				}
			}
			catch (const InvalidKey&)
			{
				return {};
			}
		}
	}

	void FieldMatch::Save (QDataStream& out) const
	{
		out << static_cast<quint8> (1)
				<< PluginID_
				<< Name_
				<< Type_;
		Util::Visit (Matcher_,
				[&out] (const QVariantMap& map) { out << map; },
				[&out] (const AN::ValueMatcher& matcher) { out << ToMap (matcher); });
	}

	std::optional<FieldMatch> FieldMatch::Load (QDataStream& in)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << "unknown version" << version;
			return {};
		}

		FieldMatch m;
		QVariantMap map;
		in >> m.PluginID_
			>> m.Name_
			>> m.Type_
			>> map;
		m.Matcher_ = FromMap (m.Type_, map)
				.transform ([] (const AN::ValueMatcher& m) { return ValueMatcherOrData { m }; })
				.value_or (map);
		return m;
	}
}
