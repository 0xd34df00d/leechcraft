/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <atomic>
#include <type_traits>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMutex>
#include <QWaitCondition>
#include <util/sll/util.h>
#include <util/threads/mutex.h>
#include "interfaces/lmp/isourceobject.h"
#include "interfaces/lmp/ipath.h"
#include "util/lmp/gstutil.h"
#include "audiosource.h"
#include "pathelement.h"

typedef struct _GstElement GstElement;
typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;
typedef struct _GstBus GstBus;

typedef std::shared_ptr<GstMessage> GstMessage_ptr;

namespace LC
{
namespace LMP
{
	class AudioSource;
	class Path;
	class MsgPopThread;

	enum class SourceError
	{
		MissingPlugin,
		SourceNotFound,
		CannotOpenSource,
		InvalidSource,
		DeviceBusy,
		Other
	};

	enum class Category
	{
		Music,
		Notification
	};

	template<typename T>
	class HandlerContainer : public QObject
	{
		QMap<QObject*, QList<T>> Dependents_;
	public:
		void AddHandler (const T& handler, QObject *dependent)
		{
			Dependents_ [dependent] << handler;

			connect (dependent,
					&QObject::destroyed,
					this,
					[dependent, this] { Dependents_.remove (dependent); });
		}

		template<typename Reducer, typename... Args>
		auto operator() (Reducer r, decltype (r (T {} (Args {}...), T {} (Args {}...))) init, Args... args) -> decltype (r (T {} (args...), T {} (args...)))
		{
			for (const auto& sublist : Dependents_)
				for (const auto& item : sublist)
					init = r (init, item (args...));

			return init;
		}

		template<typename... Args>
		auto operator() (Args... args) -> std::enable_if_t<std::is_same<void, decltype (T {} (args...))>::value>
		{
			for (const auto& sublist : Dependents_)
				for (const auto& item : sublist)
					item (args...);
		}
	};

	class SourceObject : public QObject
					   , public ISourceObject
	{
		Q_OBJECT

		friend class Path;
		friend class MsgPopThread;

		std::shared_ptr<GstElement> Dec_;

		Path *Path_ = nullptr;

		AudioSource CurrentSource_;

		Util::Mutex NextSrcMutex_;
		QWaitCondition NextSrcWC_;
		AudioSource NextSource_ GUARDED_BY (NextSrcMutex_);

		AudioSource ActualSource_;

		bool IsSeeking_ = false;

		qint64 LastCurrentTime_ = -1;

		uint PrevSoupRank_ = 0;

		QMutex BusDrainMutex_;
		QWaitCondition BusDrainWC_;
		bool IsDrainingMsgs_ = false;

		std::shared_ptr<MsgPopThread> PopThread_;
		GstUtil::TagMap_t Metadata_;

		HandlerContainer<SyncHandler_f> SyncHandlers_;
		HandlerContainer<AsyncHandler_f> AsyncHandlers_;

		const Util::DefaultScopeGuard PathStateGuard_;

		std::vector<Util::DefaultScopeGuard> SignalGuards_;
	public:
		enum class Metadata
		{
			Artist,
			Album,
			Title,
			Genre,
			Tracknumber,
			NominalBitrate,
			MinBitrate,
			MaxBitrate
		};
	private:
		SourceState OldState_ = SourceState::Stopped;
	public:
		SourceObject (Category, QObject* = 0);

		SourceObject (const SourceObject&) = delete;
		SourceObject& operator= (const SourceObject&) = delete;

		QObject* GetQObject ();

		bool IsSeekable () const;

		SourceState GetState () const;
		void SetState (SourceState);

		QString GetErrorString () const;

		QString GetMetadata (Metadata) const;

		qint64 GetCurrentTime ();
		qint64 GetRemainingTime () const;
		qint64 GetTotalTime () const;
		void Seek (qint64);

		AudioSource GetActualSource () const;
		AudioSource GetCurrentSource () const;
		void SetCurrentSource (const AudioSource&);
		void PrepareNextSource (const AudioSource&);

		void Play ();
		void Pause ();
		void Stop ();

		void Clear ();
		void ClearQueue ();

		void HandleAboutToFinish ();

		void SetupSource ();

		void AddToPath (Path*);
		void SetSink (GstElement*);

		void AddSyncHandler (const SyncHandler_f&, QObject*);
		void AddAsyncHandler (const AsyncHandler_f&, QObject*);
	private:
		void HandleErrorMsg (GstMessage*);
		void HandleTagMsg (GstMessage*);
		void HandleBufferingMsg (GstMessage*);
		void HandleStateChangeMsg (GstMessage*);
		void HandleEosMsg (GstMessage*);
		void HandleStreamStatusMsg (GstMessage*);
		void HandleWarningMsg (GstMessage*);

		int HandleSyncMessage (GstBus*, GstMessage*);
	private slots:
		void handleMessage (GstMessage_ptr);
		void updateTotalTime ();
		void handleTick ();

		void setActualSource (const AudioSource&);
	signals:
		void stateChanged (SourceState, SourceState);
		void currentSourceChanged (const AudioSource&);
		void aboutToFinish (std::shared_ptr<std::atomic_bool>);
		void finished ();
		void metaDataChanged ();
		void bufferStatus (int);
		void totalTimeChanged (qint64);

		void tick (qint64);

		void seeked (qint64);

		void error (const QString&, SourceError);
	};
}
}
