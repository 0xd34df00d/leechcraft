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
		explicit RoledItemsModel (QObject *parent, Fields...) noexcept
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

	template<auto Getter>
	struct Field
	{
		QString Name_;
	};

	namespace detail
	{
		template<Qt::ItemDataRole Role>
		using Role = std::integral_constant<Qt::ItemDataRole, Role>;

		struct Extension
		{
			static Qt::ItemFlags GetFlags (auto&&...) { return {}; }
			static void GetDataForRole () {}
			static bool SetData (auto&&...) { return false; }
		};
	}

	template<auto CheckField>
	struct ItemsCheckable : detail::Extension
	{
		static Qt::ItemFlags GetFlags (int column)
		{
			return column ? Qt::ItemFlags {} : Qt::ItemIsUserCheckable;
		}

		template<typename Item>
		static QVariant GetDataForRole (detail::Role<Qt::CheckStateRole>, const Item& item, int column)
		{
			static_assert (std::is_same_v<std::decay_t<decltype (item.*CheckField)>, Qt::CheckState>);
			return column ? QVariant {} : item.*CheckField;
		}

		template<typename Item>
		static bool SetData (Item& item, int column, const QVariant& value, int role)
		{
			if (role != Qt::CheckStateRole || column)
				return false;

			item.*CheckField = value.value<Qt::CheckState> ();
			return true;
		}
	};

	template<typename T, typename... Extensions>
	class ItemsModel : public FlatItemsModelTypedBase<T>
					 , public Extensions...
	{
		using FieldGetter_t = QVariant (*) (const T&);
		const QVector<FieldGetter_t> Fields_;
	public:
		template<auto... Getter>
		explicit ItemsModel (const Field<Getter>&... fields)
		: FlatItemsModelTypedBase<T> { { fields.Name_... } }
		, Fields_ { +[] (const T& t) -> QVariant { return t.*Getter; }... }
		{
		}

		Qt::ItemFlags flags (const QModelIndex& index) const override
		{
			auto flags = FlatItemsModelTypedBase<T>::flags (index);
			flags |= (Extensions::GetFlags (index.column ()) | ...);
			return flags;
		}

		bool setData (const QModelIndex& index, const QVariant& value, int role) override
		{
			auto& item = this->Items_ [index.row ()];
			const auto result = (Extensions::SetData (item, index.column (), value, role) ||...);
			if (result)
				emit this->dataChanged (index, index);
			return result;
		}
	protected:
		QVariant GetData (int row, int column, int role) const override
		{
			const auto& item = this->Items_ [row];

			switch (role)
			{
			case Qt::DisplayRole:
				if (const auto getter = Fields_.value (column))
					return getter (item);
				return {};
			case Qt::CheckStateRole:
				return this->GetDataForRole (detail::Role<Qt::CheckStateRole> {}, item, column);
			case Qt::DecorationRole:
				return this->GetDataForRole (detail::Role<Qt::DecorationRole> {}, item, column);
			default:
				return {};
			}
		}

		using Extensions::GetDataForRole...;

		static QVariant GetDataForRole (auto, const auto&, int)
		{
			return {};
		}
	};
}
