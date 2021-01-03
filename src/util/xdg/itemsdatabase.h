/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "itemsfinder.h"

class QFileSystemWatcher;

namespace LC
{
namespace Util
{
namespace XDG
{
	/** @brief An ItemsFinder automatically watching for changes in
	 * <code>.desktop</code> files.
	 *
	 * The only difference between this class and ItemsFinder is in this
	 * class automatically watching for changes in the directories
	 * matching the types passed to its constructor. The changes include
	 * both updates to the existing files as well as addition of new files
	 * and removal of already existing ones.
	 *
	 * Refer to the documentation for ItemsFinder for more information.
	 *
	 * @sa ItemsFinder
	 */
	class UTIL_XDG_API ItemsDatabase : public ItemsFinder
	{
		Q_OBJECT

		bool UpdateScheduled_ = false;
		QFileSystemWatcher * const Watcher_;
	public:
		/** @brief Creates the ItemsDatabase for the given \em types.
		 *
		 * @param[in] proxy The proxy to use to get the icons of the
		 * items that were found.
		 * @param[in] types The item types to watch for.
		 * @param[in] parent The parent object of this finder.
		 *
		 * @sa ItemsFinder::ItemsFinder
		 */
		ItemsDatabase (const ICoreProxy_ptr& proxy, const QList<Type>& types, QObject *parent = nullptr);
	private slots:
		void scheduleUpdate ();
	};
}
}
}
