/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QColor>
#include "qmlconfig.h"

class IColorThemeManager;

namespace LC::Util
{
	/** @brief Proxy for QML files to use colors from current color theme.
	 *
	 * LeechCraft has a concept of color themes which allows to have a
	 * consistent look for different plugins and components, both based
	 * on good old QWidgets and for new QML ones. This class provides
	 * easy access for the current and up-to-date values of colors of the
	 * current current color theme via its properties. Property binding
	 * in QML allows one to automatically handle color theme changes and
	 * updated without recreating the components.
	 *
	 * This class should be used as following:
	 * -# an object of this class is created, passed with an
	 *    IColorThemeManager (which could be obtained from ICoreProxy);
	 * -# the object is added to the QML context of the view containing
	 *    the component via QDeclarativeContext::setContextPropery();
	 * -# the corresponding properties of component are bound to the
	 *    properties of this object.
	 *
	 * The first two steps are as easy as following:
	 * \code{.cpp}
	 * ICoreProxy_ptr proxy; // core proxy object passed to IInfo::Init()
	 * View_->rootContext ()->setContextProperty ("colorProxy",
	 *		 new Util::ColorThemeProxy (proxy->GetColorThemeManager (), parent));
	 * \endcode
	 *
	 * Here the color theme proxy object is added by the "colorProxy"
	 * name.
	 *
	 * Using the added object is pretty easy too:
	 * \code{.qml}
	 * Rectangle {
	 		anchors.fill: parent
	 		radius: 5
	 		smooth: true
	 		border.color: colorProxy.color_TextBox_BorderColor
	 		border.width: 1
	 		gradient: Gradient {
	 			GradientStop {
	 				position: 0
	 				id: upperStop
	 				color: colorProxy.color_TextBox_TopColor
	 			}
	 			GradientStop {
	 				position: 1
	 				id: lowerStop
	 				color: colorProxy.color_TextBox_BottomColor
	 			}
	 		}
	 	}
	   \endcode
	 *
	 * The colors can also be used in the states and dynamic elements,
	 * for example:
	 * \code{.qml}
	  states: [
	 		State {
	 			name: "hovered"
	 			PropertyChanges { target: tabRect; border.color: colorProxy.color_TextBox_HighlightBorderColor }
	 			PropertyChanges { target: upperStop; color: colorProxy.color_TextBox_HighlightTopColor }
	 			PropertyChanges { target: lowerStop; color: colorProxy.color_TextBox_HighlightBottomColor }
	 		}
	 	]
	 	transitions: [
	 		Transition {
	 			from: ""
	 			to: "hovered"
	 			reversible: true
	 			PropertyAnimation { properties: "border.color"; duration: 200 }
	 			PropertyAnimation { properties: "color"; duration: 200 }
	 		}
	 	]
	   \endcode
	 *
	 * Good examples of color proxy usage are in LMP and SB2 plugins,
	 * for example.
	 *
	 * This class uses IColorThemeManager internally. The \em section and
	 * \em key of IColorThemeManager::GetQMLColor() form the name of the
	 * properties in this class in the form of \em color_section_key.
	 *
	 * @sa ICoreProxy
	 * @sa IInfo
	 * @sa IColorThemeManager
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API ColorThemeProxy : public QObject
	{
		Q_OBJECT

		IColorThemeManager *Manager_;
	public:
		/** @brief Constructs the color theme proxy with the given color
		 * \em manager and \em parent object.
		 *
		 * @param[in] manager The color manager to use.
		 * @param[in] parent The parent object of the proxy.
		 */
		ColorThemeProxy (IColorThemeManager *manager, QObject *parent);

#ifdef GEN_RUN
#define DECL_PROP(group,col) \
		Q_PROPERTY(QColor color_##group##_##col READ Get##group##col NOTIFY colorsChanged) \
		QColor Get##group##col () const { return GetColor (#group, #col); }

		DECL_PROP(TextView, TopColor)
		DECL_PROP(TextView, BottomColor)
		DECL_PROP(TextView, TitleTextColor)
		DECL_PROP(TextView, TextColor)
		DECL_PROP(TextView, Aux1TextColor)
		DECL_PROP(TextView, Aux2TextColor)
		DECL_PROP(TextView, Aux3TextColor)

		DECL_PROP(TextBox, TopColor)
		DECL_PROP(TextBox, BottomColor)
		DECL_PROP(TextBox, BorderColor)
		DECL_PROP(TextBox, HighlightTopColor)
		DECL_PROP(TextBox, HighlightBottomColor)
		DECL_PROP(TextBox, HighlightBorderColor)
		DECL_PROP(TextBox, TitleTextColor)
		DECL_PROP(TextBox, TextColor)
		DECL_PROP(TextBox, Aux1TextColor)
		DECL_PROP(TextBox, Aux2TextColor)
		DECL_PROP(TextBox, Aux3TextColor)

		DECL_PROP(ToolButton, TopColor)
		DECL_PROP(ToolButton, BottomColor)
		DECL_PROP(ToolButton, BorderColor)
		DECL_PROP(ToolButton, SelectedTopColor)
		DECL_PROP(ToolButton, SelectedBottomColor)
		DECL_PROP(ToolButton, SelectedBorderColor)
		DECL_PROP(ToolButton, HoveredTopColor)
		DECL_PROP(ToolButton, HoveredBottomColor)
		DECL_PROP(ToolButton, HoveredBorderColor)
		DECL_PROP(ToolButton, PressedBorderColor)
		DECL_PROP(ToolButton, TextColor)

		DECL_PROP(Panel, TopColor)
		DECL_PROP(Panel, BottomColor)
		DECL_PROP(Panel, TextColor)
#else
		Q_PROPERTY(QColor color_TextView_TopColor READ GetTextViewTopColor NOTIFY colorsChanged) QColor GetTextViewTopColor () const { return GetColor ("TextView", "TopColor"); }
		Q_PROPERTY(QColor color_TextView_BottomColor READ GetTextViewBottomColor NOTIFY colorsChanged) QColor GetTextViewBottomColor () const { return GetColor ("TextView", "BottomColor"); }
		Q_PROPERTY(QColor color_TextView_TitleTextColor READ GetTextViewTitleTextColor NOTIFY colorsChanged) QColor GetTextViewTitleTextColor () const { return GetColor ("TextView", "TitleTextColor"); }
		Q_PROPERTY(QColor color_TextView_TextColor READ GetTextViewTextColor NOTIFY colorsChanged) QColor GetTextViewTextColor () const { return GetColor ("TextView", "TextColor"); }
		Q_PROPERTY(QColor color_TextView_Aux1TextColor READ GetTextViewAux1TextColor NOTIFY colorsChanged) QColor GetTextViewAux1TextColor () const { return GetColor ("TextView", "Aux1TextColor"); }
		Q_PROPERTY(QColor color_TextView_Aux2TextColor READ GetTextViewAux2TextColor NOTIFY colorsChanged) QColor GetTextViewAux2TextColor () const { return GetColor ("TextView", "Aux2TextColor"); }
		Q_PROPERTY(QColor color_TextView_Aux3TextColor READ GetTextViewAux3TextColor NOTIFY colorsChanged) QColor GetTextViewAux3TextColor () const { return GetColor ("TextView", "Aux3TextColor"); }

		Q_PROPERTY(QColor color_TextBox_TopColor READ GetTextBoxTopColor NOTIFY colorsChanged) QColor GetTextBoxTopColor () const { return GetColor ("TextBox", "TopColor"); }
		Q_PROPERTY(QColor color_TextBox_BottomColor READ GetTextBoxBottomColor NOTIFY colorsChanged) QColor GetTextBoxBottomColor () const { return GetColor ("TextBox", "BottomColor"); }
		Q_PROPERTY(QColor color_TextBox_BorderColor READ GetTextBoxBorderColor NOTIFY colorsChanged) QColor GetTextBoxBorderColor () const { return GetColor ("TextBox", "BorderColor"); }
		Q_PROPERTY(QColor color_TextBox_HighlightTopColor READ GetTextBoxHighlightTopColor NOTIFY colorsChanged) QColor GetTextBoxHighlightTopColor () const { return GetColor ("TextBox", "HighlightTopColor"); }
		Q_PROPERTY(QColor color_TextBox_HighlightBottomColor READ GetTextBoxHighlightBottomColor NOTIFY colorsChanged) QColor GetTextBoxHighlightBottomColor () const { return GetColor ("TextBox", "HighlightBottomColor"); }
		Q_PROPERTY(QColor color_TextBox_HighlightBorderColor READ GetTextBoxHighlightBorderColor NOTIFY colorsChanged) QColor GetTextBoxHighlightBorderColor () const { return GetColor ("TextBox", "HighlightBorderColor"); }
		Q_PROPERTY(QColor color_TextBox_TitleTextColor READ GetTextBoxTitleTextColor NOTIFY colorsChanged) QColor GetTextBoxTitleTextColor () const { return GetColor ("TextBox", "TitleTextColor"); }
		Q_PROPERTY(QColor color_TextBox_TextColor READ GetTextBoxTextColor NOTIFY colorsChanged) QColor GetTextBoxTextColor () const { return GetColor ("TextBox", "TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux1TextColor READ GetTextBoxAux1TextColor NOTIFY colorsChanged) QColor GetTextBoxAux1TextColor () const { return GetColor ("TextBox", "Aux1TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux2TextColor READ GetTextBoxAux2TextColor NOTIFY colorsChanged) QColor GetTextBoxAux2TextColor () const { return GetColor ("TextBox", "Aux2TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux3TextColor READ GetTextBoxAux3TextColor NOTIFY colorsChanged) QColor GetTextBoxAux3TextColor () const { return GetColor ("TextBox", "Aux3TextColor"); }

		Q_PROPERTY(QColor color_ToolButton_TopColor READ GetToolButtonTopColor NOTIFY colorsChanged) QColor GetToolButtonTopColor () const { return GetColor ("ToolButton", "TopColor"); }
		Q_PROPERTY(QColor color_ToolButton_BottomColor READ GetToolButtonBottomColor NOTIFY colorsChanged) QColor GetToolButtonBottomColor () const { return GetColor ("ToolButton", "BottomColor"); }
		Q_PROPERTY(QColor color_ToolButton_BorderColor READ GetToolButtonBorderColor NOTIFY colorsChanged) QColor GetToolButtonBorderColor () const { return GetColor ("ToolButton", "BorderColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedTopColor READ GetToolButtonSelectedTopColor NOTIFY colorsChanged) QColor GetToolButtonSelectedTopColor () const { return GetColor ("ToolButton", "SelectedTopColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedBottomColor READ GetToolButtonSelectedBottomColor NOTIFY colorsChanged) QColor GetToolButtonSelectedBottomColor () const { return GetColor ("ToolButton", "SelectedBottomColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedBorderColor READ GetToolButtonSelectedBorderColor NOTIFY colorsChanged) QColor GetToolButtonSelectedBorderColor () const { return GetColor ("ToolButton", "SelectedBorderColor"); }
		Q_PROPERTY(QColor color_ToolButton_HoveredTopColor READ GetToolButtonHoveredTopColor NOTIFY colorsChanged) QColor GetToolButtonHoveredTopColor () const { return GetColor ("ToolButton", "HoveredTopColor"); }
		Q_PROPERTY(QColor color_ToolButton_HoveredBottomColor READ GetToolButtonHoveredBottomColor NOTIFY colorsChanged) QColor GetToolButtonHoveredBottomColor () const { return GetColor ("ToolButton", "HoveredBottomColor"); }
		Q_PROPERTY(QColor color_ToolButton_HoveredBorderColor READ GetToolButtonHoveredBorderColor NOTIFY colorsChanged) QColor GetToolButtonHoveredBorderColor () const { return GetColor ("ToolButton", "HoveredBorderColor"); }
		Q_PROPERTY(QColor color_ToolButton_PressedBorderColor READ GetToolButtonPressedBorderColor NOTIFY colorsChanged) QColor GetToolButtonPressedBorderColor () const { return GetColor ("ToolButton", "PressedBorderColor"); }
		Q_PROPERTY(QColor color_ToolButton_TextColor READ GetToolButtonTextColor NOTIFY colorsChanged) QColor GetToolButtonTextColor () const { return GetColor ("ToolButton", "TextColor"); }

		Q_PROPERTY(QColor color_Panel_TopColor READ GetPanelTopColor NOTIFY colorsChanged) QColor GetPanelTopColor () const { return GetColor ("Panel", "TopColor"); }
		Q_PROPERTY(QColor color_Panel_BottomColor READ GetPanelBottomColor NOTIFY colorsChanged) QColor GetPanelBottomColor () const { return GetColor ("Panel", "BottomColor"); }
		Q_PROPERTY(QColor color_Panel_TextColor READ GetPanelTextColor NOTIFY colorsChanged) QColor GetPanelTextColor () const { return GetColor ("Panel", "TextColor"); }
#endif
	public slots:
		/** @brief Returns the color with the alpha channel set to the given value.
		 *
		 * @param[in] color The color whose alpha value should be changed.
		 * @param[in] alpha The new alpha value. 0 is fully transparent,
		 * 1 is fully opaque.
		 */
		QColor setAlpha (QColor color, qreal alpha);
	private:
		QColor GetColor (const QByteArray&, const QByteArray&) const;
	signals:
		/** @brief Emitted when the color theme changes.
		 *
		 * This signal can be used to be notified of the properties
		 * changes.
		 */
		void colorsChanged ();
	};
}
