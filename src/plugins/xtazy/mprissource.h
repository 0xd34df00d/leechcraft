/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "tunesourcebase.h"
#include <QStringList>
#include <QDBusConnection>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace Xtazy
{
	struct PlayerStatus;

	class MPRISSource : public TuneSourceBase
	{
		Q_OBJECT

		QDBusConnection SB_;
		QStringList Players_;

		Media::AudioInfo Tune_;
	public:
		MPRISSource (QObject* = nullptr);
	private:
		void ConnectToBus (const QString&);
		void DisconnectFromBus (const QString&);
		Media::AudioInfo GetTuneMV2 (const QVariantMap&);
	private slots:
		void handlePropertyChange (const QDBusMessage&);
		void handlePlayerStatusChange (const PlayerStatus&);
		void handleTrackChange (const QVariantMap&);
		void checkMPRISService (QString, QString, QString);
	};
}
}
