/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QColor;

/** @brief Proxy class to the color theme management engine.
 *
 * @sa LC::Util::ColorThemeProxy.
 */
class Q_DECL_EXPORT IColorThemeManager
{
public:
	virtual ~IColorThemeManager () {}

	/** @brief Returns the color for the given QML section and key.
	 *
	 * See the LC::Util::ColorThemeProxy for the list of
	 * available color sections and their keys. See the documentation
	 * for that class for the list of possible sections and keys (they
	 * are separated by the \em _ sign).
	 *
	 * @param[in] section The color section like \em ToolButton or
	 * \em Panel.
	 * @param[in] key The key in the \em section like \em TopColor in a
	 * \em ToolButton.
	 * @return The given color for the \em section and \em key, or an
	 * invalid color if the combination is invalid.
	 */
	virtual QColor GetQMLColor (const QByteArray& section, const QByteArray& key) = 0;

	/** @brief Returns the manager as a QObject.
	 *
	 * Use this function to connect to the themeChanged() signal.
	 *
	 * @sa themeChanged()
	 */
	virtual QObject* GetQObject () = 0;
protected:
	/** @brief Emitted after the color theme changes.
	 *
	 * Use the GetQObject() method to get a QObject that emits this signal.
	 *
	 * @sa GetQObject()
	 */
	virtual void themeChanged () = 0;
};

Q_DECLARE_INTERFACE (IColorThemeManager, "org.Deviant.LeechCraft.IColorThemeManager/1.0")
