/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loghandler.h"
#include <functional>
#include <QFile>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/lmp/ilocalcollection.h>
#include <util/sll/slotclosure.h>
#include <util/sll/views.h>
#include <util/sll/qtutil.h>
#include <util/sll/functor.h>
#include <util/sll/monad.h>
#include <util/sll/monadplus.h>
#include "parser.h"
#include "tracksselectordialog.h"

namespace LC
{
namespace LMP
{
namespace PPL
{
	namespace
	{
		class LocalCollectionScrobbler : public QObject
									   , public Media::IAudioScrobbler
		{
			ILocalCollection * const Coll_;
		public:
			LocalCollectionScrobbler (ILocalCollection*, QObject*);

			bool SupportsFeature (Feature feature) const override;
			QString GetServiceName () const override;
			void NowPlaying (const Media::AudioInfo& audio) override;
			void SendBackdated (const BackdatedTracks_t& list) override;
			void PlaybackStopped () override;
			void LoveCurrentTrack () override;
			void BanCurrentTrack () override;
		};

		LocalCollectionScrobbler::LocalCollectionScrobbler (ILocalCollection* coll, QObject *parent)
		: QObject { parent }
		, Coll_ { coll }
		{
		}

		bool LocalCollectionScrobbler::SupportsFeature (Media::IAudioScrobbler::Feature) const
		{
			return false;
		}

		QString LocalCollectionScrobbler::GetServiceName () const
		{
			return tr ("Local collection");
		}

		void LocalCollectionScrobbler::NowPlaying (const Media::AudioInfo&)
		{
		}

		template<typename T, typename F>
		std::optional<T> FindAttrRelaxed (const QList<T>& items, const QString& attr, F&& attrGetter)
		{
			auto finder = [&] (const auto& checker) -> std::optional<T>
			{
				const auto pos = std::ranges::find_if (items,
						[&] (const auto& item) { return checker (std::invoke (attrGetter, item)); });
				if (pos == items.end ())
					return {};
				return *pos;
			};

			auto normalize = [] (const QString& str)
			{
				return str.toLower ().remove (' ');
			};
			const auto& attrNorm = normalize (attr);

			return Util::Msum ({
						finder ([&] (const QString& other) { return attr == other; }),
						finder ([&] (const QString& other) { return attr.compare (other, Qt::CaseInsensitive); }),
						finder ([&] (const QString& other) { return normalize (other) == attrNorm; })
					});
		}

		std::optional<Collection::Artist> FindArtist (const Collection::Artists_t& artists, const QString& name)
		{
			return FindAttrRelaxed (artists, name, &Collection::Artist::Name_);
		}

		std::optional<Collection::Album_ptr> FindAlbum (const QList<Collection::Album_ptr>& albums, const QString& name)
		{
			return FindAttrRelaxed (albums, name, [] (const auto& albumPtr) { return albumPtr->Name_; });
		}

		std::optional<Collection::Track> FindTrack (const QList<Collection::Track>& tracks, const QString& name)
		{
			return FindAttrRelaxed (tracks, name, &Collection::Track::Name_);
		}

		std::optional<int> FindMetadata (const Collection::Artists_t& artists, const Media::AudioInfo& info)
		{
			using Util::operator>>;
			using Util::operator*;

			const auto& maybeTrack = FindArtist (artists, info.Artist_) >>
					[&] (const auto& artist) { return FindAlbum (artist.Albums_, info.Album_); } >>
					[&] (const auto& album) { return FindTrack (album->Tracks_, info.Title_); };
			return &Collection::Track::ID_ * maybeTrack;
		}

		void LocalCollectionScrobbler::SendBackdated (const Media::IAudioScrobbler::BackdatedTracks_t& list)
		{
			const auto& artists = Coll_->GetAllArtists ();

			for (const auto& item : list)
				if (const auto& maybeTrackId = FindMetadata (artists, item.first))
					Coll_->RecordPlayedTrack (*maybeTrackId, item.second);
		}

		void LocalCollectionScrobbler::PlaybackStopped ()
		{
		}

		void LocalCollectionScrobbler::LoveCurrentTrack ()
		{
		}

		void LocalCollectionScrobbler::BanCurrentTrack ()
		{
		}
	}

	LogHandler::LogHandler (const QString& logPath,
			ILocalCollection *coll, IPluginsManager *ipm, QObject *parent)
	: QObject { parent }
	, Collection_ { coll }
	{
		QFile file { logPath };
		if (!file.open (QIODevice::ReadOnly))
			return;

		const auto& tracks = ParseData (file.readAll ());
		if (tracks.isEmpty ())
		{
			deleteLater ();
			return;
		}

		auto local = new LocalCollectionScrobbler { coll, this };

		const auto& scrobblers = QList<Media::IAudioScrobbler*> { local } +
				Util::Filter (ipm->GetAllCastableTo<Media::IAudioScrobbler*> (),
						[] (Media::IAudioScrobbler *scrob)
						{
							return scrob->SupportsFeature (Media::IAudioScrobbler::Feature::Backdating);
						});

		const auto dia = new TracksSelectorDialog { tracks, scrobblers };
		dia->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[dia, scrobblers, logPath]
			{
				QHash<Media::IAudioScrobbler*, Media::IAudioScrobbler::BackdatedTracks_t> scrob2tracks;

				for (const auto& track : dia->GetSelectedTracks ())
					for (const auto& [scrobbler, shouldSubmit] : Util::Views::Zip (scrobblers, track.Scrobbles_))
						if (shouldSubmit)
							scrob2tracks [scrobbler] << track.Track_;

				for (const auto& pair : Util::Stlize (scrob2tracks))
					pair.first->SendBackdated (pair.second);

				QFile::remove (logPath);
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};

		connect (dia,
				SIGNAL (finished (int)),
				this,
				SLOT (deleteLater ()));
	}
}
}
}
