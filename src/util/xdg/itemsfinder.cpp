/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsfinder.h"
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include <QtConcurrentRun>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include "xdg.h"
#include "item.h"

namespace LC::Util::XDG
{
	ItemsFinder::ItemsFinder (const ICoreProxy_ptr& proxy,
			const QList<Type>& types, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, Types_ { types }
	{
		QTimer::singleShot (1000, this, SLOT (update ()));
	}

	bool ItemsFinder::IsReady () const
	{
		return IsReady_;
	}

	Cat2Items_t ItemsFinder::GetItems () const
	{
		return Items_;
	}

	Item_ptr ItemsFinder::FindItem (const QString& id) const
	{
		for (const auto& list : Items_)
		{
			const auto pos = std::find_if (list.begin (), list.end (),
					[&id] (const Item_ptr& item) { return item->GetPermanentID () == id; });
			if (pos != list.end ())
				return *pos;
		}

		return {};
	}

	namespace
	{
		using Cat2ID2Item_t = QHash<QString, QHash<QString, Item_ptr>>;

		Cat2ID2Item_t ItemsList2Map (const Cat2Items_t& items)
		{
			Cat2ID2Item_t result;

			for (const auto& pair : Util::Stlize (items))
			{
				auto& map = result [pair.first];
				for (const auto& item : pair.second)
					map [item->GetPermanentID ()] = item;
			}

			return result;
		}

		Cat2Items_t ItemsMap2List (const Cat2ID2Item_t& items)
		{
			Cat2Items_t result;

			for (const auto& pair : Util::Stlize (items))
				std::copy (pair.second.begin (), pair.second.end (),
						std::back_inserter (result [pair.first]));

			return result;
		}

		QStringList ScanDir (const QString& path)
		{
			const auto& infos = QDir (path).entryInfoList ({ QStringLiteral ("*.desktop") },
						QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);

			return Util::ConcatMap (infos,
					[] (const QFileInfo& info)
					{
						return info.isDir () ?
								ScanDir (info.absoluteFilePath ()) :
								QStringList { info.absoluteFilePath () };
					});
		}

		Cat2ID2Item_t FindAndParse (const QList<Type>& types)
		{
			Cat2ID2Item_t result;

			QStringList paths;
			for (const auto& dir : ToPaths (types))
				paths << ScanDir (dir);

			for (const auto& path : paths)
			{
				Item_ptr item;
				try
				{
					item = Item::FromDesktopFile (path);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error parsing"
							<< path
							<< e.what ();
					continue;
				}

				if (!item->IsValid ())
				{
					qWarning () << Q_FUNC_INFO
							<< "invalid item"
							<< path;
					continue;
				}

				for (const auto& cat : item->GetCategories ())
					if (!cat.startsWith ("X-"_ql))
						result [cat] [item->GetPermanentID ()] = item;
			}

			return result;
		}

		template<typename T>
		struct DiffResult
		{
			std::decay_t<T> Added_;
			std::decay_t<T> Removed_;
			std::decay_t<T> Intersection_;

			DiffResult (T&& oldCont, T&& newCont)
			{
				std::set_difference (oldCont.begin (), oldCont.end (),
						newCont.begin (), newCont.end (),
						std::back_inserter (Removed_));
				std::set_difference (newCont.begin (), newCont.end (),
						oldCont.begin (), oldCont.end (),
						std::back_inserter (Added_));

				std::set_intersection (oldCont.begin (), oldCont.end (),
						newCont.begin (), newCont.end (),
						std::back_inserter (Intersection_));
			}

			bool HasChanges () const
			{
				return !Removed_.isEmpty () || !Added_.isEmpty ();
			}
		};

		template<typename Container>
		DiffResult<Container> CalcDiff (Container&& oldCont, Container&& newCont)
		{
			return { std::forward<Container> (oldCont), std::forward<Container> (newCont) };
		}

		std::optional<Cat2Items_t> Merge (const Cat2Items_t& existing, Cat2ID2Item_t result)
		{
			auto ourItems = ItemsList2Map (existing);

			using std::swap;

			const auto& diffCats = CalcDiff (Util::Sorted (existing.keys ()),
					Util::Sorted (result.keys ()));

			for (const auto& removed : diffCats.Removed_)
				ourItems.remove (removed);
			for (const auto& added : diffCats.Added_)
				swap (ourItems [added], result [added]);

			bool changed = diffCats.HasChanges ();

			for (const auto& cat : diffCats.Intersection_)
			{
				auto& ourList = ourItems [cat];
				auto& newList = result [cat];
				const auto& diffItems = CalcDiff (Util::Sorted (ourList.keys ()),
						Util::Sorted (newList.keys ()));

				changed = changed || diffItems.HasChanges ();

				for (const auto& removed : diffItems.Removed_)
					ourList.remove (removed);
				for (const auto& added : diffItems.Added_)
					swap (ourList [added], newList [added]);

				for (const auto& existing : diffItems.Intersection_)
					if (*ourList [existing] != *newList [existing])
					{
						swap (ourList [existing], newList [existing]);
						changed = true;
					}
			}

			if (!changed)
				return {};

			return ItemsMap2List (ourItems);
		}
	}

	void ItemsFinder::update ()
	{
		if (IsScanning_)
			return;

		IsScanning_ = true;

		Util::Sequence (this, QtConcurrent::run (FindAndParse, Types_)) >>
				[this] (const Cat2ID2Item_t& result)
				{
					return QtConcurrent::run (Merge, Items_, result);
				} >>
				[this] (const std::optional<Cat2Items_t>& result)
				{
					IsScanning_ = false;
					IsReady_ = true;

					if (result)
					{
						Items_ = *result;
						emit itemsListChanged ();
					}
				};
	}
}
