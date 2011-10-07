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

#include "core.h"
#include <util/resourceloader.h>
#include "notificationruleswidget.h"
#include "typedmatchers.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	QDataStream& operator<< (QDataStream& out, const NotificationRule& r)
	{
		r.Save (out);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, NotificationRule& r)
	{
		r.Load (in);
		return in;
	}

	Core::Core ()
	: NRW_ (0)
	, AudioThemeLoader_ (new Util::ResourceLoader ("sounds/"))
	{
		AudioThemeLoader_->AddLocalPrefix ();
		AudioThemeLoader_->AddGlobalPrefix ();

		qRegisterMetaType<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaType<QList<NotificationRule> > ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");
		qRegisterMetaTypeStreamOperators<QList<NotificationRule> > ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		AudioThemeLoader_.reset ();
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	NotificationRulesWidget* Core::GetNRW ()
	{
		if (!NRW_)
			NRW_ = new NotificationRulesWidget;
		return NRW_;
	}

	boost::shared_ptr<Util::ResourceLoader> Core::GetAudioThemeLoader () const
	{
		return AudioThemeLoader_;
	}

	QList<NotificationRule> Core::GetRules (const Entity& e) const
	{
		const QString& type = e.Additional_ ["org.LC.AdvNotifications.EventType"].toString ();

		QList<NotificationRule> result;

		Q_FOREACH (const NotificationRule& rule, NRW_->GetRules ())
		{
			if (!rule.IsEnabled ())
				continue;

			if (!rule.GetTypes ().contains (type))
				continue;

			bool fieldsMatch = true;
			Q_FOREACH (const FieldMatch& match, rule.GetFieldMatches ())
			{
				const QString& fieldName = match.GetFieldName ();
				const TypedMatcherBase_ptr matcher = match.GetMatcher ();
				if (!matcher->Match (e.Additional_ [fieldName]))
				{
					fieldsMatch = false;
					break;
				}
			}

			if (!fieldsMatch)
				continue;

			if (rule.IsSingleShot ())
				NRW_->SetRuleEnabled (rule, false);

			result << rule;
			break;
		}

		return result;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}
}
}
