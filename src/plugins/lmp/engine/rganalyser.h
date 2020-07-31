/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>
#include <QStringList>

typedef struct _GstMessage GstMessage;
typedef struct _GstElement GstElement;
typedef std::shared_ptr<GstMessage> GstMessage_ptr;

namespace LC
{
namespace LMP
{
	struct TrackRgResult
	{
		QString TrackPath_;
		double TrackGain_ = 0;
		double TrackPeak_ = 0;
	};

	struct AlbumRgResult
	{
		double AlbumGain_ = 0;
		double AlbumPeak_ = 0;

		QList<TrackRgResult> Tracks_;
	};

	class LightPopThread;

	class RgAnalyser : public QObject
	{
		Q_OBJECT

		QStringList Paths_;
		QString CurrentPath_;

		AlbumRgResult Result_;

		GstElement * const Pipeline_;

		GstElement * const SinkBin_;
		GstElement * const AConvert_;
		GstElement * const AResample_;
		GstElement * const RGAnalysis_;
		GstElement * const Fakesink_;

		LightPopThread * const PopThread_;

		bool IsDraining_ = false;
	public:
		RgAnalyser (const QStringList&, QObject* = nullptr);
		~RgAnalyser ();

		const AlbumRgResult& GetResult () const;
	private:
		void CheckFinish ();

		void HandleTagMsg (GstMessage*);
		void HandleErrorMsg (GstMessage*);
		void HandleEosMsg (GstMessage*);
	private slots:
		void handleMessage (GstMessage_ptr);
	signals:
		void finished ();
	};
}
}
