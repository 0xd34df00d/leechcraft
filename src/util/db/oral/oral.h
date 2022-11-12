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
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/fold.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/zip.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <QStringList>
#include <QDateTime>
#include <QPair>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDateTime>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/typelist.h>
#include <util/sll/typelevel.h>
#include <util/sll/typegetter.h>
#include <util/sll/detector.h>
#include <util/sll/unreachable.h>
#include <util/sll/void.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include "oraltypes.h"
#include "oraldetailfwd.h"
#include "impldefs.h"
#include "sqliteimpl.h"

namespace LC
{
namespace Util
{
namespace oral
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

	namespace detail
	{
		template<typename T>
		QString MorphFieldName (QString str) noexcept
		{
			if constexpr (requires { T::FieldNameMorpher (QString {}); })
				return T::FieldNameMorpher (str);
			else
			{
				if (str.endsWith ('_'))
					str.chop (1);
				return str;
			}
		}

		template<typename Seq, int Idx>
		QString GetFieldName () noexcept
		{
			return MorphFieldName<Seq> (boost::fusion::extension::struct_member_name<Seq, Idx>::call ());
		}

		template<typename S>
		constexpr auto SeqSize = boost::fusion::result_of::size<S>::type::value;

		template<typename S>
		constexpr auto SeqIndices = std::make_index_sequence<SeqSize<S>> {};

		template<typename S>
		struct GetFieldsNames
		{
			QStringList operator() () const noexcept
			{
				return Run (SeqIndices<S>);
			}
		private:
			template<size_t... Vals>
			QStringList Run (std::index_sequence<Vals...>) const noexcept
			{
				return { GetFieldName<S, Vals> ()... };
			}
		};

		template<typename S>
		struct AddressOf
		{
			inline static S Obj_ {};

			template<auto P>
			static constexpr auto Ptr () noexcept
			{
				return &(Obj_.*P);
			}

			template<int Idx>
			static constexpr auto Index () noexcept
			{
				return &boost::fusion::at_c<Idx> (Obj_);
			}
		};

		template<auto Ptr, size_t Idx = 0>
		constexpr size_t FieldIndex () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;

			if constexpr (Idx == SeqSize<S>)
				return -1;
			else
			{
				constexpr auto direct = AddressOf<S>::template Ptr<Ptr> ();
				constexpr auto indexed = AddressOf<S>::template Index<Idx> ();
				if constexpr (std::is_same_v<decltype (direct), decltype (indexed)>)
				{
					if (indexed == direct)
						return Idx;
				}

				return FieldIndex<Ptr, Idx + 1> ();
			}
		}

		template<auto Ptr>
		QString GetFieldNamePtr () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;
			return GetFieldName<S, FieldIndex<Ptr> ()> ();
		}

		template<auto Ptr>
		QString GetQualifiedFieldNamePtr () noexcept
		{
			using S = MemberPtrStruct_t<Ptr>;
			return S::ClassName () + "." + GetFieldName<S, FieldIndex<Ptr> ()> ();
		}

		template<typename T>
		concept TypeNameCustomized = requires { typename T::TypeName; };

		template<typename T>
		concept BaseTypeCustomized = requires { typename T::BaseType; };
	}

	template<typename ImplFactory, typename T, typename = void>
	struct Type2Name
	{
		QString operator() () const noexcept
		{
			if constexpr (HasType<T> (Typelist<int, qlonglong, qulonglong, bool> {}) || std::is_enum_v<T>)
				return "INTEGER";
			else if constexpr (std::is_same_v<T, double>)
				return "REAL";
			else if constexpr (std::is_same_v<T, QString> || std::is_same_v<T, QDateTime> || std::is_same_v<T, QUrl>)
				return "TEXT";
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
		QString operator() () const noexcept { return Type2Name<ImplFactory, T> () () + " UNIQUE"; }
	};

	template<typename ImplFactory, typename T>
	struct Type2Name<ImplFactory, NotNull<T>>
	{
		QString operator() () const noexcept { return Type2Name<ImplFactory, T> () () + " NOT NULL"; }
	};

	template<typename ImplFactory, typename T, typename... Tags>
	struct Type2Name<ImplFactory, PKey<T, Tags...>>
	{
		QString operator() () const noexcept { return Type2Name<ImplFactory, T> () () + " PRIMARY KEY"; }
	};

	template<typename ImplFactory, typename... Tags>
	struct Type2Name<ImplFactory, PKey<int, Tags...>>
	{
		QString operator() () const noexcept { return ImplFactory::TypeLits::IntAutoincrement; }
	};

	template<typename ImplFactory, auto Ptr>
	struct Type2Name<ImplFactory, References<Ptr>>
	{
		QString operator() () const noexcept
		{
			const auto& className = MemberPtrStruct_t<Ptr>::ClassName ();
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
		template<typename T>
		struct IsPKey : std::false_type {};

		template<typename U, typename... Tags>
		struct IsPKey<PKey<U, Tags...>> : std::true_type {};

		template<typename T>
		QVariant ToVariantF (const T& t) noexcept
		{
			return ToVariant<T> {} (t);
		}

		template<typename T>
		auto MakeInserter (const CachedFieldsData& data, const QSqlQuery_ptr& insertQuery, bool bindPrimaryKey) noexcept
		{
			return [data, insertQuery, bindPrimaryKey] (const T& t)
			{
				boost::fusion::fold (t, data.BoundFields_.begin (),
						[&] (auto pos, const auto& elem)
						{
							using Elem = std::decay_t<decltype (elem)>;
							if (bindPrimaryKey || !IsPKey<Elem>::value)
								insertQuery->bindValue (*pos++, ToVariantF (elem));
							return pos;
						});

				if (!insertQuery->exec ())
				{
					DBLock::DumpError (*insertQuery);
					throw QueryException ("insert query execution failed", insertQuery);
				}
			};
		}

		template<typename Seq, int Idx>
		using ValueAtC_t = typename boost::fusion::result_of::value_at_c<Seq, Idx>::type;

		template<typename Seq, typename Idx>
		using ValueAt_t = typename boost::fusion::result_of::value_at<Seq, Idx>::type;

		template<typename Seq, typename MemberIdx = boost::mpl::int_<0>>
		struct FindPKey
		{
			static_assert ((boost::fusion::result_of::size<Seq>::value) != (MemberIdx::value),
					"Primary key not found");

			template<typename T>
			struct Lazy
			{
				using type = T;
			};

			using result_type = typename std::conditional_t<
						IsPKey<ValueAt_t<Seq, MemberIdx>>::value,
						Lazy<MemberIdx>,
						Lazy<FindPKey<Seq, typename boost::mpl::next<MemberIdx>::type>>
					>::type;
		};

		template<typename Seq>
		using FindPKeyDetector = boost::mpl::int_<FindPKey<Seq>::result_type::value>;

		template<typename Seq>
		constexpr auto HasPKey = IsDetected_v<FindPKeyDetector, Seq>;

		template<typename Seq>
		constexpr auto HasAutogenPKey () noexcept
		{
			if constexpr (HasPKey<Seq>)
				return !HasType<NoAutogen> (AsTypelist_t<ValueAtC_t<Seq, FindPKey<Seq>::result_type::value>> {});
			else
				return false;
		}

		template<typename T>
		CachedFieldsData BuildCachedFieldsData (const QString& table) noexcept
		{
			const auto& fields = detail::GetFieldsNames<T> {} ();
			const auto& qualified = Util::Map (fields, [&table] (const QString& field) { return table + "." + field; });
			const auto& boundFields = Util::Map (fields, [] (const QString& str) { return ':' + str; });

			return { table, fields, qualified, boundFields };
		}

		template<typename T>
		CachedFieldsData BuildCachedFieldsData () noexcept
		{
			static CachedFieldsData result = BuildCachedFieldsData<T> (T::ClassName ());
			return result;
		}

		template<typename Seq>
		class AdaptInsert
		{
			const QSqlDatabase DB_;
			const CachedFieldsData Data_;

			constexpr static bool HasAutogen_ = HasAutogenPKey<Seq> ();

			IInsertQueryBuilder_ptr QueryBuilder_;
		public:
			template<typename ImplFactory>
			AdaptInsert (const QSqlDatabase& db, CachedFieldsData data, ImplFactory&& factory) noexcept
			: Data_ { RemovePKey (data) }
			, QueryBuilder_ { factory.MakeInsertQueryBuilder (db, Data_) }
			{
			}

			auto operator() (Seq& t, InsertAction action = InsertAction::Default) const
			{
				return Run<true> (t, action);
			}

			auto operator() (const Seq& t, InsertAction action = InsertAction::Default) const
			{
				return Run<false> (t, action);
			}
		private:
			template<bool UpdatePKey, typename Val>
			auto Run (Val&& t, InsertAction action) const
			{
				const auto query = QueryBuilder_->GetQuery (action);

				MakeInserter<Seq> (Data_, query, !HasAutogen_) (t);

				if constexpr (HasAutogen_)
				{
					constexpr auto index = FindPKey<Seq>::result_type::value;

					const auto& lastId = FromVariant<ValueAtC_t<Seq, index>> {} (query->lastInsertId ());
					if constexpr (UpdatePKey)
						boost::fusion::at_c<index> (t) = lastId;
					else
						return lastId;
				}
			}

			static CachedFieldsData RemovePKey (CachedFieldsData data) noexcept
			{
				if constexpr (HasAutogen_)
				{
					constexpr auto index = FindPKey<Seq>::result_type::value;
					data.Fields_.removeAt (index);
					data.BoundFields_.removeAt (index);
				}
				return data;
			}
		};

		template<typename Seq, bool HasPKey = HasPKey<Seq>>
		struct AdaptDelete
		{
			std::function<void (Seq)> Deleter_;
		public:
			template<bool B = HasPKey>
			AdaptDelete (const QSqlDatabase& db, const CachedFieldsData& data, std::enable_if_t<B>* = nullptr) noexcept
			{
				const auto index = FindPKey<Seq>::result_type::value;

				const auto& boundName = data.BoundFields_.at (index);
				const auto& del = "DELETE FROM " + data.Table_ +
						" WHERE " + data.Fields_.at (index) + " = " + boundName;

				const auto deleteQuery = std::make_shared<QSqlQuery> (db);
				deleteQuery->prepare (del);

				Deleter_ = [deleteQuery, boundName] (const Seq& t)
				{
					constexpr auto index = FindPKey<Seq>::result_type::value;
					deleteQuery->bindValue (boundName, ToVariantF (boost::fusion::at_c<index> (t)));
					if (!deleteQuery->exec ())
						throw QueryException ("delete query execution failed", deleteQuery);
				};
			}

			template<bool B = HasPKey>
			AdaptDelete (const QSqlDatabase&, const CachedFieldsData&, std::enable_if_t<!B>* = nullptr) noexcept
			{
			}

			template<bool B = HasPKey>
			std::enable_if_t<B> operator() (const Seq& seq)
			{
				Deleter_ (seq);
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
				((boost::fusion::at_c<Indices> (t) = FromVariant<ValueAtC_t<T, Indices>> {} (q.value (startIdx + Indices))), ...);
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

		inline QString TypeToSql (ExprType type) noexcept
		{
			switch (type)
			{
			case ExprType::Greater:
				return ">";
			case ExprType::Less:
				return "<";
			case ExprType::Equal:
				return "=";
			case ExprType::Geq:
				return ">=";
			case ExprType::Leq:
				return "<=";
			case ExprType::Neq:
				return "!=";
			case ExprType::Like:
				return "LIKE";
			case ExprType::And:
				return "AND";
			case ExprType::Or:
				return "OR";

			case ExprType::LeafStaticPlaceholder:
			case ExprType::LeafData:
			case ExprType::ConstTrue:
				return "invalid type";
			}

			Util::Unreachable ();
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
		struct ToSqlState
		{
			int LastID_;
			QVariantMap BoundMembers_;
		};

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

			template<typename T>
			QString ToSql (ToSqlState<T>& state) const noexcept
			{
				if constexpr (IsExprTree<L> {})
					return Left_.GetFieldName () + " = " + Right_.ToSql (state);
				else
					return Left_.ToSql (state) + ", " + Right_.ToSql (state);
			}

			template<typename OL, typename OR>
			auto operator, (const AssignList<OL, OR>& tail) noexcept
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

			template<typename T>
			QString ToSql (ToSqlState<T>& state) const noexcept
			{
				static_assert (RelationalTypesChecker<Type, T, L, R>::value,
						"Incompatible types passed to a relational operator.");

				return Left_.ToSql (state) + " " + TypeToSql (Type) + " " + Right_.ToSql (state);
			}

			template<typename T>
			QSet<QString> AdditionalTables () const noexcept
			{
				return Left_.template AdditionalTables<T> () + Right_.template AdditionalTables<T> ();
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

			template<typename ObjT>
			QString ToSql (ToSqlState<ObjT>& state) const noexcept
			{
				const auto& name = ":bound_" + QString::number (++state.LastID_);
				state.BoundMembers_ [name] = ToVariantF (Data_);
				return name;
			}

			template<typename>
			QSet<QString> AdditionalTables () const noexcept
			{
				return {};
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

			template<typename T>
			QString ToSql (ToSqlState<T>&) const noexcept
			{
				return MemberPtrStruct_t<Ptr>::ClassName () + "." + GetFieldName ();
			}

			QString GetFieldName () const noexcept
			{
				return detail::GetFieldNamePtr<Ptr> ();
			}

			template<typename T>
			QSet<QString> AdditionalTables () const noexcept
			{
				using Seq = MemberPtrStruct_t<Ptr>;
				if constexpr (std::is_same_v<Seq, T>)
					return {};
				else
					return { Seq::ClassName () };
			}

			template<typename T>
			constexpr static bool HasAdditionalTables () noexcept
			{
				return !std::is_same_v<MemberPtrStruct_t<Ptr>, T>;
			}

			auto operator= (const ExpectedType_t& r) const noexcept
			{
				return AssignList { *this, AsLeafData (r) };
			}
		};

		template<>
		class ExprTree<ExprType::ConstTrue, void, void> {};

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

		template<typename F>
		struct ExprTreeHandler
		{
			QString Sql_;
			F Binder_;
			int LastID_;

			ExprTreeHandler (const QString& sql, F&& binder, int lastId) noexcept
			: Sql_ { sql }
			, Binder_ { std::move (binder) }
			, LastID_ { lastId }
			{
			}
		};

		template<typename>
		auto HandleExprTree (const ExprTree<ExprType::ConstTrue>&, int lastId = 0) noexcept
		{
			return ExprTreeHandler { "1 = 1", [] (auto&&) {}, lastId };
		}

		template<typename Seq, typename Tree,
				typename = decltype (std::declval<Tree> ().ToSql (std::declval<ToSqlState<Seq>&> ()))>
		auto HandleExprTree (const Tree& tree, int lastId = 0) noexcept
		{
			ToSqlState<Seq> state { lastId, {} };

			const auto& sql = tree.ToSql (state);

			return ExprTreeHandler
			{
				sql,
				[state] (QSqlQuery& query)
				{
					for (const auto& pair : Stlize (state.BoundMembers_))
						query.bindValue (pair.first, pair.second);
				},
				state.LastID_
			};
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

		Limit (uint64_t count) noexcept
		: Count { count }
		{
		}
	};

	struct Offset
	{
		uint64_t Count;

		Offset (uint64_t count) noexcept
		: Count { count }
		{
		}
	};

	namespace detail
	{
		template<auto... Ptrs, size_t... Idxs>
		auto MakeIndexedQueryHandler (MemberPtrs<Ptrs...>, std::index_sequence<Idxs...>) noexcept
		{
			return [] (const QSqlQuery& q, int startIdx = 0) noexcept
			{
				if constexpr (sizeof... (Ptrs) == 1)
					return FromVariant<UnwrapIndirect_t<Head_t<Typelist<MemberPtrType_t<Ptrs>...>>>> {} (q.value (startIdx));
				else
					return std::tuple { FromVariant<UnwrapIndirect_t<MemberPtrType_t<Ptrs>>> {} (q.value (startIdx + Idxs))... };
			};
		}

		template<auto Ptr>
		auto MakeIndexedQueryHandler () noexcept
		{
			return [] (const QSqlQuery& q, int startIdx = 0) noexcept
			{
				return FromVariant<UnwrapIndirect_t<MemberPtrType_t<Ptr>>> {} (q.value (startIdx));
			};
		}

		template<auto... Ptrs>
		QStringList BuildFieldNames () noexcept
		{
			return { BuildCachedFieldsData<MemberPtrStruct_t<Ptrs>> ().QualifiedFields_.value (FieldIndex<Ptrs> ())... };
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

		struct ResultBehaviour
		{
			struct All {};
			struct First {};
		};

		template<typename L, typename R>
		constexpr auto CombineBehaviour (L, R) noexcept
		{
			if constexpr (std::is_same_v<L, ResultBehaviour::First> && std::is_same_v<R, ResultBehaviour::First>)
				return ResultBehaviour::First {};
			else
				return ResultBehaviour::All {};
		}

		template<typename ResList>
		decltype (auto) HandleResultBehaviour (ResultBehaviour::All, ResList&& list) noexcept
		{
			return std::forward<ResList> (list);
		}

		template<typename ResList>
		auto HandleResultBehaviour (ResultBehaviour::First, ResList&& list) noexcept
		{
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
		protected:
			const QSqlDatabase DB_;
			const QString LimitNone_;

			SelectWrapperCommon (const QSqlDatabase& db, const QString& limitNone)
			: DB_ { db }
			, LimitNone_ { limitNone }
			{
			}

			auto RunQuery (const QString& fields, const QString& from,
					QString where, std::function<void (QSqlQuery&)>&& binder,
					const QString& orderStr,
					const QString& groupStr,
					const QString& limitOffsetStr) const
			{
				if (!where.isEmpty ())
					where.prepend (" WHERE ");

				const auto& queryStr = "SELECT " + fields +
						" FROM " + from +
						where +
						orderStr +
						groupStr +
						limitOffsetStr;

				QSqlQuery query { DB_ };
				query.prepare (queryStr);
				if (binder)
					binder (query);

				if (!query.exec ())
				{
					DBLock::DumpError (query);
					throw QueryException ("fetch query execution failed", std::make_shared<QSqlQuery> (query));
				}

				return query;
			}

			QString HandleLimitOffset (LimitNone, OffsetNone) const noexcept
			{
				return {};
			}

			QString HandleLimitOffset (Limit limit, OffsetNone) const noexcept
			{
				return " LIMIT " + QString::number (limit.Count);
			}

			template<typename L>
			QString HandleLimitOffset (L limit, Offset offset) const noexcept
			{
				QString limitStr;
				if constexpr (std::is_same_v<std::decay_t<L>, LimitNone>)
				{
					Q_UNUSED (limit)
					limitStr = LimitNone_;
				}
				else if constexpr (std::is_integral_v<L>)
					limitStr = QString::number (limit);
				else
					limitStr = QString::number (limit.Count);
				return " LIMIT " + limitStr +
						" OFFSET " + QString::number (offset.Count);
			}
		};

		template<typename T, SelectBehaviour SelectBehaviour>
		class SelectWrapper : SelectWrapperCommon
		{
			const CachedFieldsData Cached_;

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

				template<typename U = Limit>
				constexpr auto Limit (U&& limit) && noexcept
				{
					return RepTuple (ReplaceTupleElem<4> (std::move (Params_), std::forward<U> (limit)));
				}

				template<typename U = Offset>
				constexpr auto Offset (U&& offset) && noexcept
				{
					return RepTuple (ReplaceTupleElem<5> (std::move (Params_), std::forward<U> (offset)));
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
			template<typename ImplFactory>
			SelectWrapper (const QSqlDatabase& db, const CachedFieldsData& data, ImplFactory&& factory) noexcept
			: SelectWrapperCommon { db, factory.LimitNone }
			, Cached_ { data }
			{
			}

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
			auto operator() (Selector selector,
					const ExprTree<Type, L, R>& tree,
					Order order = OrderNone {},
					Group group = GroupNone {},
					Limit limit = LimitNone {},
					Offset offset = OffsetNone {}) const
			{
				const auto& [where, binder, _] = HandleExprTree<T> (tree);
				Q_UNUSED (_);
				const auto& [fields, initializer, resultBehaviour] = HandleSelector (std::forward<Selector> (selector));
				return HandleResultBehaviour (resultBehaviour,
						Select (fields, BuildFromClause (tree),
								where, binder,
								initializer,
								HandleOrder (std::forward<Order> (order)),
								HandleGroup (std::forward<Group> (group)),
								HandleLimitOffset (std::forward<Limit> (limit), std::forward<Offset> (offset))));
			}
		private:
			template<typename Binder, typename Initializer>
			auto Select (const QString& fields, const QString& from,
					const QString& where, Binder&& binder,
					Initializer&& initializer,
					const QString& orderStr,
					const QString& groupStr,
					const QString& limitOffsetStr) const
			{
				std::function<void (QSqlQuery&)> binderFunc;
				if constexpr (!std::is_same_v<Void, std::decay_t<Binder>>)
					binderFunc = binder;
				auto query = RunQuery (fields, from, where, std::move (binderFunc), orderStr, groupStr, limitOffsetStr);

				if constexpr (SelectBehaviour == SelectBehaviour::Some)
				{
					QList<std::result_of_t<Initializer (QSqlQuery)>> result;
					while (query.next ())
						result << initializer (query);
					return result;
				}
				else
				{
					using RetType_t = std::optional<std::result_of_t<Initializer (QSqlQuery)>>;
					return query.next () ?
						RetType_t { initializer (query) } :
						RetType_t {};
				}
			}

			template<ExprType Type, typename L, typename R>
			QString BuildFromClause (const ExprTree<Type, L, R>& tree) const noexcept
			{
				if constexpr (Type != ExprType::ConstTrue)
				{
					auto result = Cached_.Table_;
					for (const auto& item : tree.template AdditionalTables<T> ())
						result += ", " + item;
					return result;
				}
				else
					return Cached_.Table_;
			}

			auto HandleSelector (SelectWhole) const noexcept
			{
				return HandleSelectorResult
				{
					Cached_.QualifiedFields_.join (", "),
					[] (const QSqlQuery& q, int startIdx = 0)
					{
						return InitializeFromQuery<T> (q, SeqIndices<T>, startIdx);
					},
					ResultBehaviour::All {}
				};
			}

			template<auto... Ptrs>
			auto HandleSelector (MemberPtrs<Ptrs...> ptrs) const noexcept
			{
				return HandleSelectorResult
				{
					BuildFieldNames<Ptrs...> ().join (", "),
					MakeIndexedQueryHandler (ptrs, std::make_index_sequence<sizeof... (Ptrs)> {}),
					ResultBehaviour::All {}
				};
			}

			auto HandleSelector (AggregateType<AggregateFunction::Count, CountAllPtr>) const noexcept
			{
				return HandleSelectorResult
				{
					"count(1)",
					[] (const QSqlQuery& q, int startIdx = 0) { return q.value (startIdx).toLongLong (); },
					ResultBehaviour::First {}
				};
			}

			template<auto Ptr>
			auto HandleSelector (AggregateType<AggregateFunction::Count, Ptr>) const noexcept
			{
				return HandleSelectorResult
				{
					"count(" + GetQualifiedFieldNamePtr<Ptr> () + ")",
					[] (const QSqlQuery& q, int startIdx = 0) { return q.value (startIdx).toLongLong (); },
					ResultBehaviour::First {}
				};
			}

			template<auto Ptr>
			auto HandleSelector (AggregateType<AggregateFunction::Min, Ptr>) const noexcept
			{
				return HandleSelectorResult
				{
					"min(" + GetQualifiedFieldNamePtr<Ptr> () + ")",
					MakeIndexedQueryHandler<Ptr> (),
					ResultBehaviour::First {}
				};
			}

			template<auto Ptr>
			auto HandleSelector (AggregateType<AggregateFunction::Max, Ptr>) const noexcept
			{
				return HandleSelectorResult
				{
					"max(" + GetQualifiedFieldNamePtr<Ptr> () + ")",
					MakeIndexedQueryHandler<Ptr> (),
					ResultBehaviour::First {}
				};
			}

			template<typename L, typename R>
			auto HandleSelector (SelectorUnion<L, R>) const noexcept
			{
				const auto& lSel = HandleSelector (L {});
				const auto& rSel = HandleSelector (R {});

				const auto& lHandler = lSel.Initializer_;
				const auto& rHandler = rSel.Initializer_;

				return HandleSelectorResult
				{
					lSel.Fields_ + ", " + rSel.Fields_,
					[lHandler, rHandler] (const QSqlQuery& q, int startIdx = 0)
					{
						constexpr auto shift = DetectShift<T, decltype (lHandler (q))>::Value;
						return Combine (lHandler (q, startIdx), rHandler (q, startIdx + shift));
					},
					CombineBehaviour (lSel.Behaviour_, rSel.Behaviour_)
				};
			}

			QString HandleOrder (OrderNone) const noexcept
			{
				return {};
			}

			template<auto... Ptrs>
			QList<QString> HandleSuborder (sph::asc<Ptrs...>) const noexcept
			{
				return { (GetQualifiedFieldNamePtr<Ptrs> () + " ASC")... };
			}

			template<auto... Ptrs>
			QList<QString> HandleSuborder (sph::desc<Ptrs...>) const noexcept
			{
				return { (GetQualifiedFieldNamePtr<Ptrs> () + " DESC")... };
			}

			template<typename... Suborders>
			QString HandleOrder (OrderBy<Suborders...>) const noexcept
			{
				return " ORDER BY " + QStringList { Concat (QList { HandleSuborder (Suborders {})... }) }.join (", ");
			}

			QString HandleGroup (GroupNone) const noexcept
			{
				return {};
			}

			template<auto... Ptrs>
			QString HandleGroup (GroupBy<Ptrs...>) const noexcept
			{
				return " GROUP BY " + QStringList { GetQualifiedFieldNamePtr<Ptrs> ()... }.join (", ");
			}
		};

		template<typename T>
		class DeleteByFieldsWrapper
		{
			const QSqlDatabase DB_;
			const QString Table_;
		public:
			DeleteByFieldsWrapper (const QSqlDatabase& db, const CachedFieldsData& data) noexcept
			: DB_ { db }
			, Table_ (data.Table_)
			{
			}

			template<ExprType Type, typename L, typename R>
			void operator() (const ExprTree<Type, L, R>& tree) const noexcept
			{
				const auto& [where, binder, _] = HandleExprTree<T> (tree);
				Q_UNUSED (_);

				const auto& selectAll = "DELETE FROM " + Table_ +
						" WHERE " + where;

				QSqlQuery query { DB_ };
				query.prepare (selectAll);
				binder (query);
				query.exec ();
			}
		};

		template<typename T, bool HasPKey = HasPKey<T>>
		class AdaptUpdate
		{
			const QSqlDatabase DB_;
			const QString Table_;

			std::function<void (T)> Updater_;
		public:
			AdaptUpdate (const QSqlDatabase& db, const CachedFieldsData& data) noexcept
			: DB_ { db }
			, Table_ { data.Table_ }
			{
				if constexpr (HasPKey)
				{
					constexpr auto index = FindPKey<T>::result_type::value;

					auto statements = Util::ZipWith<QList> (data.Fields_, data.BoundFields_,
							[] (const QString& s1, const QString& s2) { return s1 + " = " + s2; });
					auto wherePart = statements.takeAt (index);
					const auto& update = "UPDATE " + data.Table_ +
							" SET " + statements.join (", ") +
							" WHERE " + wherePart;

					const auto updateQuery = std::make_shared<QSqlQuery> (db);
					updateQuery->prepare (update);
					Updater_ = MakeInserter<T> (data, updateQuery, true);
				}
			}

			template<bool B = HasPKey>
			std::enable_if_t<B> operator() (const T& seq)
			{
				Updater_ (seq);
			}

			template<typename SL, typename SR, ExprType WType, typename WL, typename WR>
			int operator() (const AssignList<SL, SR>& set, const ExprTree<WType, WL, WR>& where)
			{
				static_assert (!ExprTree<WType, WL, WR>::template HasAdditionalTables<T> (),
						"joins in update statements are not supported by SQL");

				const auto& [setClause, setBinder, setLast] = HandleExprTree<T> (set);
				const auto& [whereClause, whereBinder, _] = HandleExprTree<T> (where, setLast);

				const auto& update = "UPDATE " + Table_ +
						" SET " + setClause +
						" WHERE " + whereClause;

				QSqlQuery query { DB_ };
				query.prepare (update);
				setBinder (query);
				whereBinder (query);
				if (!query.exec ())
				{
					DBLock::DumpError (query);
					throw QueryException ("update query execution failed", std::make_shared<QSqlQuery> (query));
				}

				return query.numRowsAffected ();
			}
		};

		template<typename T>
		using ConstraintsDetector = typename T::Constraints;

		template<typename T>
		using ConstraintsType = Util::IsDetected_t<Constraints<>, ConstraintsDetector, T>;

		template<typename T>
		struct ExtractConstraintFields;

		template<int... Fields>
		struct ExtractConstraintFields<UniqueSubset<Fields...>>
		{
			QString operator() (const CachedFieldsData& data) const noexcept
			{
				return "UNIQUE (" + QStringList { data.Fields_.value (Fields)... }.join (", ") + ")";
			}
		};

		template<int... Fields>
		struct ExtractConstraintFields<PrimaryKey<Fields...>>
		{
			QString operator() (const CachedFieldsData& data) const noexcept
			{
				return "PRIMARY KEY (" + QStringList { data.Fields_.value (Fields)... }.join (", ") + ")";
			}
		};

		template<typename... Args>
		QStringList GetConstraintsStringList (Constraints<Args...>, const CachedFieldsData& data) noexcept
		{
			return { ExtractConstraintFields<Args> {} (data)... };
		}

		template<typename ImplFactory, typename T, size_t... Indices>
		QList<QString> GetTypes (std::index_sequence<Indices...>) noexcept
		{
			return { Type2Name<ImplFactory, ValueAtC_t<T, Indices>> {} ()... };
		}

		template<typename ImplFactory, typename T>
		QString AdaptCreateTable (const CachedFieldsData& data) noexcept
		{
			const auto& types = GetTypes<ImplFactory, T> (SeqIndices<T>);

			const auto& constraints = GetConstraintsStringList (ConstraintsType<T> {}, data);
			const auto& constraintsStr = constraints.isEmpty () ?
					QString {} :
					(", " + constraints.join (", "));

			const auto& statements = Util::ZipWith<QList> (types, data.Fields_,
					[] (const QString& type, const QString& field) { return field + " " + type; });
			return "CREATE TABLE " +
					data.Table_ +
					" (" +
					statements.join (", ") +
					constraintsStr +
					");";
		}
	}

	template<auto... Ptrs>
	InsertAction::Replace::FieldsType<Ptrs...>::operator InsertAction::Replace () const
	{
		return { { detail::BuildCachedFieldsData<MemberPtrStruct_t<Ptrs>> ().Fields_.value (detail::FieldIndex<Ptrs> ())... } };
	}

	template<typename Seq>
	InsertAction::Replace::PKeyType<Seq>::operator InsertAction::Replace () const
	{
		static_assert (detail::HasPKey<Seq>, "Sequence does not have any primary keys");
		return { { detail::GetFieldName<Seq, detail::FindPKey<Seq>::result_type::value> () } };
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
		const auto& cachedData = detail::BuildCachedFieldsData<T> ();

		if (!db.tables ().contains (cachedData.Table_, Qt::CaseInsensitive))
			RunTextQuery (db, detail::AdaptCreateTable<ImplFactory, T> (cachedData));

		ImplFactory factory;

		return
		{
			{ db, cachedData, factory },
			{ db, cachedData },
			{ db, cachedData },

			{ db, cachedData, factory },
			{ db, cachedData, factory },
			{ db, cachedData }
		};
	}

	template<typename T>
	using ObjectInfo_ptr = std::shared_ptr<ObjectInfo<T>>;

	template<typename T, typename ImplFactory = SQLiteImplFactory>
	ObjectInfo_ptr<T> AdaptPtr (const QSqlDatabase& db)
	{
		return std::make_shared<ObjectInfo<T>> (Adapt<T, ImplFactory> (db));
	}

	namespace detail
	{
		template<size_t Idx, typename Tuple>
		using UnderlyingObject_t = typename std::decay_t<std::tuple_element_t<Idx, Tuple>>::element_type::ObjectType_t;

		template<typename ImplFactory, typename Tuple, size_t... Idxs>
		void AdaptPtrs (const QSqlDatabase& db, Tuple& tuple, std::index_sequence<Idxs...>)
		{
			((std::get<Idxs> (tuple) = AdaptPtr<UnderlyingObject_t<Idxs, Tuple>, ImplFactory> (db)), ...);
		}
	}

	template<typename ImplFactory, typename Tuple>
	void AdaptPtrs (const QSqlDatabase& db, Tuple& tuple)
	{
		detail::AdaptPtrs<ImplFactory> (db, tuple, std::make_index_sequence<std::tuple_size_v<Tuple>> {});
	}
}
}
}
