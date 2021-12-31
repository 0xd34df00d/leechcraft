/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "utilconfig.h"
#include <QString>

class QTranslator;
class QLocale;
class QAction;
class QImage;
class QObject;

template<typename T>
class QList;

class QModelIndex;

namespace LC::Util
{
	/** @brief The "leechcraft" literal, with no run-time overhead.
	 */
	Q_DECL_IMPORT extern const QString LCLowercase;

	/** @brief Returns the given image in a Base64-encoded form.
	 *
	 * The return result is suitable for inserting into
	 * <code>&lt;img></code>'s <code>src</code> attribute as is.
	 *
	 * @param[in] image The image to represent as Base64-encoded
	 * form.
	 * @return The source string.
	 */
	UTIL_API QString GetAsBase64Src (const QImage& image);

	/** @brief Makes a formatted size from number.
	 *
	 * Converts, for example, 1048576 to 1.0 MB.
	 *
	 * @param[in] sourceSize Size in bytes.
	 * @return Formatted string.
	 *
	 * @sa MakeTimeFromLong()
	 * @sa MakePrettySizeShort()
	 */
	UTIL_API QString MakePrettySize (qint64 sourceSize);

	/** @brief Converts a bytes count to a string representation with
	 * appropriately chosen units.
	 *
	 * Converts, for example, <em>1048576</em> to <em>1.0 M</em>.
	 *
	 * As opposed to MakePrettySize(), this function tries to keep the
	 * returned string short.
	 *
	 * @param[in] size Size in bytes.
	 * @return Formatted string in relevant units.
	 *
	 * @sa MakeTimeFromLong()
	 * @sa MakePrettySize()
	 */
	UTIL_API QString MakePrettySizeShort (qint64 size);

	/** @brief Makes a formatted time from number.
	 *
	 * Converts, for example 256 to 00:04:16.
	 *
	 * @param[in] time Time interval in seconds.
	 * @return DateTime object.
	 *
	 * @sa MakePrettySize
	 */
	UTIL_API QString MakeTimeFromLong (ulong time);

	UTIL_API QTranslator* LoadTranslator (const QString& base,
			const QString& locale,
			const QString& prefix = LCLowercase,
			const QString& appname = LCLowercase);

	/** @brief Loads and installs a translator.
	 *
	 * Attempts to load and install a translator for the current
	 * locale. The name is formed like this:
	 * 'prefix_' + base + '_' + locale
	 * If base is an empty string, the second _ isn't appended.
	 *
	 * First resources are searched (:/), then APPDIR/translations
	 * on Windows and /usr/[local/]share/appname/translations on
	 * Unix.
	 *
	 * @param[in] base Base name of the translation file.
	 * @param[in] prefix The optional prefix of the translation
	 * (useful if it's not LC's one).
	 * @param[in] appname Base name of the application.
	 * @return The translator object if loading is successful, NULL
	 * otherwise.
	 */
	UTIL_API QTranslator* InstallTranslator (const QString& base,
			const QString& prefix = LCLowercase,
			const QString& appname = LCLowercase);

	/** @brief Returns the current locale name, like en_US.
	 *
	 * First, this function checks the locale value stored in
	 * "Language" key of settings object with organizationName() and
	 * applicationName(). If it's equal to "system", this function
	 * queries the LANG environment variable, and if it is empty or
	 * in invalid format (not like en_US), it takes the value of
	 * QLocale::system().name().
	 *
	 * Then, if the resulting name the name of the language only,
	 * GetLocaleName() tries to find any countries for that
	 * language. If any countries are found, the code of the first
	 * found country is appended, else "_00" is appended.
	 *
	 * @return Current locale name.
	 *
	 * @sa GetLanguage()
	 */
	UTIL_API QString GetLocaleName ();

	UTIL_API QString GetInternetLocaleName (const QLocale&);

	/** @brief Returns the current language name.
	 *
	 * This function works as GetLocaleName() except it doesn't
	 * return (and doesn't query for) country name.
	 *
	 * @return Current language name.
	 *
	 * @sa GetLocaleName()
	 */
	UTIL_API QString GetLanguage ();

	UTIL_API QList<QModelIndex> GetSummarySelectedRows (QObject *sender);

	/** @brief Returns the action that is set to act as a separator.
	 *
	 * That is the action with setSeparator(true);
	 *
	 * @param[in] parent The parent of the action.
	 * @return The separator action.
	 */
	UTIL_API QAction* CreateSeparator (QObject *parent);
}
