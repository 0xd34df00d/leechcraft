/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "confwidget.h"
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Azoth
{
namespace Herbicide
{
	ConfWidget::ConfWidget (Util::BaseSettingsManager *bsm, QWidget *parent)
	: QWidget { parent }
	, BSM_ { bsm }
	{
		Ui_.setupUi (this);

		LoadSettings ();

		const QList<QPair<QString, QStringList>> mathQuests
		{
			{ "(cos(x))'", { "-sin(x)" } },
			{ QString::fromUtf8 ("e^(iπ)"), { "-1" } }
		};
		PredefinedQuests_ << mathQuests;

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { IsDirty_ = true; },
			Ui_.Question_,
			SIGNAL (textChanged ()),
			this
		};
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { IsDirty_ = true; },
			Ui_.Answers_,
			SIGNAL (textChanged ()),
			this
		};
	}

	QString ConfWidget::GetQuestion () const
	{
		return Ui_.Question_->toPlainText ();
	}

	QStringList ConfWidget::GetAnswers () const
	{
		return Ui_.Answers_->toPlainText ().split ('\n', Qt::SkipEmptyParts);
	}

	void ConfWidget::SaveSettings () const
	{
		BSM_->setProperty ("Question", GetQuestion ());
		BSM_->setProperty ("Answers", GetAnswers ());

		IsDirty_ = false;
	}

	void ConfWidget::LoadSettings ()
	{
		const auto& question = BSM_->property ("Question").toString ();
		Ui_.Question_->setPlainText (question);

		const auto& answers = BSM_->property ("Answers").toStringList ();
		Ui_.Answers_->setPlainText (answers.join ("\n"));

		IsDirty_ = false;
	}

	void ConfWidget::accept ()
	{
		if (!IsDirty_)
			return;

		SaveSettings ();
		emit listsChanged ();
	}

	void ConfWidget::reject ()
	{
		LoadSettings ();
	}

	void ConfWidget::on_QuestStyle__currentIndexChanged (int idx)
	{
		Ui_.QuestVariant_->clear ();
		for (const auto& pair : PredefinedQuests_.value (idx - 1))
			Ui_.QuestVariant_->addItem (pair.first, pair.second);
	}

	void ConfWidget::on_QuestVariant__currentIndexChanged (int idx)
	{
		Ui_.Question_->setPlainText (Ui_.QuestVariant_->currentText ());
		Ui_.Answers_->setPlainText (Ui_.QuestVariant_->itemData (idx).toStringList ().join ("\n"));
	}
}
}
}
