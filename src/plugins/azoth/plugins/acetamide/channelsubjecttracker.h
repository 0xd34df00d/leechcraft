/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <util/azoth/emitters/mucentry.h>

class QTimer;

namespace LC::Azoth::Acetamide
{
	class ChannelSubjectTracker : public QObject
	{
		Q_OBJECT

		Emitters::MUCEntry& Emitter_;

		QString Text_;
		QString Url_;

		bool PendingHistoricalEmit_ = false;
		QTimer * const FallbackTimer_;
	public:
		explicit ChannelSubjectTracker (Emitters::MUCEntry& emitter, QObject *parent = nullptr);

		QString GetText () const;

		void Set (const QString& text, const std::optional<QString>& actorNick, MucEvents::Liveness);
		void NotifyAuthor (const QString& who, quint64 time);
		void SetUrl (const QString& url);
	};
}
