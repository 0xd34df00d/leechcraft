/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "resourcesmanager.h"
#include <QIcon>
#include <QtDebug>
#include <util/sys/resourceloader.h>
#include "xmlsettingsmanager.h"
#include "interfaces/azoth/iclentry.h"

namespace LC
{
namespace Azoth
{
	ResourcesManager::ResourcesManager ()
	{
		ResourceLoaders_ [RLTStatusIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/contactlist/", this));
		ResourceLoaders_ [RLTClientIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/clients/", this));
		ResourceLoaders_ [RLTAffIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/affiliations/", this));
		ResourceLoaders_ [RLTSystemIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/system/", this));
		ResourceLoaders_ [RLTActivityIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/activities/", this));
		ResourceLoaders_ [RLTMoodIconLoader].reset (new Util::ResourceLoader ("azoth/iconsets/moods/", this));

		for (const auto& rl : ResourceLoaders_)
		{
			rl->AddLocalPrefix ();
			rl->AddGlobalPrefix ();

			rl->SetCacheParams (1000, 0);
		}
	}

	ResourcesManager& ResourcesManager::Instance ()
	{
		static ResourcesManager rm;
		return rm;
	}

	Util::ResourceLoader* ResourcesManager::GetResourceLoader (LoaderType type) const
	{
		return ResourceLoaders_ [type].get ();
	}

	void ResourcesManager::HandleEntry (ICLEntry *clEntry)
	{
		connect (clEntry->GetQObject (),
				SIGNAL (availableVariantsChanged (const QStringList&)),
				this,
				SLOT (invalidateClientsIconCache ()));
		connect (clEntry->GetQObject (),
				SIGNAL (statusChanged (EntryStatus, QString)),
				this,
				SLOT (invalidateClientsIconCache ()));
		connect (clEntry->GetQObject (),
				SIGNAL (entryGenerallyChanged ()),
				this,
				SLOT (invalidateClientsIconCache ()));
	}

	void ResourcesManager::HandleRemoved (ICLEntry *entry)
	{
		EntryClientIconCache_.remove (entry);
		invalidateClientsIconCache (entry->GetQObject ());
	}

	namespace
	{
		QString GetStateIconFilename (State state)
		{
			QString iconName;
			switch (state)
			{
			case SOnline:
				iconName = "online";
				break;
			case SChat:
				iconName = "chatty";
				break;
			case SAway:
				iconName = "away";
				break;
			case SDND:
				iconName = "dnd";
				break;
			case SXA:
				iconName = "xa";
				break;
			case SOffline:
				iconName = "offline";
				break;
			case SConnecting:
				iconName = "connect";
				break;
			default:
				iconName = "perr";
				break;
			}

			QString filename = XmlSettingsManager::Instance ()
					.property ("StatusIcons").toString ();
			filename += '/';
			filename += iconName;

			return filename;
		}
	}

	Util::QIODevice_ptr ResourcesManager::GetIconPathForState (State state) const
	{
		const QString& filename = GetStateIconFilename (state);
		return ResourceLoaders_ [RLTStatusIconLoader]->GetIconDevice (filename, true);
	}

	QIcon ResourcesManager::GetIconForState (State state) const
	{
		const QString& filename = GetStateIconFilename (state);
		return ResourceLoaders_ [RLTStatusIconLoader]->LoadPixmap (filename);
	}

	QIcon ResourcesManager::GetAffIcon (const QByteArray& affName) const
	{
		QString filename = XmlSettingsManager::Instance ()
				.property ("AffIcons").toString ();
		filename += '/';
		filename += affName;

		return QIcon (ResourceLoaders_ [RLTAffIconLoader]->LoadPixmap (filename));
	}

	QMap<QString, QIcon> ResourcesManager::GetClientIconForEntry (ICLEntry *entry)
	{
		if (EntryClientIconCache_.contains (entry))
			return EntryClientIconCache_ [entry];

		QMap<QString, QIcon> result;

		const auto& pack = XmlSettingsManager::Instance ()
					.property ("ClientIcons").toString () + '/';
		for (const auto& variant : entry->Variants ())
		{
			const auto& info = entry->GetClientInfo (variant);
			if (info.contains ("client_image"))
			{
				const auto& image = info ["client_image"].value<QImage> ();
				result [variant] = QIcon (QPixmap::fromImage (image));
			}
			else
			{
				const auto& type = info ["client_type"].toString ();

				const auto& filename = pack + type;

				auto pixmap = ResourceLoaders_ [RLTClientIconLoader]->LoadPixmap (filename);
				if (pixmap.isNull ())
					pixmap = ResourceLoaders_ [RLTClientIconLoader]->LoadPixmap (pack + "unknown");

				result [variant] = QIcon (pixmap);
			}
		}

		EntryClientIconCache_ [entry] = result;
		return result;
	}

	QImage ResourcesManager::GetDefaultAvatar (int size) const
	{
		const auto& name = XmlSettingsManager::Instance ()
				.property ("SystemIcons").toString () + "/default_avatar";
		const auto& image = ResourceLoaders_ [RLTSystemIconLoader]->LoadPixmap (name).toImage ();

		if (image.isNull ())
			return {};

		if (size == -1)
			return image;

		return image.scaled (size, size,
				Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	void ResourcesManager::invalidateClientsIconCache (QObject *passedObj)
	{
		QObject *obj = passedObj ? passedObj : sender ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "could not be casted to ICLEntry";
			return;
		}

		invalidateClientsIconCache (entry);
	}

	void ResourcesManager::invalidateClientsIconCache (ICLEntry *entry)
	{
		EntryClientIconCache_.remove (entry);
	}

	void ResourcesManager::flushIconCaches ()
	{
		for (const auto& rl : ResourceLoaders_)
			rl->FlushCache ();
	}
}
}
