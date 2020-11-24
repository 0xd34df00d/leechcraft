/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "findnotification.h"
#include <QShortcut>
#include <interfaces/core/ishortcutproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "clearlineeditaddon.h"
#include "util/shortcuts/util.h"
#include "ui_findnotification.h"

namespace LC
{
namespace Util
{
	FindNotification::FindNotification (ICoreProxy_ptr proxy, QWidget *parent)
	: Util::PageNotification { parent }
	, Ui_ { std::make_unique<Ui::FindNotification> () }
	, EscShortcut_ { new QShortcut { Qt::Key_Escape, this, SLOT (reject ()) } }
	{
		Ui_->setupUi (this);

		setFocusProxy (Ui_->Pattern_);

		EscShortcut_->setContext (Qt::WidgetWithChildrenShortcut);

		const auto addon = new Util::ClearLineEditAddon { proxy, Ui_->Pattern_ };
		addon->SetEscClearsEdit (false);

		const auto coreInstance = proxy->GetPluginsManager ()->
				GetPluginByID ("org.LeechCraft.CoreInstance");
		const auto scProxy = proxy->GetShortcutProxy ();

		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Show"),
				[this]
				{
					show ();
					setFocus ();
				},
				parent);
		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Next"),
				this, SLOT (findNext ()), parent);
		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Prev"),
				this, SLOT (findPrevious ()), parent);

		connect (Ui_->Pattern_,
				&QLineEdit::textChanged,
				[this] (const auto& str) { Ui_->FindButton_->setEnabled (!str.isEmpty ()); });
		connect (Ui_->FindButton_,
				&QPushButton::released,
				[this]
				{
					auto flags = GetFlags ();
					if (Ui_->SearchBackwards_->checkState () == Qt::Checked)
						flags |= FindBackwards;
					handleNext (Ui_->Pattern_->text (), flags);
				});
	}

	FindNotification::~FindNotification () = default;

	void FindNotification::SetEscCloses (bool close)
	{
		EscShortcut_->setEnabled (close);
	}

	void FindNotification::SetText (const QString& text)
	{
		Ui_->Pattern_->setText (text);
	}

	QString FindNotification::GetText () const
	{
		return Ui_->Pattern_->text ();
	}

	void FindNotification::SetSuccessful (bool success)
	{
		auto ss = QString { "QLineEdit {"
				"background-color:rgb(" };
		if (!success)
			ss.append ("255,0,0");
		else
		{
			auto color = QApplication::palette ().color (QPalette::Base);
			color.setRedF (color.redF () / 2);
			color.setBlueF (color.blueF () / 2);

			int r = 0, g = 0, b = 0;
			color.getRgb (&r, &g, &b);

			ss.append (QString ("%1,%2,%3")
					.arg (r)
					.arg (g)
					.arg (b));
		}
		ss.append (") }");
		Ui_->Pattern_->setStyleSheet (ss);
	}

	auto FindNotification::GetFlags () const -> FindFlags
	{
		FindFlags flags;
		if (Ui_->MatchCase_->checkState () == Qt::Checked)
			flags |= FindCaseSensitively;
		if (Ui_->WrapAround_->checkState () == Qt::Checked)
			flags |= FindWrapsAround;
		return flags;
	}

	void FindNotification::findNext ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		handleNext (text, GetFlags ());
	}

	void FindNotification::findPrevious ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		handleNext (text, GetFlags () | FindBackwards);
	}

	void FindNotification::clear ()
	{
		SetText ({});
	}

	void FindNotification::reject ()
	{
		Ui_->Pattern_->clear ();
		hide ();
	}
}
}
