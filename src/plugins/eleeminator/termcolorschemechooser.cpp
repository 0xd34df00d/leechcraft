/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "termcolorschemechooser.h"
#include <QMenu>
#include <QToolButton>
#include <qtermwidget.h>
#include "colorschemesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Eleeminator
{
	class ChooserState : public QObject
	{
		QTermWidget& Term_;
		const ColorSchemesManager& ColorSchemes_;

		QString Current_;
	public:
		explicit ChooserState (QTermWidget&, const ColorSchemesManager&);

		QMenu* MakeMenu (QWidget&);
	private:
		void SetScheme (const QString&);
	};

	ChooserState::ChooserState (QTermWidget& term, const ColorSchemesManager& colorSchemes)
	: QObject { &term }
	, Term_ { term }
	, ColorSchemes_ { colorSchemes }
	, Current_ { XmlSettingsManager::Instance ().Property ("LastColorScheme", "Linux").toString () }
	{
		term.setColorScheme (Current_);
	}

	QMenu* ChooserState::MakeMenu (QWidget& parent)
	{
		auto menu = new QMenu { tr ("Color scheme"), &parent };
		menu->menuAction ()->setProperty ("ActionIcon", "fill-color");
		connect (menu,
				&QMenu::aboutToHide,
				this,
				[this] { Term_.setColorScheme (Current_); });

		const auto actionGroup = new QActionGroup { this };
		for (const auto& scheme : ColorSchemes_.GetSchemes ())
		{
			const auto act = menu->addAction (scheme.Name_);
			act->setCheckable (true);
			act->setChecked (scheme.ID_ == Current_);
			actionGroup->addAction (act);

			const auto& schemeId = scheme.ID_;
			connect (act,
					&QAction::triggered,
					this,
					[this, schemeId] { SetScheme (schemeId); });
			connect (act,
					&QAction::hovered,
					this,
					[this, schemeId] { Term_.setColorScheme (schemeId); });
		}

		return menu;
	}

	void ChooserState::SetScheme (const QString& schemeId)
	{
		Term_.setColorScheme (schemeId);
		Current_ = schemeId;
		XmlSettingsManager::Instance ().setProperty ("LastColorScheme", schemeId);
	}

	QToolButton& MakeColorChooser (QTermWidget& term, const ColorSchemesManager& colorSchemes)
	{
		auto& button = *new QToolButton;
		button.setPopupMode (QToolButton::InstantPopup);
		button.setMenu ((new ChooserState { term, colorSchemes })->MakeMenu (button));
		button.setProperty ("ActionIcon", "fill-color");
		return button;
	}
}
