/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playerrulesmanager.h"
#include <QUrl>
#include <QStandardItemModel>
#include <QtConcurrentMap>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/ianrulesstorage.h>
#include <interfaces/an/constants.h>
#include <util/structuresops.h>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "player.h"

Q_DECLARE_METATYPE (QList<LC::Entity>)

namespace LC
{
namespace LMP
{
	PlayerRulesManager::PlayerRulesManager (QStandardItemModel *model, QObject *parent)
	: QObject { parent }
	, Model_ { model }
	{
		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (insertRows (QModelIndex, int, int)));
		connect (model,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (removeRows (QModelIndex, int, int)));
		connect (model,
				SIGNAL (modelReset ()),
				this,
				SLOT (handleReset ()));
	}

	namespace
	{
		struct StringMatcher
		{
			const QString Value_;

			bool operator() (const ANStringFieldValue& value) const
			{
				return value.Rx_.exactMatch (Value_) == value.Contains_;
			}

			template<typename T>
			bool operator() (const T&) const
			{
				return false;
			}
		};

		struct IntMatcher
		{
			const int Value_;

			bool operator() (const ANIntFieldValue& value) const
			{
				return ((value.Ops_ & ANIntFieldValue::OEqual) && Value_ == value.Boundary_) ||
						((value.Ops_ & ANIntFieldValue::OGreater) && Value_ > value.Boundary_) ||
						((value.Ops_ & ANIntFieldValue::OLess) && Value_ < value.Boundary_);
			}

			template<typename T>
			bool operator() (const T&) const
			{
				return false;
			}
		};

		struct Matcher
		{
			const QVariant LengthField_;
			const QVariant ArtistField_;
			const QVariant AlbumField_;
			const QVariant TitleField_;
			const QVariant PlayerUrlField_;

			Matcher (const Entity& info)
			: LengthField_ { info.Additional_.value (AN::Field::MediaLength) }
			, ArtistField_ { info.Additional_.value (AN::Field::MediaArtist) }
			, AlbumField_ { info.Additional_.value (AN::Field::MediaAlbum) }
			, TitleField_ { info.Additional_.value (AN::Field::MediaTitle) }
			, PlayerUrlField_ { info.Additional_.value (AN::Field::MediaPlayerURL) }
			{
			}

			template<typename U, typename T>
			static bool MatchField (const QVariant& wrapped, const T& value, bool& hadSome)
			{
				if (wrapped.isNull ())
					return true;

				hadSome = true;
				auto variant = wrapped.value<ANFieldValue> ();
				return std::visit (U { value }, variant);
			}

			bool operator() (const MediaInfo& info) const
			{
				bool hadSome = false;
				auto matchStr = [&hadSome] (const QVariant& var, const QString& value)
				{
					return MatchField<StringMatcher> (var, value, hadSome);
				};
				auto matchInt = [&hadSome] (const QVariant& var, int value)
				{
					return MatchField<IntMatcher> (var, value, hadSome);
				};

				if (!matchInt (LengthField_, info.Length_) ||
						!matchStr (ArtistField_, info.Artist_) ||
						!matchStr (AlbumField_, info.Album_) ||
						!matchStr (TitleField_, info.Title_))
					return false;

				auto url = info.Additional_.value ("URL").toUrl ();
				if (url.isEmpty ())
					url = QUrl::fromLocalFile (info.LocalPath_);

				if (!matchStr (PlayerUrlField_, url.toEncoded ()))
					return false;

				return hadSome;
			}
		};

		void ReapplyRules (const QList<QStandardItem*>& items, const QList<Entity>& rules)
		{
			using RulesMap_t = QHash<QStandardItem*, QList<Entity>>;
			RulesMap_t newRules;

			if (!rules.isEmpty ())
			{
				const auto& infoCache = Util::Map (items,
						[] (QStandardItem *item)
						{
							return qMakePair (item, item->data (Player::Role::Info).value<MediaInfo> ());
						});

				for (const auto& rule : rules)
				{
					const Matcher matcher { rule };
					for (const auto& pair : infoCache)
						if (matcher (pair.second))
							newRules [pair.first] << rule;
				}
			}

			for (const auto item : items)
			{
				const auto& matching = newRules.value (item);
				const auto& current = item->data (Player::Role::MatchingRules).value<QList<Entity>> ();
				if (current != matching)
					item->setData (matching.isEmpty () ? QVariant {} : QVariant::fromValue (matching),
							Player::Role::MatchingRules);
			}
		}
	}

	void PlayerRulesManager::InitializePlugins ()
	{
		const auto plugMgr = GetProxyHolder ()->GetPluginsManager ();
		for (auto storage : plugMgr->GetAllCastableRoots<IANRulesStorage*> ())
			connect (storage,
					SIGNAL (rulesChanged ()),
					this,
					SLOT (handleRulesChanged ()));

		refillRules ();

		ReapplyRules (ManagedItems_, Rules_);
	}

	void PlayerRulesManager::insertRows (const QModelIndex& parent, int first, int last)
	{
		QList<QStandardItem*> list;
		for (int i = first; i <= last; ++i)
			list << Model_->itemFromIndex (Model_->index (i, 0, parent));

		QList<QStandardItem*> newItems;
		for (int i = 0; i < list.size (); ++i)
		{
			const auto item = list.at (i);

			if (!item->data (Player::IsAlbum).toBool ())
				newItems << item;

			for (int j = 0; j < item->rowCount (); ++j)
				list << item->child (j);
		}

		ReapplyRules (newItems, Rules_);

		ManagedItems_ += newItems;
	}

	void PlayerRulesManager::removeRows (const QModelIndex& parent, int first, int last)
	{
		QList<QStandardItem*> list;
		for (int i = first; i <= last; ++i)
			list << Model_->itemFromIndex (Model_->index (i, 0, parent));

		for (int i = 0; i < list.size (); ++i)
		{
			const auto item = list.at (i);

			ManagedItems_.removeOne (item);

			for (int j = 0; j < item->rowCount (); ++j)
				list << item->child (j);
		}
	}

	void PlayerRulesManager::handleReset ()
	{
		ManagedItems_.clear ();
		if (const auto rc = Model_->rowCount ())
			insertRows ({}, 0, rc - 1);
	}

	void PlayerRulesManager::refillRules ()
	{
		Rules_.clear ();

		const auto plugMgr = GetProxyHolder ()->GetPluginsManager ();
		for (auto storage : plugMgr->GetAllCastableTo<IANRulesStorage*> ())
			Rules_ += storage->GetAllRules (AN::CatMediaPlayer);
	}

	void PlayerRulesManager::handleRulesChanged ()
	{
		refillRules ();
		ReapplyRules (ManagedItems_, Rules_);
	}
}
}
