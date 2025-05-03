/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/ctstring.h>
#include "flatitemsmodeltypedbase.h"

namespace LC::Util
{
	template<CtString RoleArg, auto GetterArg>
	struct RoledMemberField
	{
		static constexpr auto Getter = GetterArg;
		static constexpr auto Role = RoleArg;
	};

	template<CtString RoleArg, auto GetterArg>
	RoledMemberField<RoleArg, GetterArg> RoledMemberField_v;

	template<typename T>
	class RoledItemsModel : public FlatItemsModelTypedBase<T>
	{
	public:
		using FieldGetter_t = QVariant (*) (const T&);
		using FieldsList_t = QVector<QPair<QByteArray, FieldGetter_t>>;
	private:
		const QVector<FieldGetter_t> Fields_;
		const QHash<int, QByteArray> Roles_;
	public:
		template<typename... Fields>
		RoledItemsModel (QObject *parent, Fields...) noexcept
		: FlatItemsModelTypedBase<T> { QStringList { {} }, parent }
		, Fields_ { +[] (const T& t) -> QVariant { return t.*(Fields::Getter); }... }
		, Roles_ { MakeRoles ({ ToByteArray<Fields::Role> ()... }) }
		{
		}

		QHash<int, QByteArray> roleNames () const override
		{
			return Roles_;
		}
	protected:
		QVariant GetData (int row, int, int role) const override
		{
			if (const auto getter = Fields_.value (role - this->DataRole - 1))
				return getter (this->Items_.at (row));
			return {};
		}
	private:
		QHash<int, QByteArray> MakeRoles (QVector<QByteArray> fields) const
		{
			auto result = FlatItemsModelTypedBase<T>::roleNames ();
			result.reserve (result.size () + fields.size ());
			for (int i = 0; i < fields.size (); ++i)
				result [this->DataRole + i + 1] = std::move (fields [i]);
			return result;
		}
	};
}
