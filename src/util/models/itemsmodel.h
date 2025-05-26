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
			explicit Extension (auto&&...) {}

			static Qt::ItemFlags GetFlags (auto&&...) { return {}; }
			static bool SetData (auto&&...) { return false; }
			static void GetDataForRole () {}
		};

		template<typename P>
		struct ParameterizedExtension : Extension
		{
			struct Param
			{
				P Param_;
			};

			P Param_;

			template<typename... Params>
				requires (std::is_same_v<Param, Params> || ...)
			explicit ParameterizedExtension (const std::tuple<Params...>& args)
			: Param_ { std::get<Param> (args).Param_ }
			{
			}
		};
	}

	template<auto IconField>
	struct ItemsDecorated : detail::Extension
	{
		using Extension::Extension;

		static QVariant GetDataForRole (detail::Role<Qt::DecorationRole>, const auto& item, int column)
		{
			if (column)
				return {};
			return item.*IconField;
		}
	};

	template<auto CheckField>
	struct ItemsCheckable : detail::Extension
	{
		using Extension::Extension;

		static Qt::ItemFlags GetFlags (int column)
		{
			return column ? Qt::ItemFlags {} : Qt::ItemIsUserCheckable;
		}

		static QVariant GetDataForRole (detail::Role<Qt::CheckStateRole>, const auto& item, int, int column)
		{
			if (column)
				return QVariant {};

			if constexpr (std::is_same_v<std::decay_t<decltype (item.*CheckField)>, Qt::CheckState>)
				return item.*CheckField;
			else if constexpr (std::is_same_v<std::decay_t<decltype (item.*CheckField)>, bool>)
				return item.*CheckField ? Qt::Checked : Qt::Unchecked;
			else
				static_assert (false, "expected Qt::CheckState or bool field");
		}

		template<typename Item>
		static bool SetData (Item& item, int, int column, const QVariant& value, int role)
		{
			if (role != Qt::CheckStateRole || column)
				return false;

			const auto state = value.value<Qt::CheckState> ();
			if constexpr (std::is_same_v<std::decay_t<decltype (item.*CheckField)>, Qt::CheckState>)
				item.*CheckField = state;
			else if constexpr (std::is_same_v<std::decay_t<decltype (item.*CheckField)>, bool>)
				item.*CheckField = state == Qt::Checked;
			else
				static_assert (false, "expected Qt::CheckState or bool field");
			return true;
		}
	};

	struct NoField {};
	inline constexpr auto NoField_v = NoField {};

	template<>
	struct ItemsCheckable<NoField_v> : detail::ParameterizedExtension<Qt::CheckState>
	{
		QHash<int, Qt::CheckState> RowsStates_;

		using ParameterizedExtension::ParameterizedExtension;

		static Qt::ItemFlags GetFlags (int column)
		{
			return column ? Qt::ItemFlags {} : Qt::ItemIsUserCheckable;
		}

		bool IsChecked (int row) const
		{
			return RowsStates_.value (row, Param_) == Qt::Checked;
		}

		QVariant GetDataForRole (detail::Role<Qt::CheckStateRole>, const auto&, int row, int column)
		{
			return column ? QVariant {} : RowsStates_.value (row, Param_);
		}

		template<typename Item>
		bool SetData (Item&, int row, int column, const QVariant& value, int role)
		{
			if (role != Qt::CheckStateRole || column)
				return false;

			RowsStates_ [row] = value.value<Qt::CheckState> ();
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
		: ItemsModel { std::tuple {}, fields... }
		{
		}

		template<auto... Getter, typename... ExtParams>
		explicit ItemsModel (const std::tuple<ExtParams...>& extParams, const Field<Getter>&... fields)
		: FlatItemsModelTypedBase<T> { { fields.Name_... } }
		, Extensions { extParams }...
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
			const auto result = (Extensions::SetData (item, index.row (), index.column (), value, role) ||...);
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
				return this->GetDataForRole (detail::Role<Qt::CheckStateRole> {}, item, row, column);
			case Qt::DecorationRole:
				return this->GetDataForRole (detail::Role<Qt::DecorationRole> {}, item, row, column);
			default:
				return {};
			}
		}

		using Extensions::GetDataForRole...;

		static QVariant GetDataForRole (auto&&...)
		{
			return {};
		}
	};
}
