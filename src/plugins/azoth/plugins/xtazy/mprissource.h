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

#ifndef PLUGINS_AZOTH_PLUGINS_XTAZY_MPRISSOURCE_H
#define PLUGINS_AZOTH_PLUGINS_XTAZY_MPRISSOURCE_H
#include "tunesourcebase.h"
#include <QStringList>
#include <QDBusConnection>

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	struct PlayerStatus
	{
		int PlayStatus_;
		int PlayOrder_;
		int PlayRepeat_;
		int StopOnce_;
	};

	class MPRISSource : public TuneSourceBase
	{
		Q_OBJECT
		
		QStringList Players_;
		QDBusConnection SB_;
		TuneInfo_t Tune_;
	public:
		MPRISSource (QObject* = 0);
		virtual ~MPRISSource ();
	private:
		void ConnectToBus (const QString&);
		void DisconnectFromBus (const QString&);
		TuneInfo_t GetTuneMV2 (const QVariantMap&);
	private slots:
		void handlePropertyChange (const QDBusMessage&);
		void handlePlayerStatusChange (PlayerStatus);
		void handleTrackChange (const QVariantMap&);
		void checkMPRISService (QString, QString, QString);
	};
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::Xtazy::PlayerStatus);

#endif
