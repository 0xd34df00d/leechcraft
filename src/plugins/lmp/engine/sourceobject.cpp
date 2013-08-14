/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "sourceobject.h"
#include <memory>
#include <QtDebug>
#include <QTimer>
#include <gst/gst.h>
#include "audiosource.h"
#include "path.h"
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		gboolean CbBus (GstBus *bus, GstMessage *message, gpointer data)
		{
			auto src = static_cast<SourceObject*> (data);

			switch (GST_MESSAGE_TYPE (message))
			{
			case GST_MESSAGE_ERROR:
				src->HandleErrorMsg (message);
				break;
			case GST_MESSAGE_TAG:
			case GST_MESSAGE_NEW_CLOCK:
			case GST_MESSAGE_ASYNC_DONE:
				break;
			case GST_MESSAGE_BUFFERING:
				src->HandleBufferingMsg (message);
				break;
			case GST_MESSAGE_STATE_CHANGED:
				src->HandleStateChangeMsg (message);
				break;
			case GST_MESSAGE_DURATION:
				QTimer::singleShot (0,
						src,
						SLOT (updateTotalTime ()));
				break;
			case GST_MESSAGE_ELEMENT:
				src->HandleElementMsg (message);
				break;
			case GST_MESSAGE_EOS:
				src->HandleEosMsg (message);
				break;
			default:
				qDebug () << Q_FUNC_INFO << GST_MESSAGE_TYPE (message);
				break;
			}

			return true;
		}

		gboolean CbAboutToFinish (GstElement*, gpointer data)
		{
			static_cast<SourceObject*> (data)->HandleAboutToFinish ();
			return true;
		}

		gboolean CbSourceChanged (GstElement*, GParamSpec*, gpointer data)
		{
			static_cast<SourceObject*> (data)->SetupSource ();
			return true;
		}

		gboolean CbElement (GstBus *bus, GstMessage *msg, gpointer data)
		{
			auto src = static_cast<SourceObject*> (data);
			src->HandleElementMsg (msg);
			return true;
		}
	}

	SourceObject::SourceObject (QObject *parent)
	: QObject (parent)
#if GST_VERSION_MAJOR < 1
	, Dec_ (gst_element_factory_make ("playbin2", "play"))
#else
	, Dec_ (gst_element_factory_make ("playbin", "play"))
#endif
	, Path_ (nullptr)
	, IsSeeking_ (false)
	, LastCurrentTime_ (-1)
	, PrevSoupRank_ (0)
	, OldState_ (State::Stopped)
	{
		auto bus = gst_pipeline_get_bus (GST_PIPELINE (Dec_));
		gst_bus_add_watch (bus, CbBus, this);
		gst_object_unref (bus);

		g_signal_connect (Dec_, "about-to-finish", G_CALLBACK (CbAboutToFinish), this);
		g_signal_connect (Dec_, "notify::source", G_CALLBACK (CbSourceChanged), this);

		// Seems like it never gets called.
		// g_signal_connect (bus, "sync-message::element", G_CALLBACK (CbElement), this);

		qRegisterMetaType<AudioSource> ("AudioSource");

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTick ()));
		timer->start (1000);
	}

	SourceObject::~SourceObject ()
	{
	}

	bool SourceObject::IsSeekable () const
	{
		std::shared_ptr<GstQuery> query (gst_query_new_seeking (GST_FORMAT_TIME), gst_query_unref);

		if (!gst_element_query (GST_ELEMENT (Dec_), query.get ()))
			return false;

		gboolean seekable = false;
		GstFormat format;
		gint64 start = 0, stop = 0;
		gst_query_parse_seeking (query.get (), &format, &seekable, &start, &stop);
		return seekable;
	}

	SourceObject::State SourceObject::GetState () const
	{
		return OldState_;
	}

	QString SourceObject::GetErrorString () const
	{
// 		return Obj_->errorString ();
		return {};
	}

	QStringList SourceObject::GetMetadata (Metadata field) const
	{
// 		switch (field)
// 		{
// 		case Metadata::Artist:
// 			return Obj_->metaData (Phonon::ArtistMetaData);
// 		case Metadata::Album:
// 			return Obj_->metaData (Phonon::AlbumMetaData);
// 		case Metadata::Title:
// 			return Obj_->metaData (Phonon::TitleMetaData);
// 		case Metadata::Genre:
// 			return Obj_->metaData (Phonon::GenreMetaData);
// 		case Metadata::Tracknumber:
// 			return Obj_->metaData (Phonon::TracknumberMetaData);
// 		}
//
// 		qWarning () << Q_FUNC_INFO
// 				<< "unknown field"
// 				<< static_cast<int> (field);
//
// 		return {};
		return {};
	}

	qint64 SourceObject::GetCurrentTime ()
	{
		if (GetState () != State::Paused)
		{
			auto format = GST_FORMAT_TIME;
			gint64 position = 0;
			gst_element_query_position (GST_ELEMENT (Dec_), &format, &position);
			LastCurrentTime_ = position;
		}
		return LastCurrentTime_ / GST_MSECOND;
	}

	qint64 SourceObject::GetRemainingTime () const
	{
		auto format = GST_FORMAT_TIME;
		gint64 duration = 0;
		if (!gst_element_query_duration (GST_ELEMENT (Dec_), &format, &duration))
			return -1;

		return (duration - LastCurrentTime_) / GST_MSECOND;
	}

	qint64 SourceObject::GetTotalTime () const
	{
		auto format = GST_FORMAT_TIME;
		gint64 duration = 0;
		if (gst_element_query_duration (GST_ELEMENT (Dec_), &format, &duration))
			return duration / GST_MSECOND;
		return -1;
	}

	void SourceObject::Seek (qint64 pos)
	{
		if (OldState_ == State::Playing)
			IsSeeking_ = true;

		gst_element_seek (GST_ELEMENT (Dec_), 1.0, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, pos * GST_MSECOND,
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}

	void SourceObject::SetTransitionTime (int time)
	{
// 		Obj_->setTransitionTime (time);
	}

	AudioSource SourceObject::GetCurrentSource () const
	{
		return CurrentSource_;
	}

	namespace
	{
		uint SetSoupRank (uint rank)
		{
			const auto factory = gst_element_factory_find ("souphttpsrc");
			if (!factory)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot find soup factory";
				return 0;
			}

			const auto oldRank = gst_plugin_feature_get_rank (GST_PLUGIN_FEATURE (factory));
			gst_plugin_feature_set_rank (GST_PLUGIN_FEATURE (factory), rank);
			gst_registry_add_feature (gst_registry_get_default (), GST_PLUGIN_FEATURE (factory));

			return oldRank;
		}
	}

	void SourceObject::SetCurrentSource (const AudioSource& source)
	{
		IsSeeking_ = false;

		CurrentSource_ = source;

		if (source.ToUrl ().scheme ().startsWith ("http"))
			PrevSoupRank_ = SetSoupRank (G_MAXINT);

		const auto& path = source.ToUrl ().toString ();
		g_object_set (G_OBJECT (Dec_), "uri", path.toUtf8 ().constData (), nullptr);
	}

	void SourceObject::PrepareNextSource (const AudioSource& source)
	{
		NextSrcMutex_.lock ();

		qDebug () << Q_FUNC_INFO << source.ToUrl ();
		NextSource_ = source;

		NextSrcWC_.wakeAll ();
		NextSrcMutex_.unlock ();
	}

	void SourceObject::Play ()
	{
		if (CurrentSource_.IsEmpty ())
		{
			qDebug () << Q_FUNC_INFO
					<< "current source is invalid, setting next one";
			if (NextSource_.IsEmpty ())
				return;

			SetCurrentSource (NextSource_);
			NextSource_.Clear ();
		}

		gst_element_set_state (Path_->GetPipeline (), GST_STATE_PLAYING);
	}

	void SourceObject::Pause ()
	{
		gst_element_set_state (Path_->GetPipeline (), GST_STATE_PAUSED);
	}

	void SourceObject::Stop ()
	{
		gst_element_set_state (Path_->GetPipeline (), GST_STATE_READY);
		Seek (0);
	}

	void SourceObject::Clear ()
	{
		ClearQueue ();
		CurrentSource_.Clear ();
		gst_element_set_state (Path_->GetPipeline (), GST_STATE_READY);
	}

	void SourceObject::ClearQueue ()
	{
		NextSource_.Clear ();
	}

	void SourceObject::HandleAboutToFinish ()
	{
		qDebug () << Q_FUNC_INFO;
		emit aboutToFinish ();

		NextSrcMutex_.lock ();
		if (NextSource_.IsEmpty ())
			NextSrcWC_.wait (&NextSrcMutex_, 500);
		qDebug () << "wait finished";

		std::shared_ptr<void> mutexGuard (nullptr,
				[this] (void*) { NextSrcMutex_.unlock (); });

		if (NextSource_.IsEmpty ())
		{
			qDebug () << Q_FUNC_INFO
					<< "no next source set, will stop playing";
			return;
		}

		SetCurrentSource (NextSource_);
		NextSource_.Clear ();
	}

	void SourceObject::HandleErrorMsg (GstMessage *msg)
	{
		GError *error = nullptr;
		gchar *debug = nullptr;
		gst_message_parse_error (msg, &error, &debug);

		const auto& msgStr = QString::fromUtf8 (error->message);
		const auto& debugStr = QString::fromUtf8 (debug);

		g_error_free (error);
		g_free (debug);

		qWarning () << Q_FUNC_INFO
				<< msgStr
				<< debugStr;
	}

	void SourceObject::HandleBufferingMsg (GstMessage *msg)
	{
		gint percentage = 0;
		gst_message_parse_buffering (msg, &percentage);

		emit bufferStatus (percentage);
	}

	namespace
	{
		SourceObject::State GstToState (GstState state)
		{
			switch (state)
			{
			case GST_STATE_PAUSED:
				return SourceObject::State::Paused;
			case GST_STATE_READY:
				return SourceObject::State::Stopped;
			case GST_STATE_PLAYING:
				return SourceObject::State::Playing;
			default:
				return SourceObject::State::Error;
			}
		}
	}

	void SourceObject::HandleStateChangeMsg (GstMessage *msg)
	{
		if (msg->src != GST_OBJECT (Path_->GetPipeline ()))
			return;

		GstState oldState, newState, pending;
		gst_message_parse_state_changed (msg, &oldState, &newState, &pending);

		if (oldState == newState)
			return;

		if (IsSeeking_)
		{
			if (oldState == GST_STATE_PLAYING && newState == GST_STATE_PAUSED)
				return;

			if (oldState == GST_STATE_PAUSED && newState == GST_STATE_PLAYING)
			{
				IsSeeking_ = false;
				return;
			}
		}

		auto newNativeState = GstToState (newState);
		OldState_ = newNativeState;
		emit stateChanged (newNativeState, OldState_);
	}

	void SourceObject::HandleElementMsg (GstMessage *msg)
	{
		const auto msgStruct = gst_message_get_structure (msg);

#if GST_VERSION_MAJOR < 1
		if (gst_structure_has_name (msgStruct, "playbin2-stream-changed"))
#else
		if (gst_structure_has_name (msgStruct, "playbin-stream-changed"))
#endif
		{
			gchar *uri = nullptr;
			g_object_get (Dec_, "uri", &uri, nullptr);
			qDebug () << Q_FUNC_INFO << uri;
			g_free (uri);

			emit currentSourceChanged (CurrentSource_);
		}
	}

	void SourceObject::HandleEosMsg (GstMessage*)
	{
		gst_element_set_state (Path_->GetPipeline (), GST_STATE_READY);
	}

	void SourceObject::SetupSource ()
	{
		GstElement *src;
		g_object_get (Dec_, "source", &src, nullptr);

		if (!CurrentSource_.ToUrl ().scheme ().startsWith ("http"))
			return;

		if (PrevSoupRank_)
		{
			SetSoupRank (PrevSoupRank_);
			PrevSoupRank_ = 0;
		}

		if (!g_object_class_find_property (G_OBJECT_GET_CLASS (src), "user-agent"))
		{
			qDebug () << Q_FUNC_INFO
					<< "user-agent property not found for"
					<< CurrentSource_.ToUrl ()
					<< G_OBJECT_TYPE_NAME (src);
			return;
		}

		const auto& str = QString ("LeechCraft LMP/%1 (%2)")
				.arg (Core::Instance ().GetProxy ()->GetVersion ())
				.arg (gst_version_string ());
		qDebug () << Q_FUNC_INFO
				<< "setting user-agent to"
				<< str;
		g_object_set (src, "user-agent", str.toUtf8 ().constData (), nullptr);
	}

	void SourceObject::AddToPath (Path *path)
	{
		path->SetPipeline (Dec_);
		Path_ = path;
	}

	void SourceObject::PostAdd (Path *path)
	{
		auto bin = path->GetAudioBin ();
		g_object_set (GST_OBJECT (Dec_), "audio-sink", bin, nullptr);
	}

	void SourceObject::updateTotalTime ()
	{
		emit totalTimeChanged (GetTotalTime ());
	}

	void SourceObject::handleTick ()
	{
		emit tick (GetCurrentTime ());
	}
}
}
