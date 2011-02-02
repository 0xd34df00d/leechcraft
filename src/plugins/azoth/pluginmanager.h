/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINMANAGER_H
#define PLUGINS_AZOTH_PLUGINMANAGER_H
#include <QString>
#include <QDateTime>
#include <plugininterface/basehookinterconnector.h>
#include <interfaces/iinfo.h>

class QDateTime;
class QObject;

namespace LeechCraft
{
namespace Azoth
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		PluginManager (QObject* = 0);
	signals:
		void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatDateTime (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QDateTime dateTime,
				QObject *message);
		void hookFormatNickname (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString nick,
				QObject *message);
		void hookFormatBodyBegin (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString body,
				QObject *message);
		void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString body,
				QObject *message);
		void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				int type,
				QString variant,
				QString text);
		void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
	};
}
}

#endif
