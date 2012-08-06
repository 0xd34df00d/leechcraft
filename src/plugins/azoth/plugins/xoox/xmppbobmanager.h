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

#pragma once

#include <QXmppClientExtension.h>
#include <QCache>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class XMPPBobIq;

	class XMPPBobManager : public QXmppClientExtension
	{
		Q_OBJECT

		QCache<QPair<QString, QString>, QByteArray> BobCache_;
	public:
		XMPPBobManager (unsigned int cacheSizeKb = 2048);

		bool handleStanza (const QDomElement&);
		QStringList discoveryFeatures () const;

		QString RequestBob (const QString&, const QString&);
		QByteArray Take (const QString&, const QString&);
	signals:
		void bobReceived (const XMPPBobIq&);
	};
}
}
}
