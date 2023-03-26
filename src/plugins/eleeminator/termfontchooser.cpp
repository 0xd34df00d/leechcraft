/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "termfontchooser.h"
#include <optional>
#include <QFont>
#include <QFontDialog>
#include <qtermwidget.h>
#include "xmlsettingsmanager.h"

namespace LC::Eleeminator
{
	void SetDefaultFont (const QFont& font)
	{
		XmlSettingsManager::Instance ().setProperty ("Font", QVariant::fromValue (font));
	}

	namespace
	{
		std::optional<QFont> GetSavedFont ()
		{
			const auto& savedFontVar = XmlSettingsManager::Instance ().property ("Font");
			if (!savedFontVar.isNull () && savedFontVar.canConvert<QFont> ())
				return savedFontVar.value<QFont> ();

			return {};
		}

		void SelectFont (QTermWidget& term)
		{
			const auto& currentFont = term.getTerminalFont ();

			bool ok = false;
			const auto& font = QFontDialog::getFont (&ok, currentFont, &term);
			if (!ok)
				return;

			term.setTerminalFont (font);
			SetDefaultFont (font);
		}
	}

	std::unique_ptr<QAction> MakeFontChooser (QTermWidget& term)
	{
		if (const auto& font = GetSavedFont ())
			term.setTerminalFont (*font);

		auto action = std::make_unique<QAction> (QObject::tr ("Select font..."));
		action->setProperty ("ActionIcon", "preferences-desktop-font");
		QObject::connect (action.get (),
				&QAction::triggered,
				&term,
				[&term] { SelectFont (term); });
		return action;
	}
}
