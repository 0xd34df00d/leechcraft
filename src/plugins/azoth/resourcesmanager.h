/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>
#include <QHash>
#include "interfaces/azoth/azothcommon.h"

class QIcon;
class QIODevice;

namespace LC
{
namespace Util
{
	class ResourceLoader;

	typedef std::shared_ptr<QIODevice> QIODevice_ptr;
}

namespace Azoth
{
	class ICLEntry;

	class ResourcesManager : public QObject
	{
		Q_OBJECT
	public:
		enum LoaderType
		{
			RLTStatusIconLoader,
			RLTClientIconLoader,
			RLTAffIconLoader,
			RLTSystemIconLoader,
			RLTActivityIconLoader,
			RLTMoodIconLoader
		};
	private:
		QMap<LoaderType, std::shared_ptr<Util::ResourceLoader>> ResourceLoaders_;

		typedef QHash<ICLEntry*, QMap<QString, QIcon>> EntryClientIconCache_t;
		EntryClientIconCache_t EntryClientIconCache_;

		ResourcesManager ();

		ResourcesManager (const ResourcesManager&) = delete;
		ResourcesManager& operator= (const ResourcesManager&) = delete;
	public:
		static ResourcesManager& Instance ();

		Util::ResourceLoader* GetResourceLoader (LoaderType) const;

		void HandleEntry (ICLEntry*);
		void HandleRemoved (ICLEntry*);

		/** Returns the name of the icon from the current iconset for
		 * the given contact list entry state.
		 */
		Util::QIODevice_ptr GetIconPathForState (State state) const;

		/** Returns an icon from the current iconset for the given
		 * contact list entry state.
		 */
		QIcon GetIconForState (State state) const;

		/** Returns an icon from the current iconset for the given
		 * affiliation.
		 */
		QIcon GetAffIcon (const QByteArray& affName) const;

		/** @brief Returns icons for the given CL entry.
		 *
		 * This function returns an icon for each variant of the entry,
		 * since different variants may have different clients. If the
		 * protocol which the entry belongs doesn't support variants,
		 * the map would have only one key/value pair of null QString
		 * and corresponding icon.
		 *
		 * This function returns the icons from the currently selected
		 * (in settings) iconset.
		 *
		 * @param[in] entry Entry for which to return the icons.
		 * @return Map from entity variant to corresponding
		 * client icon.
		 */
		QMap<QString, QIcon> GetClientIconForEntry (ICLEntry *entry);

		QImage GetDefaultAvatar (int size) const;
	private slots:
		/** Removes the entries in the client icon cache for the sender,
		 * if obj is null, or for obj, if it is not null.
		 *
		 * If the object can't be casted to ICLEntry, this function does
		 * nothing.
		 */
		void invalidateClientsIconCache (QObject *obj = 0);
		void invalidateClientsIconCache (ICLEntry*);
		void flushIconCaches ();
	};
}
}
