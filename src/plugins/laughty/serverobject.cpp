/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverobject.h"
#include <QUrl>
#include <QFile>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QDBusArgument>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xdg/xdg.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC
{
namespace Laughty
{
	namespace
	{
		const QString LaughtyID = "org.LeechCraft.Laughty";
	}

	ServerObject::ServerObject (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	, LastID_ (0)
	{
	}

	QStringList ServerObject::GetCapabilities () const
	{
		return
		{
			"actions",
			"body",
			"body-hyperlinks",
			"body-images",
			"body-markup",
			"persistence"
		};
	}

	namespace
	{
		Priority GetPriority (const QVariantMap& hints)
		{
			switch (hints.value ("urgency", 1).toInt ())
			{
			case 0:
				return Priority::Info;
			case 1:
				return Priority::Info;
			default:
				return Priority::Warning;
			}
		}

		QPair<QString, QString> GetCatTypePair (const QVariantMap&)
		{
			return { AN::CatGeneric, AN::TypeGeneric };
		}
	}

	uint ServerObject::Notify (const QString& app_name, uint,
			const QString& app_icon, QString summary, QString body,
			const QStringList& actions, const QVariantMap& hints, uint expire_timeout)
	{
		const auto replaces = hints.value ("replaces_id", 0).toInt ();
		const auto id = replaces > 0 ? replaces : ++LastID_;

		const auto prio = GetPriority (hints);

		body.remove ("<html>");
		body.remove ("</html>");

		if (summary == app_name && !body.isEmpty ())
		{
			summary = body;
			body.clear ();
		}

		Entity e;
		if (hints.value ("transient", false).toBool () || expire_timeout)
		{
			e = Util::MakeNotification (app_name, summary, prio);

			if (expire_timeout > 0)
				e.Additional_ ["NotificationTimeout"] = expire_timeout;
		}
		else
		{
			const auto& catTypePair = GetCatTypePair (hints);
			e = Util::MakeAN (app_name,
					summary,
					prio,
					LaughtyID,
					catTypePair.first, catTypePair.second,
					LaughtyID + '/' + QString::number (id),
					QStringList (),
					0,
					1,
					summary,
					body);
		}

		HandleActions (e, id, actions, hints);
		HandleSounds (hints);
		HandleImages (e, app_icon, hints);

		Proxy_->GetEntityManager ()->HandleEntity (e);

		return id;
	}

	void ServerObject::CloseNotification (uint id)
	{
		const auto& e = Util::MakeANCancel (LaughtyID, LaughtyID + '/' + QString::number (id));
		Proxy_->GetEntityManager ()->HandleEntity (e);

		emit NotificationClosed (id, 3);
	}

	void ServerObject::HandleActions (Entity& e, int id, const QStringList& actions, const QVariantMap& hints)
	{
		if (actions.isEmpty () || actions.size () % 2)
			return;

		const auto resident = hints.value ("resident", false).toBool ();

		auto nah = new Util::NotificationActionHandler (e);

		for (int i = 0; i < actions.size (); i += 2)
		{
			auto key = actions.at (i);
			nah->AddFunction (actions.at (i + 1),
					[this, key, id, resident] () -> void
					{
						emit ActionInvoked (id, key);
						if (!resident)
							emit NotificationClosed (id, 2);
					});
		}

		if (resident)
			nah->AddFunction (tr ("Dismiss"),
					[this, id] { emit NotificationClosed (id, 2); });
	}

	namespace
	{
		QString GetImgPath (const QVariantMap& hints)
		{
			if (hints.contains ("image-path"))
				return hints.value ("image-path").toString ();

			if (hints.contains ("image_path"))
				return hints.value ("image_path").toString ();

			return {};
		}
	}

	void ServerObject::HandleImages (Entity& e, const QString& appIcon, const QVariantMap& hints)
	{
		HandleImageData (e, hints) ||
			HandleImagePath (e, hints) ||
			HandleImageAppIcon (e, appIcon);
	}

	bool ServerObject::HandleImageData (Entity& e, const QVariantMap& hints)
	{
		const auto& dataVar = hints.value ("image-data", hints.value ("image_data"));
		if (dataVar.isNull ())
			return false;

		const auto& arg = dataVar.value<QDBusArgument> ();

		int width = 0, height = 0, rowstride = 0;
		bool hasAlpha = false;
		int bps = 0, channels = 0;
		QByteArray data;

		arg.beginStructure ();
		arg >> width >> height >> rowstride >> hasAlpha >> bps >> channels >> data;
		arg.endStructure ();

		const QImage img (reinterpret_cast<const uchar*> (data.constBegin ()),
				width, height, QImage::Format_ARGB32);
		if (img.isNull ())
			return false;

		e.Additional_ ["NotificationPixmap"] = QPixmap::fromImage (img.rgbSwapped ());
		return true;
	}

	bool ServerObject::HandleImagePath (Entity& e, const QVariantMap& hints)
	{
		auto path = GetImgPath (hints);
		if (path.isEmpty ())
			return false;

		if (QFile::exists (path))
			e.Additional_ ["NotificationPixmap"] = QPixmap (path);
		else if (path.startsWith ("file:"))
			e.Additional_ ["NotificationPixmap"] = QPixmap (QUrl (path).toLocalFile ());
		else
			return false;

		return true;
	}

	bool ServerObject::HandleImageAppIcon (Entity& e, const QString& appIcon)
	{
		if (appIcon.isEmpty ())
			return false;

		QPixmap result;

		auto icon = Proxy_->GetIconThemeManager ()->GetIcon (appIcon);
		if (!icon.isNull ())
		{
			const auto& sizes = icon.availableSizes ();
			result = icon.pixmap (sizes.value (sizes.size () - 1, QSize (48, 48)));
		}

		if (result.isNull ())
			result = Util::XDG::GetAppPixmap (appIcon);

		if (result.isNull ())
			return false;

		e.Additional_ ["NotificationPixmap"] = result;
		return true;
	}

	void ServerObject::HandleSounds (const QVariantMap& hints)
	{
		if (hints.contains ("sound-name"))
			qWarning () << Q_FUNC_INFO
					<< "sounds aren't supported yet :(";

		if (!hints.contains ("sound-file"))
			return;

		const auto& filename = hints.value ("sound-file").toString ();
		const auto& e = Util::MakeEntity (QUrl::fromLocalFile (filename),
				QString (),
				TaskParameter::Internal);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
