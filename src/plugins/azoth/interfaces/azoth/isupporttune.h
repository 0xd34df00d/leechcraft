/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTTUNE_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTTUNE_H
#include <QtGlobal>
#include <QMap>

namespace LC
{
namespace Azoth
{
	/** @brief Interface for publishing user tunes.
	 *
	 * This interface can be implemented by account objects (those that
	 * implement IAccount) to advertise the support for publishing
	 * currently playing tune information.
	 *
	 * @sa IAccount
	 */
	class ISupportTune
	{
	public:
		virtual ~ISupportTune () {}

		/** @brief Publishes the currently listening music information.
		 *
		 * The tuneData parameter is the map containing the following
		 * keys:
		 * - "artist" of type QString.
		 * - "title" of type QString.
		 * - "source" of type QString.
		 * - "length" of type QString.
		 * - "track" of type int.
		 */
		virtual void PublishTune (const QMap<QString, QVariant>& tuneData) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportTune,
		"org.Deviant.LeechCraft.Azoth.ISupportTune/1.0")

#endif
