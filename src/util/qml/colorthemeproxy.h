/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QColor>
#include <util/utilconfig.h>

class IColorThemeManager;

namespace LeechCraft
{
namespace Util
{
	class UTIL_API ColorThemeProxy : public QObject
	{
		Q_OBJECT

		IColorThemeManager *Manager_;
	public:
		ColorThemeProxy (IColorThemeManager*, QObject* = 0);

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
		DECL_PROP(TextBox, HighlightBorderColor)
		DECL_PROP(TextBox, TitleTextColor)
		DECL_PROP(TextBox, TextColor)
		DECL_PROP(TextBox, Aux1TextColor)
		DECL_PROP(TextBox, Aux2TextColor)
		DECL_PROP(TextBox, Aux3TextColor)

		DECL_PROP(ToolButton, TopColor)
		DECL_PROP(ToolButton, BottomColor)
		DECL_PROP(ToolButton, BorderColorColor)
		DECL_PROP(ToolButton, SelectedTopColor)
		DECL_PROP(ToolButton, SelectedBottomColor)
		DECL_PROP(ToolButton, SelectedBorderColor)

		DECL_PROP(Panel, TopColor)
		DECL_PROP(Panel, BottomColor)
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
		Q_PROPERTY(QColor color_TextBox_HighlightBorderColor READ GetTextBoxHighlightBorderColor NOTIFY colorsChanged) QColor GetTextBoxHighlightBorderColor () const { return GetColor ("TextBox", "HighlightBorderColor"); }
		Q_PROPERTY(QColor color_TextBox_TitleTextColor READ GetTextBoxTitleTextColor NOTIFY colorsChanged) QColor GetTextBoxTitleTextColor () const { return GetColor ("TextBox", "TitleTextColor"); }
		Q_PROPERTY(QColor color_TextBox_TextColor READ GetTextBoxTextColor NOTIFY colorsChanged) QColor GetTextBoxTextColor () const { return GetColor ("TextBox", "TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux1TextColor READ GetTextBoxAux1TextColor NOTIFY colorsChanged) QColor GetTextBoxAux1TextColor () const { return GetColor ("TextBox", "Aux1TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux2TextColor READ GetTextBoxAux2TextColor NOTIFY colorsChanged) QColor GetTextBoxAux2TextColor () const { return GetColor ("TextBox", "Aux2TextColor"); }
		Q_PROPERTY(QColor color_TextBox_Aux3TextColor READ GetTextBoxAux3TextColor NOTIFY colorsChanged) QColor GetTextBoxAux3TextColor () const { return GetColor ("TextBox", "Aux3TextColor"); }

		Q_PROPERTY(QColor color_ToolButton_TopColor READ GetToolButtonTopColor NOTIFY colorsChanged) QColor GetToolButtonTopColor () const { return GetColor ("ToolButton", "TopColor"); }
		Q_PROPERTY(QColor color_ToolButton_BottomColor READ GetToolButtonBottomColor NOTIFY colorsChanged) QColor GetToolButtonBottomColor () const { return GetColor ("ToolButton", "BottomColor"); }
		Q_PROPERTY(QColor color_ToolButton_BorderColorColor READ GetToolButtonBorderColorColor NOTIFY colorsChanged) QColor GetToolButtonBorderColorColor () const { return GetColor ("ToolButton", "BorderColorColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedTopColor READ GetToolButtonSelectedTopColor NOTIFY colorsChanged) QColor GetToolButtonSelectedTopColor () const { return GetColor ("ToolButton", "SelectedTopColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedBottomColor READ GetToolButtonSelectedBottomColor NOTIFY colorsChanged) QColor GetToolButtonSelectedBottomColor () const { return GetColor ("ToolButton", "SelectedBottomColor"); }
		Q_PROPERTY(QColor color_ToolButton_SelectedBorderColor READ GetToolButtonSelectedBorderColor NOTIFY colorsChanged) QColor GetToolButtonSelectedBorderColor () const { return GetColor ("ToolButton", "SelectedBorderColor"); }

		Q_PROPERTY(QColor color_Panel_TopColor READ GetPanelTopColor NOTIFY colorsChanged) QColor GetPanelTopColor () const { return GetColor ("Panel", "TopColor"); }
		Q_PROPERTY(QColor color_Panel_BottomColor READ GetPanelBottomColor NOTIFY colorsChanged) QColor GetPanelBottomColor () const { return GetColor ("Panel", "BottomColor"); }
#endif
	public slots:
		QColor setAlpha (QColor, qreal);
	private:
		QColor GetColor (const QString&, const QString&) const;
	signals:
		void colorsChanged ();
	};
}
}
