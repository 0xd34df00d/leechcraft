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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_CORE_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include "notificationrule.h"

namespace LeechCraft
{
namespace Util
{
	class ResourceLoader;
}

namespace AdvancedNotifications
{
	class NotificationRulesWidget;

	class Core : public QObject
	{
		Q_OBJECT
		
		ICoreProxy_ptr Proxy_;
		
		NotificationRulesWidget *NRW_;
		std::shared_ptr<Util::ResourceLoader> AudioThemeLoader_;
		
		Core ();
	public:
		static Core& Instance ();
		void Release ();
		
		ICoreProxy_ptr GetProxy () const;
		void SetProxy (ICoreProxy_ptr);
		
		NotificationRulesWidget* GetNRW ();
		std::shared_ptr<Util::ResourceLoader> GetAudioThemeLoader () const;

		QList<NotificationRule> GetRules (const Entity&) const;
		
		void SendEntity (const Entity&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}

#endif
