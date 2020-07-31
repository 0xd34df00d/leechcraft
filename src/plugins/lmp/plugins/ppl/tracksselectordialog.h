/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/media/iaudioscrobbler.h>
#include "ui_tracksselectordialog.h"

namespace LC
{
namespace LMP
{
namespace PPL
{
	class TracksSelectorDialog : public QDialog
	{
		Q_OBJECT

		Ui::TracksSelectorDialog Ui_;

		class TracksModel;
		TracksModel * const Model_;
	public:
		TracksSelectorDialog (const Media::IAudioScrobbler::BackdatedTracks_t&,
				const QList<Media::IAudioScrobbler*>&, QWidget* = nullptr);

		struct SelectedTrack
		{
			Media::IAudioScrobbler::BackdatedTrack_t Track_;
			QVector<bool> Scrobbles_;
		};

		QList<SelectedTrack> GetSelectedTracks () const;
	private:
		void FixSize ();
	};
}
}
}
