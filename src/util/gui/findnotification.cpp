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

namespace LC::Util
{
	FindNotification::FindNotification (const ICoreProxy_ptr& proxy, QWidget *parent)
	: Util::PageNotification { parent }
	, Ui_ { std::make_unique<Ui::FindNotification> () }
	, EscShortcut_ { new QShortcut { Qt::Key_Escape, this, this, &FindNotification::Reject } }
	{
		Ui_->setupUi (this);

		setFocusProxy (Ui_->Pattern_);

		EscShortcut_->setContext (Qt::WidgetWithChildrenShortcut);

		const auto addon = new Util::ClearLineEditAddon { proxy, Ui_->Pattern_ };
		addon->SetEscClearsEdit (false);

		const auto coreInstance = proxy->GetPluginsManager ()->GetPluginByID ("org.LeechCraft.CoreInstance");
		const auto scProxy = proxy->GetShortcutProxy ();

		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Show"),
				[this]
				{
					show ();
					setFocus ();
				},
				parent);
		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Next"),
				this, &FindNotification::FindNext, parent);
		CreateShortcuts (scProxy->GetShortcuts (coreInstance, "Find.Prev"),
				this, &FindNotification::FindPrevious, parent);

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
					HandleNext (Ui_->Pattern_->text (), flags);
				});
		connect (Ui_->CloseButton_,
				&QPushButton::released,
				this,
				&FindNotification::Reject);
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
		auto ss = QStringLiteral ("QLineEdit { background-color: ");
		if (!success)
			ss.append ("#FF0000");
		else
		{
			auto color = QApplication::palette ().color (QPalette::Base);
			color.setRedF (color.redF () / 2);
			color.setBlueF (color.blueF () / 2);
			ss.append (color.name ());
		}
		ss.append (" }");
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

	void FindNotification::FindNext ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		HandleNext (text, GetFlags ());
	}

	void FindNotification::FindPrevious ()
	{
		const auto& text = GetText ();
		if (text.isEmpty ())
			return;

		HandleNext (text, GetFlags () | FindBackwards);
	}

	void FindNotification::Clear ()
	{
		SetText ({});
	}

	void FindNotification::Reject ()
	{
		Ui_->Pattern_->clear ();
		hide ();
	}
}
