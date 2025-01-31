/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <random>
#include <memory>
#include <atomic>
#include <QObject>

#ifdef ENABLE_MPRIS
#include <qdbuscontext.h>
#endif

#include <interfaces/media/iradiostation.h>
#include "engine/audiosource.h"
#include "util/lmp/mediainfo.h"
#include "sortingcriteria.h"
#include "nativeplaylist.h"

class QModelIndex;
class QStandardItem;
class QAbstractItemModel;
class QStandardItemModel;

typedef QPair<QString, QString> StringPair_t;

namespace LC
{
namespace LMP
{
	class SourceObject;
	class Output;
	class Path;
	class PlayerRulesManager;
	struct MediaInfo;
	enum class SourceError;
	enum class SourceState;

	class Player : public QObject
#ifdef ENABLE_MPRIS
				 , public QDBusContext
#endif
	{
		Q_OBJECT

		QStandardItemModel *PlaylistModel_;
		SourceObject *Source_;
		Output *Output_;
		Path *Path_;

		mutable std::mt19937 PRG_;

		QList<AudioSource> CurrentQueue_;
		QHash<AudioSource, QStandardItem*> Items_;
		QHash<QString, QList<QStandardItem*>> AlbumRoots_;

		std::function<void ()> PlaybackStopHandler_;

		AudioSource CurrentStopSource_;
		QList<AudioSource> CurrentOneShotQueue_;

		Media::IRadioStation_ptr CurrentStation_;
		QHash<QUrl, MediaInfo> Url2Info_;

		PlayerRulesManager * const RulesManager_;

		MediaInfo LastPhononMediaInfo_;

		bool FirstPlaylistRestore_ = true;
		bool IgnoreNextSaves_;
	public:
		enum class PlayMode
		{
			Sequential,
			Shuffle,
			ShuffleAlbums,
			ShuffleArtists,
			RepeatTrack,
			RepeatAlbum,
			RepeatWhole
		};
	private:
		PlayMode PlayMode_ = PlayMode::Sequential;

		struct ResolveJobResult;

		struct Sorter
		{
			QList<SortingCriteria> Criteria_;

			Sorter ();
			bool operator() (const MediaInfo&, const MediaInfo&) const;
		} Sorter_;
	public:
		enum Role
		{
			IsCurrent = Qt::UserRole + 1,
			IsStop,
			IsAlbum,
			IsRadioItem,
			Source,
			Info,
			AlbumArt,
			AlbumLength,
			OneShotPos,
			MatchingRules
		};

		enum EnqueueFlag
		{
			EnqueueNone = 0x0,
			EnqueueSort = 0x1,
			EnqueueReplace = 0x2
		};

		Q_DECLARE_FLAGS (EnqueueFlags, EnqueueFlag)

		Player (QObject* = 0);

		void InitWithOtherPlugins ();

		QAbstractItemModel* GetPlaylistModel () const;
		SourceObject* GetSourceObject () const;
		Output* GetAudioOutput () const;
		Path* GetPath () const;

		PlayMode GetPlayMode () const;
		void SetPlayMode (PlayMode);

		SourceState GetState () const;

		QList<SortingCriteria> GetSortingCriteria () const;
		void SetSortingCriteria (const QList<SortingCriteria>&);

		void PrepareURLInfo (const QUrl&, const MediaInfo&);
		void Enqueue (const QStringList&, EnqueueFlags = EnqueueSort);
		void Enqueue (const QList<AudioSource>&, EnqueueFlags = EnqueueSort);
		QList<AudioSource> GetQueue () const;
		QList<AudioSource> GetIndexSources (const QModelIndex&) const;
		QModelIndex GetSourceIndex (const AudioSource&) const;

		void Dequeue (const QModelIndex&);
		void Dequeue (const QList<AudioSource>&);

		AudioSource GetCurrentStopSource () const;
		void SetStopAfter (const QModelIndex&);

		void RestorePlayState ();
		void SavePlayState (bool ignoreNext);

		void AddToOneShotQueue (const QModelIndex&);
		void AddToOneShotQueue (const AudioSource&);
		void RemoveFromOneShotQueue (const QModelIndex&);
		void OneShotMoveUp (const QModelIndex&);
		void OneShotMoveDown (const QModelIndex&);
		int GetOneShotQueueSize () const;

		void SetRadioStation (Media::IRadioStation_ptr);

		MediaInfo GetCurrentMediaInfo () const;
		QString GetCurrentAAPath () const;

		MediaInfo GetMediaInfo (const AudioSource&) const;

		NativePlaylist_t GetAsNativePlaylist () const;
		void SetNativePlaylist (NativePlaylist_t);
	private:
		MediaInfo GetPhononMediaInfo () const;
		void AddToPlaylistModel (QList<AudioSource>, bool sort, bool clear);

		void SetStopAfter (const AudioSource&);
		bool HandleCurrentStop (const AudioSource&);

		void RemoveFromOneShotQueue (const AudioSource&);

		QStandardItem* FindRadioItem () const;

		void UnsetRadio ();

		void EmitStateChange (SourceState);

		template<typename T>
		AudioSource GetRandomBy (AudioSources_t::const_iterator,
				std::function<T (AudioSources_t::const_iterator, AudioSources_t)>) const;

		AudioSource GetNextSource (const AudioSource&);

		void MarkAsCurrent (QStandardItem*);

		void ContinueAfterSorted (const ResolveJobResult&);

		void SaveOnLoadPlaylist () const;
	public slots:
		void play (const QModelIndex&);
		void previousTrack ();
		void nextTrack ();
		void togglePause ();
		void setPause ();
		void stop ();
		void stopAfterCurrent ();
		void clear ();
		void shufflePlaylist ();
	private slots:
		void restorePlaylist ();
		void handleStationError (const QString&);
		void handleRadioStream (const QUrl&, const Media::AudioInfo&);
		void handleGotRadioPlaylist (const QString&, const QString&);
		void handleGotAudioInfos (const QList<Media::AudioInfo>&);
		void postPlaylistCleanup (const QString&);
		void handleUpdateSourceQueue (const std::shared_ptr<std::atomic_bool>&);
		void handlePlaybackFinished ();
		void handleStateChanged (SourceState, SourceState);
		void handleCurrentSourceChanged (const AudioSource&);
		void handleMetadata ();

		void refillPlaylist ();
	signals:
		void songChanged (const MediaInfo&);
		void songInfoUpdated (const MediaInfo&);
		void indexChanged (const QModelIndex&);
		void insertedAlbum (const QModelIndex&);

		void playModeChanged (Player::PlayMode);
		void bufferStatusChanged (int);

		void playerAvailable (bool);

		void aboutToStopInternally ();

		void shouldClearFiltering ();

		void playlistRestored ();

		void currentStopSourceChanged ();
	};
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::LMP::Player::EnqueueFlags)
