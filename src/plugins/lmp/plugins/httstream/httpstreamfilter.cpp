/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "httpstreamfilter.h"
#include <QUuid>
#include <QtDebug>
#include <QTimer>
#include <gst/gst.h>
#include "interfaces/lmp/ifilterconfigurator.h"
#include "util/lmp/gstutil.h"
#include "httpserver.h"
#include "filterconfigurator.h"

namespace LC
{
namespace LMP
{
namespace HttStream
{
	namespace
	{
		void CbRemoved (void*, gint fd, int reason, gpointer udata)
		{
			static_cast<HttpStreamFilter*> (udata)->HandleRemoved (fd, reason);
		}
	}

	HttpStreamFilter::HttpStreamFilter (const QByteArray& filterId,
			const QByteArray& instanceId, IPath *path)
	: FilterId_ { filterId }
	, InstanceId_ { instanceId.isEmpty () ? QUuid::createUuid ().toByteArray () : instanceId }
	, Path_ { path }
	, Configurator_ { new FilterConfigurator { instanceId, this } }
	, Elem_ { gst_bin_new (nullptr) }
	, Tee_ { gst_element_factory_make ("tee", nullptr) }
	, TeeTemplate_ { gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (Tee_), GstUtil::GetTeePadTemplateName ()) }
	, AudioQueue_ { gst_element_factory_make ("queue", nullptr) }
	, StreamQueue_ { gst_element_factory_make ("queue", nullptr) }
	, AConv_ { gst_element_factory_make ("audioconvert", nullptr) }
	, Encoder_ { gst_element_factory_make ("vorbisenc", nullptr) }
	, Muxer_ { gst_element_factory_make ("oggmux", nullptr) }
	, MSS_ { gst_element_factory_make ("multifdsink", nullptr) }
	, Server_ { new HttpServer { this } }
	{
		if (!MSS_)
			qWarning () << Q_FUNC_INFO
					<< "cannot create multisocketsink";

		for (const auto elem : GetStreamBranchElements ())
			gst_object_ref (elem);

		gst_bin_add_many (GST_BIN (Elem_), Tee_, AudioQueue_, nullptr);

		TeeAudioPad_ = gst_element_request_pad (Tee_, TeeTemplate_, nullptr, nullptr);
		auto audioPad = gst_element_get_static_pad (AudioQueue_, "sink");
		gst_pad_link (TeeAudioPad_, audioPad);
		gst_object_unref (audioPad);

		g_object_set (G_OBJECT (MSS_),
				"recover-policy", 3,
				"sync-method", 1,
				"async", FALSE,
				"sync", FALSE,
				nullptr);

		GstUtil::AddGhostPad (Tee_, Elem_, "sink");
		GstUtil::AddGhostPad (AudioQueue_, Elem_, "src");

		connect (Server_,
				SIGNAL (gotClient (int)),
				this,
				SLOT (handleClient (int)));
		connect (Server_,
				SIGNAL (clientDisconnected (int)),
				this,
				SLOT (handleClientDisconnected (int)));

		g_signal_connect (MSS_, "client-removed", G_CALLBACK (CbRemoved), this);
	}

	HttpStreamFilter::~HttpStreamFilter ()
	{
		for (const auto elem : GetStreamBranchElements ())
		{
			gst_element_set_state (elem, GST_STATE_NULL);
			gst_object_unref (elem);
		}
	}

	QByteArray HttpStreamFilter::GetEffectId () const
	{
		return FilterId_;
	}

	QByteArray HttpStreamFilter::GetInstanceId () const
	{
		return InstanceId_;
	}

	IFilterConfigurator* HttpStreamFilter::GetConfigurator () const
	{
		return Configurator_;
	}

	void HttpStreamFilter::SetQuality (double val)
	{
		g_object_set (G_OBJECT (Encoder_), "quality", val, nullptr);
	}

	void HttpStreamFilter::SetAddress (const QString& host, int port)
	{
		Server_->SetAddress (host, port);
	}

	namespace
	{
		// http://cgit.collabora.com/git/user/kakaroto/gst-plugins-base.git/plain/gst/tcp/gstmultihandlesink.c
		const int GST_CLIENT_STATUS_REMOVED = 2;
	}

	void HttpStreamFilter::HandleRemoved (int fd, int reason)
	{
		if (reason != GST_CLIENT_STATUS_REMOVED)
			return;

		qDebug () << Q_FUNC_INFO
				<< "detected client removal because of"
				<< reason
				<< "; scheduling readd...";

		QTimer::singleShot (0, this, [this, fd] { g_signal_emit_by_name (MSS_, "add", fd); });
	}

	GstElement* HttpStreamFilter::GetElement () const
	{
		return Elem_;
	}

	void HttpStreamFilter::PostAdd (IPath *path)
	{
		path->AddSyncHandler ([this] (GstBus*, GstMessage *msg) { return HandleError (msg); }, this);
	}

	void HttpStreamFilter::CreatePad ()
	{
		qDebug () << Q_FUNC_INFO;

		gst_bin_add_many (GST_BIN (Elem_), StreamQueue_, Encoder_, AConv_, Muxer_, MSS_, nullptr);
		gst_element_link_many (StreamQueue_, AConv_, Encoder_, Muxer_, MSS_, nullptr);
		for (auto elem : GetStreamBranchElements ())
			gst_element_sync_state_with_parent (elem);

		TeeStreamPad_ = gst_element_request_pad (Tee_, TeeTemplate_, nullptr, nullptr);
		auto streamPad = gst_element_get_static_pad (StreamQueue_, "sink");
		gst_pad_link (TeeStreamPad_, streamPad);
		gst_object_unref (streamPad);
	}

	void HttpStreamFilter::DestroyPad ()
	{
		auto streamPad = gst_element_get_static_pad (StreamQueue_, "sink");
		gst_pad_unlink (TeeStreamPad_, streamPad);
		gst_object_unref (streamPad);

		gst_element_release_request_pad (Tee_, TeeStreamPad_);
		gst_object_unref (TeeStreamPad_);

		gst_element_unlink_many (StreamQueue_, AConv_, Encoder_, Muxer_, MSS_, nullptr);
		gst_bin_remove_many (GST_BIN (Elem_), StreamQueue_, Encoder_, AConv_, Muxer_, MSS_, nullptr);

		TeeStreamPad_ = nullptr;
	}

	std::vector<GstElement*> HttpStreamFilter::GetStreamBranchElements () const
	{
		return { StreamQueue_, AConv_, Encoder_, Muxer_, MSS_ };
	}

	bool HttpStreamFilter::HandleFirstClientConnected ()
	{
		const auto source = Path_->GetSourceObject ();
		StateOnFirst_ = source->GetState ();

		if (StateOnFirst_ == SourceState::Playing)
		{
			CreatePad ();
			return true;
		}
		else
		{
			connect (source->GetQObject (),
					SIGNAL (stateChanged (SourceState, SourceState)),
					this,
					SLOT (checkCreatePad (SourceState)),
					Qt::UniqueConnection);

			source->SetState (SourceState::Playing);

			return false;
		}
	}

	void HttpStreamFilter::HandleLastClientDisconnected ()
	{
		DestroyPad ();

		if (StateOnFirst_ != SourceState::Playing)
			Path_->GetSourceObject ()->SetState (SourceState::Paused);
	}

	int HttpStreamFilter::HandleError (GstMessage *msg)
	{
		if (GST_MESSAGE_TYPE (msg) != GST_MESSAGE_ERROR)
			return GST_BUS_PASS;

		if (QList<GstElement*> { StreamQueue_, Encoder_, MSS_ }.contains (GST_ELEMENT (msg->src)))
		{
			qDebug () << Q_FUNC_INFO
					<< "detected stream error";

			gst_message_unref (msg);

			return GST_BUS_DROP;
		}

		return GST_BUS_PASS;
	}

	void HttpStreamFilter::checkCreatePad (SourceState state)
	{
		if (state != SourceState::Playing)
			return;

		disconnect (Path_->GetSourceObject ()->GetQObject (),
				SIGNAL (stateChanged (SourceState, SourceState)),
				this,
				SLOT (checkCreatePad (SourceState)));
		CreatePad ();

		for (const auto sock : PendingSockets_)
			g_signal_emit_by_name (MSS_, "add", sock);

		PendingSockets_.clear ();
	}

	void HttpStreamFilter::handleClient (int socket)
	{
		if (ClientsCount_ || HandleFirstClientConnected ())
			g_signal_emit_by_name (MSS_, "add", socket);
		else
			PendingSockets_ << socket;

		++ClientsCount_;
	}

	void HttpStreamFilter::handleClientDisconnected (int socket)
	{
		g_signal_emit_by_name (MSS_, "remove", socket);

		if (!--ClientsCount_)
			HandleLastClientDisconnected ();
	}
}
}
}
