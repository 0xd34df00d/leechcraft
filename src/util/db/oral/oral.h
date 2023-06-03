/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <type_traits>
#include <memory>
#include <optional>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDateTime>
#include <QtDebug>
#include <util/sll/ctstringutils.h>
#include <util/sll/prelude.h>
#include <util/sll/typelist.h>
#include <util/sll/typegetter.h>
#include <util/sll/detector.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include "oraltypes.h"
#include "sqliteimpl.h"

#ifndef ORAL_ADAPT_STRUCT

#define ORAL_STRING_FIELD(_, index, tuple)																			\
	if constexpr (Idx == index)																						\
		return CtString { BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(index, tuple)) };

#define ORAL_GET_FIELD(_, index, tuple)																				\
	if constexpr (Idx == index)																						\
		return s.BOOST_PP_TUPLE_ELEM(index, tuple);

#define ORAL_GET_FIELD_INDEX(_, index, args)																		\
	template<>																										\
	constexpr size_t FieldIndex<&BOOST_PP_TUPLE_ELEM(0, args)::BOOST_PP_TUPLE_ELEM(index, BOOST_PP_TUPLE_ELEM(1, args))> () \
	{																												\
		return index;																								\
	}

#define ORAL_ADAPT_STRUCT(sname, ...)																				\
namespace LC::Util::oral																							\
{																													\
	template<>																										\
	constexpr auto SeqSize<sname> = BOOST_PP_TUPLE_SIZE((__VA_ARGS__));												\
																													\
	template<>																										\
	struct MemberNames<sname>																						\
	{																												\
		template<size_t Idx>																						\
		constexpr static auto Get ()																				\
		{																											\
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), ORAL_STRING_FIELD, (__VA_ARGS__))					\
		}																											\
	};																												\
																													\
	template<>																										\
	struct FieldAccess<sname>																						\
	{																												\
		template<size_t Idx>																						\
		constexpr static const auto& Get (const sname& s)															\
		{																											\
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), ORAL_GET_FIELD, (__VA_ARGS__))						\
		}																											\
																													\
		template<size_t Idx>																						\
		constexpr static auto& Get (sname& s)																		\
		{																											\
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), ORAL_GET_FIELD, (__VA_ARGS__))						\
		}																											\
	};																												\
																													\
	template<>																										\
	struct FieldIndexAccess<sname>																					\
	{																												\
		template<auto Ptr>																							\
		constexpr static size_t FieldIndex ();																		\
																													\
		BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), ORAL_GET_FIELD_INDEX, (sname, (__VA_ARGS__)))			\
	};																												\
}																													\

#endif

namespace LC::Util::oral
{
	using QSqlQuery_ptr = std::shared_ptr<QSqlQuery>;

	class QueryException : public std::runtime_error
	{
		const QSqlQuery Query_;
	public:
		QueryException (const std::string& str, const QSqlQuery_ptr& q)
		: QueryException { str, *q }
		{
		}

		QueryException (const std::string& str, const QSqlQuery& q)
		: std::runtime_error { str }
		, Query_ { q }
		{
		}

		~QueryException () noexcept = default;

		const QSqlQuery& GetQuery () const
		{
			return Query_;
		}
	};

	template<typename>
	constexpr size_t SeqSize = -1;

	template<typename>
	struct MemberNames {};

	template<typename>
	struct FieldAccess {};

	template<typename>
	struct FieldIndexAccess {};

	namespace detail
	{
		template<size_t Idx, typename Seq>
		constexpr decltype (auto) Get (const Seq& seq)
		{
			return FieldAccess<Seq>::template Get<Idx> (seq);
		}

		template<size_t Idx, typename Seq>
		constexpr decltype (auto) Get (Seq& seq)
		{
			return FieldAccess<Seq>::template Get<Idx> (seq);
		}

		template<typename T, CtString str>
		consteval auto MorphFieldName ()
		{
			if constexpr (requires { T::template FieldNameMorpher<str> (); })
				return T::template FieldNameMorpher<str> ();
			else if constexpr (str.EndsWith ('_'))
				return str.template Chop<1> ();
			else
				return str;
		}

		template<typename Seq, int Idx>
		consteval auto GetFieldName ()
		{
			constexpr auto str = MemberNames<Seq>::template Get<Idx> ();
			return MorphFieldName<Seq, str> ();
		}

		template<typename S>
		constexpr auto SeqIndices = std::make_index_sequence<SeqSize<S>> {};

		template<typename S>
		constexpr auto FieldNames = []<size_t... Ix> (std::index_sequence<Ix...>) constexpr
			{
				return std::tuple { GetFieldName<S, Ix> ()... };
			} (SeqIndices<S>);

		template<typename S>
		constexpr auto BoundFieldNames = []<size_t... Ix> (std::index_sequence<Ix...>) constexpr
			{
				return std::tuple { (":" + GetFieldName<S, Ix> ())... };
			} (SeqIndices<S>);

		template<typename S>
		constexpr auto QualifiedFieldNames = []<size_t... Ix> (std::index_sequence<Ix...>) constexpr
			{
				return std::tuple { (S::ClassName + "." + GetFieldName<S, Ix> ())... };
			} (SeqIndices<S>);

		template<auto Ptr>
		constexpr size_t FieldIndex () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;
			return FieldIndexAccess<S>::template FieldIndex<Ptr> ();
		}

		template<auto Ptr>
		constexpr auto GetFieldNamePtr () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;
			return GetFieldName<S, FieldIndex<Ptr> ()> ();
		}

		template<auto Ptr>
		constexpr auto GetQualifiedFieldNamePtr () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;
			return S::ClassName + "." + GetFieldName<S, FieldIndex<Ptr> ()> ();
		}

		template<typename T>
		concept TypeNameCustomized = requires { typename T::TypeName; };

		template<typename T>
		concept BaseTypeCustomized = requires { typename T::BaseType; };
	}

	template<typename ImplFactory, typename T, typename = void>
	struct Type2Name
	{
		constexpr auto operator() () const noexcept
		{
			if constexpr (HasType<T> (Typelist<int, qlonglong, qulonglong, bool> {}) || std::is_enum_v<T>)
				return "INTEGER"_ct;
			else if constexpr (std::is_same_v<T, double>)
				return "REAL"_ct;
			else if constexpr (std::is_same_v<T, QString> || std::is_same_v<T, QDateTime> || std::is_same_v<T, QUrl>)
				return "TEXT"_ct;
			else if constexpr (std::is_same_v<T, QByteArray>)
				return ImplFactory::TypeLits::Binary;
			else if constexpr (detail::TypeNameCustomized<T>)
				return T::TypeName;
			else if constexpr (detail::BaseTypeCustomized<T>)
				return Type2Name<ImplFactory, typename T::BaseType> {} ();
			else
				static_assert (std::is_same_v<T, struct Dummy>, "Unsupported type");
		}
	};

	template<typename ImplFactory, typename T>
	struct Type2Name<ImplFactory, Unique<T>>
	{
		constexpr auto operator() () const noexcept { return Type2Name<ImplFactory, T> {} () + " UNIQUE"; }
	};

	template<typename ImplFactory, typename T>
	struct Type2Name<ImplFactory, NotNull<T>>
	{
		constexpr auto operator() () const noexcept { return Type2Name<ImplFactory, T> {} () + " NOT NULL"; }
	};

	template<typename ImplFactory, typename T, typename... Tags>
	struct Type2Name<ImplFactory, PKey<T, Tags...>>
	{
		constexpr auto operator() () const noexcept { return Type2Name<ImplFactory, T> {} () + " PRIMARY KEY"; }
	};

	template<typename ImplFactory, typename... Tags>
	struct Type2Name<ImplFactory, PKey<int, Tags...>>
	{
		constexpr auto operator() () const noexcept { return ImplFactory::TypeLits::IntAutoincrement; }
	};

	template<typename ImplFactory, auto Ptr>
	struct Type2Name<ImplFactory, References<Ptr>>
	{
		constexpr auto operator() () const noexcept
		{
			constexpr auto className = MemberPtrStruct_t<Ptr>::ClassName;
			return Type2Name<ImplFactory, ReferencesValue_t<Ptr>> () () +
					" REFERENCES " + className + " (" + detail::GetFieldNamePtr<Ptr> () + ") ON DELETE CASCADE";
		}
	};

	template<typename T, typename = void>
	struct ToVariant
	{
		QVariant operator() (const T& t) const noexcept
		{
			if constexpr (std::is_same_v<T, QDateTime>)
				return t.toString (Qt::ISODate);
			else if constexpr (std::is_enum_v<T>)
				return static_cast<qint64> (t);
			else if constexpr (IsIndirect<T> {})
				return ToVariant<typename T::value_type> {} (t);
			else if constexpr (detail::TypeNameCustomized<T>)
				return t.ToVariant ();
			else if constexpr (detail::BaseTypeCustomized<T>)
				return ToVariant<typename T::BaseType> {} (t.ToBaseType ());
			else
				return t;
		}
	};

	template<typename T, typename = void>
	struct FromVariant
	{
		T operator() (const QVariant& var) const noexcept
		{
			if constexpr (std::is_same_v<T, QDateTime>)
				return QDateTime::fromString (var.toString (), Qt::ISODate);
			else if constexpr (std::is_enum_v<T>)
				return static_cast<T> (var.value<qint64> ());
			else if constexpr (IsIndirect<T> {})
				return FromVariant<typename T::value_type> {} (var);
			else if constexpr (detail::TypeNameCustomized<T>)
				return T::FromVariant (var);
			else if constexpr (detail::BaseTypeCustomized<T>)
				return T::FromBaseType (FromVariant<typename T::BaseType> {} (var));
			else
				return var.value<T> ();
		}
	};

	namespace detail
	{
		template<typename Seq, int Idx>
		using ValueAtC_t = std::decay_t<decltype (Get<Idx> (std::declval<Seq> ()))>;

		template<typename T>
		struct IsPKey : std::false_type {};

		template<typename U, typename... Tags>
		struct IsPKey<PKey<U, Tags...>> : std::true_type {};

		template<typename T>
		QVariant ToVariantF (const T& t) noexcept
		{
			return ToVariant<T> {} (t);
		}

		template<size_t Ix, typename Seq>
		void BindAtIndex (const Seq& seq, QSqlQuery& query, bool bindPrimaryKey)
		{
			const auto& value = Get<Ix> (seq);
			if (bindPrimaryKey || !IsPKey<ValueAtC_t<Seq, Ix>>::value)
				query.bindValue (std::get<Ix> (BoundFieldNames<Seq>).ToString (), ToVariantF (value));
		}

		template<typename Seq>
		auto DoInsert (const Seq& seq, QSqlQuery& insertQuery, bool bindPrimaryKey)
		{
			[&]<size_t... Ix> (std::index_sequence<Ix...>)
			{
				(BindAtIndex<Ix> (seq, insertQuery, bindPrimaryKey), ...);
			} (SeqIndices<Seq>);

			if (!insertQuery.exec ())
			{
				qCritical () << "insert query execution failed";
				DBLock::DumpError (insertQuery);
				throw QueryException ("insert query execution failed", insertQuery);
			}
		}

		template<typename Seq>
		consteval int PKeyIndexUnsafe ()
		{
			auto run = []<size_t... Idxes> (std::index_sequence<Idxes...>)
			{
				int result = -1;
				((IsPKey<ValueAtC_t<Seq, Idxes>>::value ? (result = Idxes) : 0), ...);
				return result;
			};
			return run (SeqIndices<Seq>);
		}

		template<typename Seq>
		consteval int PKeyIndex ()
		{
			const auto idx = PKeyIndexUnsafe<Seq> ();
			static_assert (idx >= 0);
			return idx;
		}

		template<typename Seq>
		constexpr int PKeyIndex_v = PKeyIndex<Seq> ();

		template<typename Seq>
		concept HasPKey = PKeyIndexUnsafe<Seq> () >= 0;

		template<typename Seq>
		constexpr auto HasAutogenPKey () noexcept
		{
			if constexpr (HasPKey<Seq>)
				return !HasType<NoAutogen> (AsTypelist_t<ValueAtC_t<Seq, PKeyIndex_v<Seq>>> {});
			else
				return false;
		}

		template<typename Seq>
		constexpr auto ExtractConflictingFields (InsertAction::Replace::PKeyType)
		{
			return std::tuple { detail::GetFieldName<Seq, PKeyIndex_v<Seq>> () };
		}

		template<typename Seq, auto... Ptrs>
		constexpr auto ExtractConflictingFields (InsertAction::Replace::FieldsType<Ptrs...>)
		{
			return std::tuple { std::get<FieldIndex<Ptrs> ()> (FieldNames<Seq>)... };
		}

		template<typename Seq>
		class AdaptInsert
		{
			const QSqlDatabase DB_;
			constexpr static bool HasAutogen_ = HasAutogenPKey<Seq> ();
		public:
			AdaptInsert (const QSqlDatabase& db) noexcept
			: DB_ { db }
			{
			}

			template<typename Action = InsertAction::DefaultTag>
			auto operator() (Seq& t, Action action = {}) const
			{
				return Run<SQLite::ImplFactory> (t, action);
			}

			template<typename ImplFactory>
			auto operator() (ImplFactory, Seq& t, auto action) const
			{
				return Run<ImplFactory> (t, action);
			}

			template<typename Action = InsertAction::DefaultTag>
			auto operator() (const Seq& t, Action action = {}) const
			{
				return Run<SQLite::ImplFactory> (t, action);
			}

			template<typename ImplFactory>
			auto operator() (ImplFactory, const Seq& t, auto action) const
			{
				return Run<ImplFactory> (t, action);
			}
		private:
			template<typename ImplFactory, typename Action>
			constexpr static auto MakeInsertSuffix (Action action)
			{
				if constexpr (std::is_same_v<Action, InsertAction::DefaultTag> || std::is_same_v<Action, InsertAction::IgnoreTag>)
					return ImplFactory::GetInsertSuffix (action);
				else
					return ImplFactory::GetInsertSuffix (InsertAction::Replace {},
							ExtractConflictingFields<Seq> (action),
							FieldNames<Seq>);
			}

			template<typename ImplFactory>
			constexpr static auto MakeQueryForAction (auto action)
			{
				return ImplFactory::GetInsertPrefix (action) +
						" INTO " + Seq::ClassName +
						" (" + JoinTup (FieldNames<Seq>, ", ") + ") " +
						"VALUES (" + JoinTup (BoundFieldNames<Seq>, ", ") + ") " +
						MakeInsertSuffix<ImplFactory> (action);
			}

			template<typename ImplFactory, typename T>
			auto Run (T& t, auto action) const
			{
				QSqlQuery query { DB_ };
				constexpr auto queryText = MakeQueryForAction<ImplFactory> (action);
				query.prepare (queryText.ToString ());

				DoInsert (t, query, !HasAutogen_);

				if constexpr (HasAutogen_)
				{
					constexpr auto index = PKeyIndex_v<Seq>;

					const auto& lastId = FromVariant<ValueAtC_t<Seq, index>> {} (query.lastInsertId ());
					if constexpr (!std::is_const_v<T>)
						Get<index> (t) = lastId;
					else
						return lastId;
				}
			}
		};

		template<typename Seq>
		struct AdaptDelete
		{
			QSqlQuery DeleteQuery_;
		public:
			AdaptDelete (const QSqlDatabase& db) noexcept
			: DeleteQuery_ { db }
			{
				if constexpr (HasPKey<Seq>)
				{
					constexpr auto index = PKeyIndex_v<Seq>;
					constexpr auto del = "DELETE FROM " + Seq::ClassName +
							" WHERE " + std::get<index> (FieldNames<Seq>) + " = ?";
					DeleteQuery_.prepare (del.ToString ());
				}
			}

			void operator() (const Seq& seq) requires HasPKey<Seq>
			{
				constexpr auto index = PKeyIndex_v<Seq>;
				DeleteQuery_.bindValue (0, ToVariantF (Get<index> (seq)));
				if (!DeleteQuery_.exec ())
					throw QueryException ("delete query execution failed", DeleteQuery_);
			}
		};

		template<typename T, size_t... Indices>
		T InitializeFromQuery (const QSqlQuery& q, std::index_sequence<Indices...>, int startIdx) noexcept
		{
			if constexpr (requires { T { FromVariant<ValueAtC_t<T, Indices>> {} (QVariant {})... }; })
				return T { FromVariant<ValueAtC_t<T, Indices>> {} (q.value (startIdx + Indices))... };
			else
			{
				T t;
				((Get<Indices> (t) = FromVariant<ValueAtC_t<T, Indices>> {} (q.value (startIdx + Indices))), ...);
				return t;
			}
		}

		enum class ExprType
		{
			ConstTrue,

			LeafStaticPlaceholder,
			LeafData,

			Greater,
			Less,
			Equal,
			Geq,
			Leq,
			Neq,

			Like,

			And,
			Or
		};

		template<ExprType Type>
		constexpr auto TypeToSql () noexcept
		{
			if constexpr (Type == ExprType::Greater)
				return ">"_ct;
			else if constexpr (Type == ExprType::Less)
				return "<"_ct;
			else if constexpr (Type == ExprType::Equal)
				return "="_ct;
			else if constexpr (Type == ExprType::Geq)
				return ">="_ct;
			else if constexpr (Type == ExprType::Leq)
				return "<="_ct;
			else if constexpr (Type == ExprType::Neq)
				return "!="_ct;
			else if constexpr (Type == ExprType::Like)
				return "LIKE"_ct;
			else if constexpr (Type == ExprType::And)
				return "AND"_ct;
			else if constexpr (Type == ExprType::Or)
				return "OR"_ct;
			else
				static_assert (std::is_same_v<struct D1, ExprType>, "Invalid expression type");
		}

		constexpr bool IsRelational (ExprType type) noexcept
		{
			return type == ExprType::Greater ||
					type == ExprType::Less ||
					type == ExprType::Equal ||
					type == ExprType::Geq ||
					type == ExprType::Leq ||
					type == ExprType::Neq ||
					type == ExprType::Like;
		}

		template<typename T>
		struct WrapDirect
		{
			using value_type = T;
		};

		template<typename T>
		using UnwrapIndirect_t = typename std::conditional_t<IsIndirect<T> {},
				T,
				WrapDirect<T>>::value_type;

		template<typename Seq, typename L, typename R>
		using ComparableDetector = decltype (std::declval<UnwrapIndirect_t<typename L::template ValueType_t<Seq>>> () ==
				std::declval<UnwrapIndirect_t<typename R::template ValueType_t<Seq>>> ());

		template<typename Seq, typename L, typename R>
		constexpr auto AreComparableTypes = IsDetected_v<ComparableDetector, Seq, L, R> || IsDetected_v<ComparableDetector, Seq, R, L>;

		template<typename Seq, typename L, typename R, typename = void>
		struct RelationalTypesCheckerBase : std::false_type {};

		template<typename Seq, typename L, typename R>
		struct RelationalTypesCheckerBase<Seq, L, R, std::enable_if_t<AreComparableTypes<Seq, L, R>>> : std::true_type {};

		template<ExprType Type, typename Seq, typename L, typename R, typename = void>
		struct RelationalTypesChecker : std::true_type {};

		template<ExprType Type, typename Seq, typename L, typename R>
		struct RelationalTypesChecker<Type, Seq, L, R, std::enable_if_t<IsRelational (Type)>> : RelationalTypesCheckerBase<Seq, L, R> {};

		template<ExprType Type, typename L = void, typename R = void>
		class ExprTree;

		template<typename T>
		struct IsExprTree : std::false_type {};

		template<ExprType Type, typename L, typename R>
		struct IsExprTree<ExprTree<Type, L, R>> : std::true_type {};

		template<typename L, typename R>
		class AssignList
		{
			L Left_;
			R Right_;
		public:
			AssignList (const L& l, const R& r) noexcept
			: Left_ { l }
			, Right_ { r }
			{
			}

			template<typename Seq, CtString S>
			constexpr static auto ToSql () noexcept
			{
				if constexpr (IsExprTree<L> {})
					return L::GetFieldName () + " = " + R::template ToSql<Seq, S + "r"> ();
				else
					return L::template ToSql<Seq, S + "l"> () + ", " + R::template ToSql<Seq, S + "r"> ();
			}

			template<typename Seq, CtString S>
			void BindValues (QSqlQuery& query) const noexcept
			{
				Left_.template BindValues<Seq, S + "l"> (query);
				Right_.template BindValues<Seq, S + "r"> (query);
			}

			template<typename OL, typename OR>
			constexpr auto operator, (const AssignList<OL, OR>& tail) noexcept
			{
				return AssignList<AssignList<L, R>, AssignList<OL, OR>> { *this, tail };
			}
		};

		template<ExprType Type, typename L, typename R>
		class ExprTree
		{
			L Left_;
			R Right_;
		public:
			ExprTree (const L& l, const R& r) noexcept
			: Left_ (l)
			, Right_ (r)
			{
			}

			template<typename Seq, CtString S>
			constexpr static auto ToSql () noexcept
			{
				static_assert (RelationalTypesChecker<Type, Seq, L, R>::value,
						"Incompatible types passed to a relational operator.");

				return L::template ToSql<Seq, S + "l"> () + " " + TypeToSql<Type> () + " " + R::template ToSql<Seq, S + "r"> ();
			}

			template<typename Seq, CtString S>
			void BindValues (QSqlQuery& query) const noexcept
			{
				Left_.template BindValues<Seq, S + "l"> (query);
				Right_.template BindValues<Seq, S + "r"> (query);
			}

			template<typename T>
			constexpr static auto AdditionalTables () noexcept
			{
				return std::tuple_cat (L::template AdditionalTables<T> (), R::template AdditionalTables<T> ());
			}

			template<typename T>
			constexpr static bool HasAdditionalTables () noexcept
			{
				return L::template HasAdditionalTables<T> () || R::template HasAdditionalTables<T> ();
			}
		};

		template<typename T>
		class ExprTree<ExprType::LeafData, T, void>
		{
			T Data_;
		public:
			template<typename>
			using ValueType_t = T;

			ExprTree (const T& t) noexcept
			: Data_ (t)
			{
			}

			template<typename, CtString S>
			constexpr static auto ToSql () noexcept
			{
				return ":bound_" + S;
			}

			template<typename Seq, CtString S>
			void BindValues (QSqlQuery& query) const noexcept
			{
				constexpr auto varName = ToSql<Seq, S> ();
				query.bindValue (varName.ToString (), ToVariantF (Data_));
			}

			template<typename>
			constexpr static auto AdditionalTables () noexcept
			{
				return std::tuple {};
			}

			template<typename>
			constexpr static bool HasAdditionalTables () noexcept
			{
				return false;
			}
		};

		template<typename T>
		constexpr auto AsLeafData (const T& node) noexcept
		{
			if constexpr (IsExprTree<T> {})
				return node;
			else
				return ExprTree<ExprType::LeafData, T> { node };
		}

		template<auto... Ptr>
		struct MemberPtrs {};

		template<auto Ptr>
		class ExprTree<ExprType::LeafStaticPlaceholder, MemberPtrs<Ptr>, void>
		{
			using ExpectedType_t = MemberPtrType_t<Ptr>;
		public:
			template<typename>
			using ValueType_t = ExpectedType_t;

			template<typename Seq, CtString S>
			constexpr static auto ToSql () noexcept
			{
				return MemberPtrStruct_t<Ptr>::ClassName + "." + GetFieldName ();
			}

			template<typename Seq, CtString S>
			void BindValues (QSqlQuery&) const noexcept
			{
			}

			constexpr static auto GetFieldName () noexcept
			{
				return detail::GetFieldNamePtr<Ptr> ();
			}

			template<typename T>
			constexpr static auto AdditionalTables () noexcept
			{
				using Seq = MemberPtrStruct_t<Ptr>;
				if constexpr (std::is_same_v<Seq, T>)
					return std::tuple {};
				else
					return std::tuple { Seq::ClassName };
			}

			template<typename T>
			constexpr static bool HasAdditionalTables () noexcept
			{
				return !std::is_same_v<MemberPtrStruct_t<Ptr>, T>;
			}

			constexpr auto operator= (const ExpectedType_t& r) const noexcept
			{
				return AssignList { *this, AsLeafData (r) };
			}
		};

		template<>
		class ExprTree<ExprType::ConstTrue, void, void>
		{
		public:
			template<typename, CtString>
			constexpr static auto ToSql () noexcept
			{
				return "1 = 1"_ct;
			}

			template<typename, CtString>
			void BindValues (QSqlQuery&) const noexcept
			{
			}

			template<typename>
			constexpr static bool HasAdditionalTables () noexcept
			{
				return false;
			}
		};

		constexpr auto ConstTrueTree_v = ExprTree<ExprType::ConstTrue> {};

		template<ExprType Type, typename L, typename R>
		auto MakeExprTree (const L& left, const R& right) noexcept
		{
			using EL = decltype (AsLeafData (left));
			using ER = decltype (AsLeafData (right));
			return ExprTree<Type, EL, ER> { AsLeafData (left), AsLeafData (right) };
		}

		template<typename L, typename R>
		constexpr bool EitherIsExprTree () noexcept
		{
			if (IsExprTree<L> {})
				return true;
			if (IsExprTree<R> {})
				return true;
			return false;
		}

		template<typename L, typename R>
		using EnableRelOp_t = std::enable_if_t<EitherIsExprTree<L, R> ()>;

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator< (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::Less> (left, right);
		}

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator> (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::Greater> (left, right);
		}

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator== (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::Equal> (left, right);
		}

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator!= (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::Neq> (left, right);
		}

		template<ExprType Op>
		struct InfixBinary {};
	}

	namespace infix
	{
		constexpr detail::InfixBinary<detail::ExprType::Like> like {};
	}

	namespace detail
	{
		template<typename L, ExprType Op>
		struct InfixBinaryProxy
		{
			const L& Left_;
		};

		template<typename L, ExprType Op>
		auto operator| (const L& left, InfixBinary<Op>) noexcept
		{
			return InfixBinaryProxy<L, Op> { left };
		}

		template<typename L, ExprType Op, typename R>
		auto operator| (const InfixBinaryProxy<L, Op>& left, const R& right) noexcept
		{
			return MakeExprTree<Op> (left.Left_, right);
		}

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator&& (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::And> (left, right);
		}

		template<typename L, typename R, typename = EnableRelOp_t<L, R>>
		auto operator|| (const L& left, const R& right) noexcept
		{
			return MakeExprTree<ExprType::Or> (left, right);
		}

		template<CtString BindPrefix, typename Seq, typename Tree>
		constexpr auto ExprTreeToSql () noexcept
		{
			return Tree::template ToSql<Seq, BindPrefix> ();
		}

		template<CtString BindPrefix, typename Seq, typename Tree>
		void BindExprTree (const Tree& tree, QSqlQuery& query)
		{
			tree.template BindValues<Seq, BindPrefix> (query);
		}

		enum class AggregateFunction
		{
			Count,
			Min,
			Max
		};

		template<AggregateFunction, auto Ptr>
		struct AggregateType {};

		struct CountAll {};

		inline constexpr CountAll *CountAllPtr = nullptr;

		template<typename... MemberDirectionList>
		struct OrderBy {};

		template<auto... Ptrs>
		struct GroupBy {};

		struct SelectWhole {};

		template<typename L, typename R>
		struct SelectorUnion {};

		template<typename T>
		struct IsSelector : std::false_type {};

		template<>
		struct IsSelector<SelectWhole> : std::true_type {};

		template<AggregateFunction Fun, auto Ptr>
		struct IsSelector<AggregateType<Fun, Ptr>> : std::true_type {};

		template<auto... Ptrs>
		struct IsSelector<MemberPtrs<Ptrs...>> : std::true_type {};

		template<typename L, typename R>
		struct IsSelector<SelectorUnion<L, R>> : std::true_type {};

		template<typename L, typename R, typename = std::enable_if_t<IsSelector<L> {} && IsSelector<R> {}>>
		SelectorUnion<L, R> operator+ (L, R) noexcept
		{
			return {};
		}
	}

	namespace sph
	{
		template<auto Ptr>
		constexpr detail::ExprTree<detail::ExprType::LeafStaticPlaceholder, detail::MemberPtrs<Ptr>> f {};

		template<auto... Ptrs>
		constexpr detail::MemberPtrs<Ptrs...> fields {};

		constexpr detail::SelectWhole all {};

		template<auto... Ptrs>
		struct asc {};

		template<auto... Ptrs>
		struct desc {};

		template<auto Ptr = detail::CountAllPtr>
		constexpr detail::AggregateType<detail::AggregateFunction::Count, Ptr> count {};

		template<auto Ptr>
		constexpr detail::AggregateType<detail::AggregateFunction::Min, Ptr> min {};

		template<auto Ptr>
		constexpr detail::AggregateType<detail::AggregateFunction::Max, Ptr> max {};
	};

	template<typename... Orders>
	constexpr detail::OrderBy<Orders...> OrderBy {};

	template<auto... Ptrs>
	constexpr detail::GroupBy<Ptrs...> GroupBy {};

	struct Limit
	{
		uint64_t Count;
	};

	struct Offset
	{
		uint64_t Count;
	};

	namespace detail
	{
		template<auto Ptr>
		auto MemberFromVariant (const QVariant& var) noexcept
		{
			return FromVariant<UnwrapIndirect_t<MemberPtrType_t<Ptr>>> {} (var);
		}

		template<auto Ptr>
		auto MakeIndexedQueryHandler (const QSqlQuery& q, int startIdx = 0) noexcept
		{
			return MemberFromVariant<Ptr> (q.value (startIdx));
		}

		template<auto... Ptrs>
		auto MakeIndexedQueryHandler (MemberPtrs<Ptrs...>, const QSqlQuery& q, int startIdx) noexcept
		{
			if constexpr (sizeof... (Ptrs) == 1)
				return MakeIndexedQueryHandler<Ptrs...> (q, startIdx);
			else
				return [&]<size_t... Ix> (std::index_sequence<Ix...>)
				{
					return std::tuple { MemberFromVariant<Ptrs> (q.value (startIdx + Ix))... };
				} (std::make_index_sequence<sizeof... (Ptrs)> {});
		}

		enum class SelectBehaviour { Some, One };

		struct OrderNone {};
		struct GroupNone {};
		struct LimitNone {};
		struct OffsetNone {};

		template<size_t RepIdx, size_t TupIdx, typename Tuple, typename NewType>
		constexpr decltype (auto) GetReplaceTupleElem (Tuple&& tuple, NewType&& arg) noexcept
		{
			if constexpr (RepIdx == TupIdx)
				return std::forward<NewType> (arg);
			else
				return std::get<TupIdx> (tuple);
		}

		template<size_t RepIdx, typename NewType, typename Tuple, size_t... TupIdxs>
		constexpr auto ReplaceTupleElemImpl (Tuple&& tuple, NewType&& arg, std::index_sequence<TupIdxs...>) noexcept
		{
			return std::tuple
			{
				GetReplaceTupleElem<RepIdx, TupIdxs> (std::forward<Tuple> (tuple), std::forward<NewType> (arg))...
			};
		}

		template<size_t RepIdx, typename NewType, typename... TupleArgs>
		constexpr auto ReplaceTupleElem (std::tuple<TupleArgs...>&& tuple, NewType&& arg) noexcept
		{
			return ReplaceTupleElemImpl<RepIdx> (std::move (tuple),
					std::forward<NewType> (arg),
					std::index_sequence_for<TupleArgs...> {});
		}

		template<typename Seq, typename T>
		struct DetectShift
		{
			constexpr static int Value = 1;
		};

		template<typename Seq, typename... Args>
		struct DetectShift<Seq, std::tuple<Args...>>
		{
			constexpr static int Value = (DetectShift<Seq, Args>::Value + ...);
		};

		template<typename Seq>
		struct DetectShift<Seq, Seq>
		{
			constexpr static int Value = SeqSize<Seq>;
		};

		template<typename... LArgs, typename... RArgs>
		auto Combine (std::tuple<LArgs...>&& left, std::tuple<RArgs...>&& right) noexcept
		{
			return std::tuple_cat (std::move (left), std::move (right));
		}

		template<typename... LArgs, typename R>
		auto Combine (std::tuple<LArgs...>&& left, const R& right) noexcept
		{
			return std::tuple_cat (std::move (left), std::tuple { right });
		}

		template<typename L, typename... RArgs>
		auto Combine (const L& left, std::tuple<RArgs...>&& right) noexcept
		{
			return std::tuple_cat (std::tuple { left }, std::move (right));
		}

		template<typename L, typename R>
		auto Combine (const L& left, const R& right) noexcept
		{
			return std::tuple { left, right };
		}

		enum ResultBehaviour
		{
			All,
			First,
		};

		template<ResultBehaviour ResultBehaviour, typename ResList>
		decltype (auto) HandleResultBehaviour (ResList&& list) noexcept
		{
			if constexpr (ResultBehaviour == ResultBehaviour::All)
				return std::forward<ResList> (list);
			else if constexpr (ResultBehaviour == ResultBehaviour::First)
				return list.value (0);
		}

		template<typename F, typename R>
		struct HandleSelectorResult
		{
			QString Fields_;
			F Initializer_;
			R Behaviour_;
		};

		template<typename F, typename R>
		HandleSelectorResult (QString, F, R) -> HandleSelectorResult<F, R>;

		class SelectWrapperCommon
		{
			const QSqlDatabase DB_;
		public:
			SelectWrapperCommon (const QSqlDatabase& db) noexcept
			: DB_ { db }
			{
			}
		protected:
			auto RunQuery (const QString& queryStr,
					auto&& binder) const
			{
				QSqlQuery query { DB_ };
				qDebug () << queryStr;
				query.prepare (queryStr);
				binder (query);

				if (!query.exec ())
				{
					qCritical () << "select query execution failed";
					DBLock::DumpError (query);
					throw QueryException ("fetch query execution failed", std::make_shared<QSqlQuery> (query));
				}

				return query;
			}
		};

		template<typename L, typename O>
		constexpr auto LimitOffsetToString () noexcept
		{
			if constexpr (std::is_same_v<L, LimitNone>)
			{
				static_assert (std::is_same_v<O, OffsetNone>, "LIMIT-less queries currently cannot have OFFSET");
				return ""_ct;
			}
			else
				return " LIMIT :limit "_ct +
						[] () constexpr
						{
							if constexpr (std::is_same_v<O, OffsetNone>)
								return ""_ct;
							else
								return " OFFSET :offset"_ct;
						} ();
		}

		template<typename L, typename O>
		void BindLimitOffset (QSqlQuery& query, L limit, O offset) noexcept
		{
			if constexpr (!std::is_same_v<std::decay_t<L>, LimitNone>)
				query.bindValue (":limit", qulonglong { limit.Count });
			if constexpr (!std::is_same_v<std::decay_t<O>, OffsetNone>)
				query.bindValue (":offset", qulonglong { offset.Count });
		}

		template<typename T, typename Selector>
		struct HandleSelector {};

		struct HSBaseAll { constexpr inline static auto ResultBehaviour_v = ResultBehaviour::All; };

		struct HSBaseFirst { constexpr inline static auto ResultBehaviour_v = ResultBehaviour::First; };

		template<typename T>
		struct HandleSelector<T, SelectWhole> : HSBaseAll
		{
			constexpr inline static auto Fields = JoinTup (QualifiedFieldNames<T>, ", "_ct);

			static auto Initializer (const QSqlQuery& q, int startIdx)
			{
				return InitializeFromQuery<T> (q, SeqIndices<T>, startIdx);
			}
		};

		template<typename T, auto... Ptrs>
		struct HandleSelector<T, MemberPtrs<Ptrs...>> : HSBaseAll
		{
		private:
			template<size_t... Ixs>
			constexpr static auto SelectFields ()
			{
				return std::tuple { std::get<Ixs> (QualifiedFieldNames<T>)... };
			}
		public:
			constexpr inline static auto Fields = JoinTup (SelectFields<FieldIndex<Ptrs> ()...> (), ", ");

			static auto Initializer (const QSqlQuery& q, int startIdx) noexcept
			{
				return MakeIndexedQueryHandler (MemberPtrs<Ptrs...> {}, q, startIdx);
			}
		};

		template<typename T>
		struct HandleSelector<T, AggregateType<AggregateFunction::Count, CountAllPtr>> : HSBaseFirst
		{
			constexpr inline static auto Fields = "count(1)"_ct;

			static auto Initializer (const QSqlQuery& q, int startIdx)
			{
				return q.value (startIdx).toLongLong ();
			}
		};

		template<typename T, auto Ptr>
		struct HandleSelector<T, AggregateType<AggregateFunction::Count, Ptr>> : HSBaseFirst
		{
			constexpr inline static auto Fields = "count(" + GetQualifiedFieldNamePtr<Ptr> () + ")";

			static auto Initializer (const QSqlQuery& q, int startIdx)
			{
				return q.value (startIdx).toLongLong ();
			}
		};

		template<CtString Aggregate, typename T, auto Ptr>
		struct HandleAggSelector : HSBaseFirst
		{
			constexpr inline static auto Fields = Aggregate + "(" + GetQualifiedFieldNamePtr<Ptr> () + ")";

			static auto Initializer (const QSqlQuery& q, int startIdx) noexcept
			{
				return MakeIndexedQueryHandler<Ptr> (q, startIdx);
			}
		};

		template<typename T, auto Ptr>
		struct HandleSelector<T, AggregateType<AggregateFunction::Min, Ptr>> : HandleAggSelector<"min"_ct, T, Ptr> {};

		template<typename T, auto Ptr>
		struct HandleSelector<T, AggregateType<AggregateFunction::Max, Ptr>> : HandleAggSelector<"max"_ct, T, Ptr> {};

		constexpr auto CombineBehaviour (ResultBehaviour l, ResultBehaviour r) noexcept
		{
			if (l == ResultBehaviour::First && r == ResultBehaviour::First)
				return ResultBehaviour::First;
			return ResultBehaviour::All;
		}

		template<typename T, typename L, typename R>
		struct HandleSelector<T, SelectorUnion<L, R>>
		{
			using HL = HandleSelector<T, L>;
			using HR = HandleSelector<T, R>;

			constexpr inline static auto ResultBehaviour_v = CombineBehaviour (HL::ResultBehaviour_v, HR::ResultBehaviour_v);

			constexpr inline static auto Fields = HL::Fields + ", " + HR::Fields;

			static auto Initializer (const QSqlQuery& q, int startIdx) noexcept
			{
				constexpr auto shift = DetectShift<T, decltype (HL::Initializer (q, 0))>::Value;
				return Combine (HL::Initializer (q, startIdx), HR::Initializer (q, startIdx + shift));
			}
		};

		template<typename T, SelectBehaviour SelectBehaviour>
		class SelectWrapper : SelectWrapperCommon
		{
			template<typename ParamsTuple>
			struct Builder
			{
				const SelectWrapper& W_;
				ParamsTuple Params_;

				template<typename NewTuple>
				constexpr auto RepTuple (NewTuple&& tuple) noexcept
				{
					return Builder<NewTuple> { W_, tuple };
				}

				template<typename U>
				constexpr auto Select (U&& selector) && noexcept
				{
					return RepTuple (ReplaceTupleElem<0> (std::move (Params_), std::forward<U> (selector)));
				}

				template<typename U>
				constexpr auto Where (U&& tree) && noexcept
				{
					return RepTuple (ReplaceTupleElem<1> (std::move (Params_), std::forward<U> (tree)));
				}

				template<typename U>
				constexpr auto Order (U&& order) && noexcept
				{
					return RepTuple (ReplaceTupleElem<2> (std::move (Params_), std::forward<U> (order)));
				}

				template<typename U>
				constexpr auto Group (U&& group) && noexcept
				{
					return RepTuple (ReplaceTupleElem<3> (std::move (Params_), std::forward<U> (group)));
				}

				constexpr auto Limit (Limit limit) && noexcept
				{
					return RepTuple (ReplaceTupleElem<4> (std::move (Params_), limit));
				}

				constexpr auto Limit (uint64_t limit) && noexcept
				{
					return std::move (*this).Limit (oral::Limit { limit });
				}

				constexpr auto Offset (Offset offset) && noexcept
				{
					return RepTuple (ReplaceTupleElem<5> (std::move (Params_), offset));
				}

				constexpr auto Offset (uint64_t offset) && noexcept
				{
					return std::move (*this).Offset (oral::Offset { offset });
				}

				auto operator() () &&
				{
					return std::apply (W_, Params_);
				}

				template<auto... Ptrs>
				constexpr auto Group () && noexcept
				{
					return std::move (*this).Group (GroupBy<Ptrs...> {});
				}
			};
		public:
			using SelectWrapperCommon::SelectWrapperCommon;

			auto Build () const noexcept
			{
				std::tuple defParams
				{
					SelectWhole {},
					ConstTrueTree_v,
					OrderNone {},
					GroupNone {},
					LimitNone {},
					OffsetNone {}
				};
				return Builder<decltype (defParams)> { *this, defParams };
			}

			auto operator() () const
			{
				return (*this) (SelectWhole {}, ConstTrueTree_v);
			}

			template<typename Single>
			auto operator() (Single&& single) const
			{
				if constexpr (IsExprTree<std::decay_t<Single>> {})
					return (*this) (SelectWhole {}, std::forward<Single> (single));
				else
					return (*this) (std::forward<Single> (single), ConstTrueTree_v);
			}

			template<
					typename Selector,
					ExprType Type, typename L, typename R,
					typename Order = OrderNone,
					typename Group = GroupNone,
					typename Limit = LimitNone,
					typename Offset = OffsetNone
				>
			auto operator() (Selector,
					const ExprTree<Type, L, R>& tree,
					Order order = OrderNone {},
					Group group = GroupNone {},
					Limit limit = LimitNone {},
					Offset offset = OffsetNone {}) const
			{
				using TreeType_t = ExprTree<Type, L, R>;

				constexpr auto where = ExprTreeToSql<"", T, TreeType_t> ();
				constexpr auto wherePrefix = [where]
				{
					if constexpr (where.IsEmpty ())
						return " "_ct;
					else
						return " WHERE "_ct;
				} ();
				constexpr auto from = BuildFromClause<TreeType_t> ();
				const auto binder = [&] (QSqlQuery& query)
				{
					BindExprTree<"", T> (tree, query);
					BindLimitOffset (query, limit, offset);
				};
				using HS = HandleSelector<T, Selector>;

				constexpr auto query = "SELECT " + HS::Fields +
						" FROM " + from +
						wherePrefix + where +
						HandleOrder (std::forward<Order> (order)) +
						HandleGroup (std::forward<Group> (group)) +
						LimitOffsetToString<Limit, Offset> ();
				auto selectResult = Select<HS> (query.ToString (),
						binder);
				return HandleResultBehaviour<HS::ResultBehaviour_v> (std::move (selectResult));
			}
		private:
			template<typename HS, typename Binder>
			auto Select (const QString& queryStr,
					Binder&& binder) const
			{
				auto query = RunQuery (queryStr, binder);

				if constexpr (SelectBehaviour == SelectBehaviour::Some)
				{
					QList<decltype (HS::Initializer (query, 0))> result;
					while (query.next ())
						result << HS::Initializer (query, 0);
					return result;
				}
				else
				{
					using RetType_t = std::optional<decltype (HS::Initializer (query, 0))>;
					return query.next () ?
						RetType_t { HS::Initializer (query, 0) } :
						RetType_t {};
				}
			}

			template<typename Tree>
			consteval static auto BuildFromClause () noexcept
			{
				if constexpr (Tree::template HasAdditionalTables<T> ())
					return T::ClassName + ", " + JoinTup (Nub<Tree::template AdditionalTables<T>> (), ", ");
				else
					return T::ClassName;
			}

			constexpr static auto HandleOrder (OrderNone) noexcept
			{
				return ""_ct;
			}

			template<auto... Ptrs>
			constexpr static auto HandleSuborder (sph::asc<Ptrs...>) noexcept
			{
				return std::tuple { (GetQualifiedFieldNamePtr<Ptrs> () + " ASC")... };
			}

			template<auto... Ptrs>
			constexpr static auto HandleSuborder (sph::desc<Ptrs...>) noexcept
			{
				return std::tuple { (GetQualifiedFieldNamePtr<Ptrs> () + " DESC")... };
			}

			template<typename... Suborders>
			constexpr static auto HandleOrder (OrderBy<Suborders...>) noexcept
			{
				return " ORDER BY " + JoinTup (std::tuple_cat (HandleSuborder (Suborders {})...), ", ");
			}

			constexpr static auto HandleGroup (GroupNone) noexcept
			{
				return ""_ct;
			}

			template<auto... Ptrs>
			constexpr static auto HandleGroup (GroupBy<Ptrs...>) noexcept
			{
				return " GROUP BY " + Join (", ", GetQualifiedFieldNamePtr<Ptrs> ()...);
			}
		};

		template<typename T>
		class DeleteByFieldsWrapper
		{
			const QSqlDatabase DB_;
		public:
			DeleteByFieldsWrapper (const QSqlDatabase& db) noexcept
			: DB_ { db }
			{
			}

			template<ExprType Type, typename L, typename R>
			void operator() (const ExprTree<Type, L, R>& tree) const noexcept
			{
				constexpr auto where = ExprTreeToSql<"", T, ExprTree<Type, L, R>> ();

				constexpr auto selectAll = "DELETE FROM " + T::ClassName + " WHERE " + where;

				QSqlQuery query { DB_ };
				query.prepare (selectAll.ToString ());
				BindExprTree<"", T> (tree, query);
				query.exec ();
			}
		};

		template<typename T>
		class AdaptUpdate
		{
			const QSqlDatabase DB_;

			// TODO this needn't be present of T doesn't have a PKey
			QSqlQuery UpdateByPKey_ { DB_ };
		public:
			AdaptUpdate (const QSqlDatabase& db) noexcept
			: DB_ { db }
			{
				if constexpr (HasPKey<T>)
				{
					constexpr auto pkeyIdx = PKeyIndex_v<T>;
					constexpr auto statements = ZipWith (FieldNames<T>, " = ", BoundFieldNames<T>);
					constexpr auto update = "UPDATE " + T::ClassName +
							" SET " + JoinTup (statements, ", ") +
							" WHERE " + std::get<pkeyIdx> (statements);
					UpdateByPKey_.prepare (update.ToString ());
				}
			}

			void operator() (const T& seq) requires HasPKey<T>
			{
				DoInsert (seq, UpdateByPKey_, true);
			}

			template<typename SL, typename SR, ExprType WType, typename WL, typename WR>
			int operator() (const AssignList<SL, SR>& set, const ExprTree<WType, WL, WR>& where)
			{
				static_assert (!ExprTree<WType, WL, WR>::template HasAdditionalTables<T> (),
						"joins in update statements are not supported by SQL");

				constexpr auto setClause = ExprTreeToSql<"set_", T, AssignList<SL, SR>> ();
				constexpr auto whereClause = ExprTreeToSql<"where_", T, ExprTree<WType, WL, WR>> ();

				constexpr auto update = "UPDATE " + T::ClassName +
						" SET " + setClause +
						" WHERE " + whereClause;

				QSqlQuery query { DB_ };
				query.prepare (update.ToString ());
				BindExprTree<"set_", T> (set, query);
				BindExprTree<"where_", T> (where, query);
				if (!query.exec ())
				{
					qCritical () << "update query execution failed";
					DBLock::DumpError (query);
					throw QueryException ("update query execution failed", std::make_shared<QSqlQuery> (query));
				}

				return query.numRowsAffected ();
			}
		};

		template<typename T, size_t... Fields>
		constexpr auto ExtractConstraintFields (UniqueSubset<Fields...>)
		{
			return "UNIQUE (" + Join (", ", std::get<Fields> (FieldNames<T>)...) + ")";
		};

		template<typename T, size_t... Fields>
		constexpr auto ExtractConstraintFields (PrimaryKey<Fields...>)
		{
			return "PRIMARY KEY (" + Join (", ", std::get<Fields> (FieldNames<T>)...) + ")";
		};

		template<typename T>
		constexpr auto GetConstraintsStrings () noexcept
		{
			if constexpr (requires { typename T::Constraints; })
			{
				return []<typename... Args> (Constraints<Args...>)
				{
					return std::tuple { ExtractConstraintFields<T> (Args {})... };
				} (typename T::Constraints {});
			}
			else
				return std::tuple<> {};
		}

		template<typename ImplFactory, typename T, size_t... Indices>
		constexpr auto GetTypes (std::index_sequence<Indices...>) noexcept
		{
			return std::tuple { Type2Name<ImplFactory, ValueAtC_t<T, Indices>> {} ()... };
		}

		template<auto Name, typename ImplFactory, typename T>
		constexpr auto AdaptCreateTableNamed () noexcept
		{
			constexpr auto types = GetTypes<ImplFactory, T> (SeqIndices<T>);

			constexpr auto constraints = GetConstraintsStrings<T> ();
			constexpr auto constraintsStr = [&]
			{
				if constexpr (!std::tuple_size_v<decltype (constraints)>)
					return ""_ct;
				else
					return ", " + JoinTup (constraints, ", ");
			} ();

			constexpr auto statements = ZipWith (FieldNames<T>, " ", types);
			return "CREATE TABLE " +
					Name +
					" (" +
					JoinTup (statements, ", ") +
					constraintsStr +
					");";
		}

		template<typename ImplFactory, typename T>
		constexpr auto AdaptCreateTable () noexcept
		{
			return AdaptCreateTableNamed<T::ClassName, ImplFactory, T> ();
		}
	}

	template<typename T>
	struct ObjectInfo
	{
		detail::AdaptInsert<T> Insert;
		detail::AdaptUpdate<T> Update;
		detail::AdaptDelete<T> Delete;

		detail::SelectWrapper<T, detail::SelectBehaviour::Some> Select;
		detail::SelectWrapper<T, detail::SelectBehaviour::One> SelectOne;
		detail::DeleteByFieldsWrapper<T> DeleteBy;

		using ObjectType_t = T;
	};

	template<typename T, typename ImplFactory = detail::SQLite::ImplFactory>
	ObjectInfo<T> Adapt (const QSqlDatabase& db)
	{
		if (!db.tables ().contains (T::ClassName.ToString (), Qt::CaseInsensitive))
		{
			constexpr auto query = detail::AdaptCreateTable<ImplFactory, T> ();
			RunTextQuery (db, query.ToString ());
		}

		return
		{
			{ db },
			{ db },
			{ db },

			{ db },
			{ db },
			{ db },
		};
	}

	template<typename T>
	using ObjectInfo_ptr = std::unique_ptr<ObjectInfo<T>>;

	template<typename T, typename ImplFactory = SQLiteImplFactory>
	ObjectInfo_ptr<T> AdaptPtr (const QSqlDatabase& db)
	{
		return std::make_unique<ObjectInfo<T>> (Adapt<T, ImplFactory> (db));
	}

	template<typename ImplFactory, typename... Ts>
	void AdaptPtrs (const QSqlDatabase& db, ObjectInfo_ptr<Ts>&... objects)
	{
		((objects = AdaptPtr<Ts, ImplFactory> (db)), ...);
	}
}
