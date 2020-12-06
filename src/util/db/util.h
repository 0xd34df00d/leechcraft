/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVariant>
#include <QSqlQuery>
#include <QtDebug>
#include "dbconfig.h"

class QSqlDatabase;
class QString;

namespace LC::Util
{
	/** @brief Runs the given query \em text on the given \em db.
	 *
	 * Prepares and executes a QSqlQuery containing the given query
	 * \em text on the given \em db. If the query fails, an exception
	 * is thrown.
	 *
	 * @param[in] db The database to execute the query \em text on.
	 * @param[in] text The text of the query to be executed.
	 * @return The query object, which may be used to retrieve the query
	 * results.
	 *
	 * @throws std::exception If the query execution failed.
	 *
	 * @ingroup DbUtil
	 */
	UTIL_DB_API QSqlQuery RunTextQuery (const QSqlDatabase& db, const QString& text);

	/** @brief Loads the query text from the given resource file.
	 *
	 * Loads the query text from the resources for the given \em plugin
	 * and exact \em filename in it.
	 *
	 * The file name to be loaded is formed as
	 * <code>:/${plugin}/resources/sql/${filename}.sql</code>.
	 *
	 * @param[in] plugin The name of the plugin whose resources should be
	 * used.
	 * @param[in] filename The name of the file under
	 * <code>${plugin}/resources/sql</code>.
	 * @return The query text in the given file.
	 *
	 * @throws std::exception If the given file cannot be opened.
	 *
	 * @sa RunQuery()
	 *
	 * @ingroup DbUtil
	 */
	UTIL_DB_API QString LoadQuery (const QString& plugin, const QString& filename);

	/** @brief Loads the query from the given resource file and runs it.
	 *
	 * Loads the query text from the resources for the given \em plugin
	 * and exact \em filename in it and executes it on the given \em db.
	 *
	 * The file name to be loaded is formed as
	 * <code>:/${plugin}/resources/sql/${filename}.sql</code>.
	 *
	 * @param[in] db The database to execute the query on.
	 * @param[in] plugin The name of the plugin whose resources should be
	 * used.
	 * @param[in] filename The name of the file under
	 * <code>${plugin}/resources/sql</code>.
	 *
	 * @throws std::exception If the given file cannot be opened or if the
	 * query execution failed.
	 *
	 * @sa LoadQuery()
	 *
	 * @ingroup DbUtil
	 */
	UTIL_DB_API void RunQuery (const QSqlDatabase& db, const QString& plugin, const QString& filename);

	/** @brief Gets the last insert ID for the given query.
	 *
	 * @tparam T The type of the last ID.
	 * @param[in] query The query whose last insert ID should be retrieved.
	 * @return The last insert ID of type \em T.
	 *
	 * @throws std::runtime_error If no last insert ID has been reported,
	 * or if the last insert ID cannot be converted to type \em T.
	 *
	 * @ingroup DbUtil
	 */
	template<typename T = int>
	T GetLastId (const QSqlQuery& query)
	{
		const auto& lastVar = query.lastInsertId ();
		if (lastVar.isNull ())
			throw std::runtime_error { "No last ID has been reported." };

		if (!lastVar.canConvert<T> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot convert"
					<< lastVar;
			throw std::runtime_error { "Cannot convert last ID." };
		}

		return lastVar.value<T> ();
	}

	/** @brief Generates an unique thread-safe connection name.
	 *
	 * This function generates a connection name using the given \em base
	 * that is unique across all threads.
	 *
	 * @param[in] base The identifier base to be used to generate the
	 * unique connection string.
	 * @return An unique connection name across all threads.
	 *
	 * @ingroup DbUtil
	 */
	UTIL_DB_API QString GenConnectionName (const QString& base);
}
