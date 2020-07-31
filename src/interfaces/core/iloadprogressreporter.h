/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

/** @brief Describes the progress of a single long-running operation.
 *
 * @sa ILoadProgressReporter
 */
class Q_DECL_EXPORT ILoadProcess
{
public:
	virtual ~ILoadProcess () {}

	/** @brief Sets the current progress to \em value.
	 *
	 * @param[in] value The new value for the progress.
	 */
	virtual void ReportValue (int value) = 0;

	/** @brief Increments the current value for the progress.
	 */
	virtual void operator++ () = 0;
};

using ILoadProcess_ptr = std::shared_ptr<ILoadProcess>;

/** @brief Interface for reporting progress of some long-running operation
 * during load time.
 *
 * This object can be used to track multiple operations at once. Each
 * operation corresponds to an object returned by InitiateProcess() for
 * that operation.
 *
 * This object should be used by any plugin doing some long-running
 * operation during LeechCraft load (like a DB migration) because of
 * usability reasons.
 *
 * @sa IPluginManager::CreateLoadProgressReporter()
 * @sa ILoadProcess
 */
class Q_DECL_EXPORT ILoadProgressReporter
{
public:
	virtual ~ILoadProgressReporter () {}

	/** @brief Notifies about a specific long-running process during load.
	 *
	 * @param[in] title The human-readable title of the operation.
	 * @param[in] min The initial number of steps already done.
	 * @param[in] max The total number of steps that should be done by.
	 * @return The object used to track this exact operation.
	 */
	virtual ILoadProcess_ptr InitiateProcess (const QString& title, int min, int max) = 0;
};

using ILoadProgressReporter_ptr = std::shared_ptr<ILoadProgressReporter>;

Q_DECLARE_INTERFACE (ILoadProgressReporter, "org.Deviant.LeechCraft.ILoadProgressReporter/1.0")
