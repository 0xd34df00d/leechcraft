/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chroma.h"
#include <stdexcept>
#include <memory>
#include <QString>
#include <QByteArray>
#include <QtDebug>
#include <util/sll/util.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#include <chromaprint.h>

namespace LC
{
namespace MusicZombie
{
	QMutex Chroma::CodecMutex_;

	Chroma::Chroma ()
	: Ctx_ (chromaprint_new (CHROMAPRINT_ALGORITHM_DEFAULT), &chromaprint_free)
	{
	}

	namespace
	{
		template<typename T>
		auto AdaptDeleter (void (*f) (T**))
		{
			return [f] (T *item) { f (&item); };
		}
	}

	Chroma::Result Chroma::operator() (const QString& filename)
	{
		std::shared_ptr<AVFormatContext> formatCtx;
		{
			AVFormatContext *formatCtxRaw = nullptr;
			if (avformat_open_input (&formatCtxRaw, filename.toLatin1 ().constData (), nullptr, nullptr))
				throw std::runtime_error ("error opening file");

			formatCtx.reset (formatCtxRaw, AdaptDeleter (&avformat_close_input));
		}

		{
			QMutexLocker locker (&CodecMutex_);
			if (avformat_find_stream_info (formatCtx.get (), nullptr) < 0)
				throw std::runtime_error ("could not find stream");
		}

#if LIBAVFORMAT_VERSION_MAJOR >= 59
		const AVCodec *codec = nullptr;
#else
		AVCodec *codec = nullptr;
#endif
		const auto streamIndex = av_find_best_stream (formatCtx.get (), AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
		if (streamIndex < 0)
			throw std::runtime_error ("could not find audio stream");

		auto stream = formatCtx->streams [streamIndex];

		std::shared_ptr<AVCodecContext> codecCtx (avcodec_alloc_context3 (codec), AdaptDeleter (&avcodec_free_context));
		{
			QMutexLocker locker (&CodecMutex_);
			if (avcodec_open2 (codecCtx.get (), codec, nullptr) < 0)
				throw std::runtime_error ("couldn't open the codec");
		}

		const auto numChannels = stream->codecpar->channels;
		if (numChannels <= 0)
			throw std::runtime_error ("no channels found");

		std::shared_ptr<SwrContext> swr;
		const auto sampleFormat = static_cast<AVSampleFormat> (stream->codecpar->format);
		if (sampleFormat != AV_SAMPLE_FMT_S16)
		{
			swr.reset (swr_alloc (), AdaptDeleter (swr_free));
			av_opt_set_int (swr.get (), "in_channel_layout", stream->codecpar->channel_layout, 0);
			av_opt_set_int (swr.get (), "out_channel_layout", stream->codecpar->channel_layout,  0);
			av_opt_set_int (swr.get (), "in_sample_rate", stream->codecpar->sample_rate, 0);
			av_opt_set_int (swr.get (), "out_sample_rate", stream->codecpar->sample_rate, 0);
			av_opt_set_sample_fmt (swr.get (), "in_sample_fmt", sampleFormat, 0);
			av_opt_set_sample_fmt (swr.get (), "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
			swr_init (swr.get ());
		}

		auto packetDeleter = [] (AVPacket *packet) { av_packet_free (&packet); };
		std::unique_ptr<AVPacket, decltype (packetDeleter)> packet { av_packet_alloc () };

		const int maxLength = 120;
		auto remaining = maxLength * numChannels * stream->codecpar->sample_rate;
		chromaprint_start (Ctx_.get (), stream->codecpar->sample_rate, numChannels);

		std::shared_ptr<AVFrame> frame (av_frame_alloc (), AdaptDeleter (&av_frame_free));
		auto maxDstNbSamples = 0;

		uint8_t *dstData [1] = { nullptr };
		const auto dstDataGuard = Util::MakeScopeGuard ([&dstData] { if (dstData [0]) av_freep (&dstData [0]); });
		while (true)
		{
			if (av_read_frame (formatCtx.get (), packet.get ()) < 0)
				break;

			const auto guard = Util::MakeScopeGuard ([&packet] { if (packet->data) av_packet_unref (packet.get ()); });

			if (packet->stream_index != streamIndex)
				continue;

			if (avcodec_send_packet (codecCtx.get (), packet.get ()) ||
					avcodec_receive_frame (codecCtx.get (), frame.get ()))
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to read another frame";
				continue;
			}

			const auto unrefGuard = Util::MakeScopeGuard ([&frame] { av_frame_unref (frame.get ()); });

			uint8_t **data = nullptr;
			if (swr)
			{
				if (frame->nb_samples > maxDstNbSamples)
				{
					if (dstData [0])
						av_freep (&dstData [0]);
					int linesize = 0;
					if (av_samples_alloc (dstData, &linesize, numChannels, frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0)
						throw std::runtime_error ("cannot allocate memory for resampling");
				}

				if (swr_convert (swr.get (), dstData, frame->nb_samples, const_cast<const uint8_t**> (frame->data), frame->nb_samples) < 0)
					throw std::runtime_error ("cannot resample audio");

				data = dstData;
			}
			else
				data = frame->data;

			auto length = std::min (remaining, frame->nb_samples * numChannels);
			if (!chromaprint_feed (Ctx_.get (),
					reinterpret_cast<const int16_t*> (data [0]),
					length))
				throw std::runtime_error ("cannot feed data");

			remaining -= length;
			if (remaining <= 0)
				break;
		}

		if (!chromaprint_finish (Ctx_.get ()))
			throw std::runtime_error ("fingerprint calculation failed");

		char *fingerprint = 0;
		if (!chromaprint_get_fingerprint (Ctx_.get (), &fingerprint))
			throw std::runtime_error ("unable to get fingerprint");

		QByteArray result (fingerprint);
		chromaprint_dealloc (fingerprint);

		const double divideFactor = 1. / av_q2d (stream->time_base);
		const double duration = stream->duration / divideFactor;

		return { result, static_cast<int> (duration) };
	}
}
}
