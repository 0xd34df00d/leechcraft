/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QFont;

/** @brief Interface to aid WebKit-like-view-containing tabs to expose the view
 * fonts configuration to the user.
 *
 * The tabs implementing this interface should just be registered with an
 * instance of LC::Util::WkFontsWidget, which will take care of
 * the rest.
 *
 * @sa ITabWidget
 */
class Q_DECL_EXPORT IWkFontsSettable
{
protected:
	virtual ~IWkFontsSettable () = default;
public:
	/** @brief Enumeration for possible font families.
	 *
	 */
	enum class FontFamily
	{
		StandardFont,
		FixedFont,
		SerifFont,
		SansSerifFont,
		CursiveFont,
		FantasyFont
	};

	/** @brief Enumeration for possible font sizes.
	 */
	enum class FontSize
	{
		MinimumFontSize,
		MinimumLogicalFontSize,
		DefaultFontSize,
		DefaultFixedFontSize
	};

	/** @brief Returns this tab as a QObject.
	 *
	 * @return This tab as a QObject.
	 */
	virtual QObject* GetQObject () = 0;

	/** @brief Sets the \em font for the given font \em family.
	 *
	 * See also <code>QWebSettings::setFontFamily()</code>.
	 *
	 * @param[in] family The font family to change.
	 * @param[in] font The font to set for the font family.
	 */
	virtual void SetFontFamily (FontFamily family, const QFont& font) = 0;

	/** @brief Sets the \em size for the given font size \em type.
	 *
	 * See also <code>QWebSettings::setFontSize()</code>.
	 *
	 * @param[in] type The font type to change.
	 * @param[in] size The font size to set.
	 */
	virtual void SetFontSize (FontSize type, int size) = 0;
};

inline uint qHash (IWkFontsSettable::FontFamily f)
{
	return static_cast<uint> (f);
}

inline uint qHash (IWkFontsSettable::FontSize f)
{
	return static_cast<uint> (f);
}

Q_DECLARE_INTERFACE (IWkFontsSettable, "org.LeechCraft.IWkFontsSettable/1.0")
