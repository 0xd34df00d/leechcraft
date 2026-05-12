/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsubjecttracker.h"
#include <utility>
#include <QTimer>

namespace LC::Azoth::Acetamide
{
	ChannelSubjectTracker::ChannelSubjectTracker (Emitters::MUCEntry& emitter, QObject *parent)
	: QObject { parent }
	, Emitter_ { emitter }
	, FallbackTimer_ { new QTimer { this } }
	{
		FallbackTimer_->setSingleShot (true);
		FallbackTimer_->setInterval (500);
		connect (FallbackTimer_,
				&QTimer::timeout,
				this,
				[this]
				{
					if (std::exchange (PendingHistoricalEmit_, false))
						emit Emitter_.mucSubjectChanged ({
									.Subject_ = GetText (),
									.ActorNick_ = std::nullopt,
									.Liveness_ = MucEvents::Liveness::Historical
								});
				});
	}

	QString ChannelSubjectTracker::GetText () const
	{
		return Url_.isEmpty () ? Text_ : Text_ + "\nURL: " + Url_;
	}

	void ChannelSubjectTracker::Set (const QString& text,
			const std::optional<QString>& actorNick, MucEvents::Liveness liveness)
	{
		if (Text_ == text)
			return;

		Text_ = text;

		if (liveness == MucEvents::Liveness::Live)
		{
			PendingHistoricalEmit_ = false;
			FallbackTimer_->stop ();
			emit Emitter_.mucSubjectChanged ({
						.Subject_ = GetText (),
						.ActorNick_ = actorNick,
						.Liveness_ = liveness
					});
		}
		else
		{
			PendingHistoricalEmit_ = true;
			FallbackTimer_->start ();
		}
	}

	void ChannelSubjectTracker::NotifyAuthor (const QString& who, quint64)
	{
		if (!std::exchange (PendingHistoricalEmit_, false))
			return;
		FallbackTimer_->stop ();
		emit Emitter_.mucSubjectChanged ({
					.Subject_ = GetText (),
					.ActorNick_ = who,
					.Liveness_ = MucEvents::Liveness::Historical
				});
	}

	void ChannelSubjectTracker::SetUrl (const QString& url)
	{
		if (Url_ == url)
			return;
		Url_ = url;

		if (PendingHistoricalEmit_)
			return;

		emit Emitter_.mucSubjectChanged ({
					.Subject_ = GetText (),
					.ActorNick_ = std::nullopt,
					.Liveness_ = MucEvents::Liveness::Historical
				});
	}
}
