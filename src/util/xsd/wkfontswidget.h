/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <QHash>
#include <interfaces/iwkfontssettable.h>
#include "xsdconfig.h"

class QSpinBox;

namespace Ui
{
	class WkFontsWidget;
}

namespace LC
{
namespace Util
{
	class BaseSettingsManager;
	class FontChooserWidget;

	/** @brief A settings widget for configuring WebKit fonts.
	 *
	 * Provides a common widget for configuring QtWebKit fonts for
	 * standard WebKit font types.
	 *
	 * This widget works through LeechCraft's XML Settings Dialog system,
	 * storing the configuration in an BaseSettingsManager instance.
	 *
	 * This widget also supports automatically updating font settings for
	 * objects implementing the IWkFontsSettable interface if the user
	 * changes them.
	 *
	 * Typical usage includes creating an item of type
	 * <code>customwidget</code> in the XML file describing the settings
	 * and then setting an instance of the WkFontsWidget as the widget
	 * for that item. On the C++ side this looks like:
	 *  \code{.cpp}
		FontsWidget_ = new Util::WkFontsWidget { &XmlSettingsManager::Instance () };
		XmlSettingsDialog_->SetCustomWidget ("FontsSelector", FontsWidget_);
	    \endcode
	 * assuming the <code>Util::BaseSettingsManager</code> is provided by
	 * a singleton XmlSettingsManager class, and
	 * <code>XmlSettingsDialog_</code> is an instance of
	 * <code>Util::XmlSettingsDialog</code>.
	 *
	 * The code above also stores the WkFontsWidget as a class variable,
	 * which may be a good idea if one wishes to use the settings
	 * autoupdate feature. For example, assuming a class
	 * <code>ChatTab</code> properly implements the IWkFontsSettable
	 * interface:
	 * \code{.cpp}
		const auto tab = new ChatTab {};
		FontsWidget_->RegisterSettable (tab);
	   \endcode
	 *
	 * @sa IWkFontsSettable
	 */
	class UTIL_XSD_API WkFontsWidget : public QWidget
	{
		Q_OBJECT

		std::shared_ptr<Ui::WkFontsWidget> Ui_;
		BaseSettingsManager * const BSM_;

		QHash<IWkFontsSettable::FontFamily, FontChooserWidget*> Family2Chooser_;
		QHash<IWkFontsSettable::FontFamily, std::string_view> Family2Name_;
		QHash<IWkFontsSettable::FontFamily, QFont> PendingFontChanges_;

		QHash<IWkFontsSettable::FontSize, QSpinBox*> Size2Spinbox_;
		QHash<IWkFontsSettable::FontSize, std::string_view> Size2Name_;
		QHash<IWkFontsSettable::FontSize, int> PendingSizeChanges_;

		QList<IWkFontsSettable*> Settables_;
	public:
		/** @brief Creates the fonts settings widget.
		 *
		 * @param[in] bsm The settings manager to use for storing
		 * settings.
		 * @param[in] parent The parent widget for this widget.
		 */
		WkFontsWidget (Util::BaseSettingsManager *bsm, QWidget *parent = nullptr);

		/** @brief Registers an object to be automatically updated
		 * whenever font settings change.
		 *
		 * @param[in] settable An object implementing IWkFontsSettable.
		 *
		 * @sa IWkFontsSettable
		 */
		void RegisterSettable (IWkFontsSettable *settable);

		/** @brief Sets the \em size for the given font size \em type.
		 *
		 * The new size is stored in settings and applied to all settables
		 * registered with this widget via RegisterSettable().
		 *
		 * @param type The type of font size to change.
		 * @param size The new size.
		 */
		void SetSize (IWkFontsSettable::FontSize type, int size);
	private:
		void ResetFontChoosers ();
		void ResetSizeChoosers ();

		void ApplyPendingSizeChanges ();
	private slots:
		void on_ChangeAll__released ();
	public slots:
		void accept ();
		void reject ();
	signals:
		/** @brief Notifies the \em font for the given \em family has been
		 * changed.
		 *
		 * @param[out] family The font family for which the \em font has
		 * been changed.
		 * @param[out] font The new font for the given \em family.
		 */
		void fontChanged (IWkFontsSettable::FontFamily family, const QFont& font);

		/** @brief Notifies the \em size for the given font \em type has
		 * been changed.
		 *
		 * @param[out] type The font type for which the \em size has been
		 * changed.
		 * @param[out] size The new font size for the given font \em type.
		 */
		void sizeChanged (IWkFontsSettable::FontSize type, int size);

		/** @brief Notifies the text zoom \em factor has been changed.
		 *
		 * @param[out] factor The new font size multiplier.
		 */
		void sizeMultiplierChanged (qreal factor);
	};
}
}
