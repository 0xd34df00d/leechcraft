/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_ENTITYTESTHANDLERESULT_H
#define INTERFACES_ENTITYTESTHANDLERESULT_H

/** @brief The result of testing whether an entity could be handled by a
 * plugin.
 *
 * Both processing an Entity with IEntityHandler and IDownload are
 * considered to be "handling".
 *
 * The test result also includes the so-called priority which shows how
 * much the plugin is ready to handle the entity. The higher is the
 * priority, the more ready the plugin is to handle the entity.
 *
 * Typically the handler with the highest priority will be chosen.
 * A value of 0 or lower means that the given entity can't be
 * handled by this handler/downloader at all.
 */
struct EntityTestHandleResult
{
	/** @brief The priority with which an entity could be handled.
	 */
	int HandlePriority_;

	/** @brief The typical values for the priority.
	 */
	enum Priority
	{
		PIdeal = 1000,
		PHigh = 800,
		PNormal = 600,
		PLow = 200,
		PNone = 0
	};

	/** @brief Whether other handlers should be canceled.
	 *
	 * If this is set to true, then other handlers won't be called to
	 * handle the given entity.
	 */
	bool CancelOthers_;

	/** @brief Default-constructs a test result.
	 *
	 * The default-constructed entity test handle result can't handle
	 * anything.
	 */
	EntityTestHandleResult ()
	: HandlePriority_ ()
	, CancelOthers_ (false)
	{
	}

	/** @brief Constructs a test result with given predefined priority.
	 *
	 * @param[in] prio One of the predefined priorities.
	 */
	explicit EntityTestHandleResult (Priority prio)
	: HandlePriority_ (prio)
	, CancelOthers_ (false)
	{
	}
};

#endif
