/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "chroma.h"
#include <stdexcept>
#include <memory>
#include <QString>
#include <QByteArray>
#include <QtDebug>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audioconvert.h>
#include <libavutil/samplefmt.h>
}

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	Chroma::Chroma ()
	: Ctx_ (chromaprint_new (CHROMAPRINT_ALGORITHM_DEFAULT))
	{
		av_register_all ();
	}

	Chroma::~Chroma ()
	{
		chromaprint_free (Ctx_);
	}

	QByteArray Chroma::operator() (const QString& filename)
	{
		std::shared_ptr<AVFormatContext> formatCtx;
		{
			AVFormatContext *formatCtxRaw = nullptr;
			if (avformat_open_input (&formatCtxRaw, filename.toLatin1  ().constData (), nullptr, nullptr))
				throw std::runtime_error ("error opening file");

			formatCtx.reset (formatCtxRaw,
					[] (AVFormatContext *ctx) { avformat_close_input (&ctx); });
		}

		if (avformat_find_stream_info (formatCtx.get (), nullptr) < 0)
			throw std::runtime_error ("could not find stream");

		bool codecOpened = false;
		std::shared_ptr<AVCodecContext> codecCtx;
		AVStream *stream = nullptr;
		for (unsigned int i = 0; i < formatCtx->nb_streams; ++i)
		{
			codecCtx.reset (formatCtx->streams [i]->codec,
					[&codecOpened] (AVCodecContext *ctx)
					{
						if (codecOpened)
							avcodec_close (ctx);
						avcodec_close (ctx);
					});
			if (codecCtx && codecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				stream = formatCtx->streams [i];
				break;
			}
		}

		if (!stream)
			throw std::runtime_error ("could not find stream");

		auto codec = avcodec_find_decoder (codecCtx->codec_id);
		if (!codec)
			throw std::runtime_error ("unknown codec");

		if (avcodec_open (codecCtx.get (), codec) < 0)
			throw std::runtime_error ("couldn't open the codec");
		codecOpened = true;

		if (codecCtx->channels <= 0)
			throw std::runtime_error ("no channels found");

		/* TODO swresample
		 *
		 * Upstream ffmpeg/libav have migrated to it.
		 * https://github.com/xbmc/xbmc/pull/882
		 */
		if (codecCtx->sample_fmt != AV_SAMPLE_FMT_S16)
			throw std::runtime_error ("invalid sampling format");

		AVPacket packet, tmpPacket;
		av_init_packet (&packet);
		av_init_packet (&tmpPacket);

		const int maxLength = 120;
		auto remaining = maxLength * codecCtx->channels * codecCtx->sample_rate;
		chromaprint_start (Ctx_, codecCtx->sample_rate, codecCtx->channels);

		const int bufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * sizeof (int16_t);
		int16_t *buffer = static_cast<int16_t*> (av_malloc (bufferSize + 16));

		while (true)
		{
			if (av_read_frame (formatCtx.get (), &packet) < 0)
				break;

			tmpPacket.data = packet.data;
			tmpPacket.size = packet.size;

			bool finished = false;
			while (tmpPacket.size > 0)
			{
				auto bufferUsed = bufferSize;
				auto consumed = avcodec_decode_audio3 (codecCtx.get (), buffer, &bufferUsed, &tmpPacket);
				if (consumed < 0)
					break;

				tmpPacket.data += consumed;
				tmpPacket.size -= consumed;

				if (bufferUsed <= 0 || bufferUsed >= bufferSize)
					continue;

				const auto length = std::min (remaining, AVCODEC_MAX_AUDIO_FRAME_SIZE);
				if (!chromaprint_feed (Ctx_, buffer, length))
					throw std::runtime_error ("fingerprint calculation failed");

				if (maxLength)
				{
					remaining -= length;
					finished = remaining <= 0;
				}
			}

			if (packet.data)
				av_free_packet (&packet);

			if (finished)
				break;
		}

		if (!chromaprint_finish (Ctx_))
			throw std::runtime_error ("fingerprint calculation failed");

		char *fingerprint = 0;
		if (!chromaprint_get_fingerprint (Ctx_, &fingerprint))
			throw std::runtime_error ("unable to get fingerprint");

		QByteArray result (fingerprint);
		chromaprint_dealloc (fingerprint);

		return result;
	}
}
}
}
