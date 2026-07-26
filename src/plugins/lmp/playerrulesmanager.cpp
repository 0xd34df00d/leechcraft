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
#include <util/xpc/anutil.h>
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
		std::optional<AN::ValueMatcher> GetMatcher (const Entity& info, const QString& field)
		{
			const auto& var = info.Additional_.value (field);
			if (var.isNull ())
				return {};

			if (!var.canConvert<AN::ValueMatcher> ())
			{
				qWarning () << var << "at" << field << "is not a matcher";
				return {};
			}

			return var.value<AN::ValueMatcher> ();
		}

		struct Matcher
		{
			const std::optional<AN::ValueMatcher> LengthMatcher_;
			const std::optional<AN::ValueMatcher> ArtistMatcher_;
			const std::optional<AN::ValueMatcher> AlbumMatcher_;
			const std::optional<AN::ValueMatcher> TitleMatcher_;
			const std::optional<AN::ValueMatcher> UrlMatcher_;

			const bool NonEmpty_ = LengthMatcher_ || ArtistMatcher_ || AlbumMatcher_ || TitleMatcher_ || UrlMatcher_;

			Matcher (const Entity& info)
			: LengthMatcher_ { GetMatcher (info, AN::Field::MediaLength) }
			, ArtistMatcher_ { GetMatcher (info, AN::Field::MediaArtist) }
			, AlbumMatcher_ { GetMatcher (info, AN::Field::MediaAlbum) }
			, TitleMatcher_ { GetMatcher (info, AN::Field::MediaTitle) }
			, UrlMatcher_ { GetMatcher (info, AN::Field::MediaPlayerURL) }
			{
			}

			bool operator() (const MediaInfo& info) const
			{
				auto match = []<typename T> (const std::optional<AN::ValueMatcher>& matcher, const T& value)
				{
					const auto type = static_cast<QMetaType::Type> (QMetaType::fromType<T> ().id ());
					return !matcher || Util::AN::Matches (value, type, *matcher);
				};

				if (!match (LengthMatcher_, info.Length_) ||
						!match (ArtistMatcher_, info.Artist_) ||
						!match (AlbumMatcher_, info.Album_) ||
						!match (TitleMatcher_, info.Title_))
					return false;

				auto url = info.Additional_.value ("URL").toUrl ();
				if (url.isEmpty ())
					url = QUrl::fromLocalFile (info.LocalPath_);

				if (!url.isEmpty () && !match (UrlMatcher_, url))
					return false;

				return NonEmpty_;
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
