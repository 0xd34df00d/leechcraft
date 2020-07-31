/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pastedialog.h"
#include "pasteservicefactory.h"

namespace LC::Azoth::Autopaste
{
	PasteDialog::PasteDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		for (const auto& info : PasteServiceFactory {}.GetInfos ())
		  Ui_.ServiceCombo_->addItem (info.Icon_, info.Name_);
	}

	PasteDialog::Choice PasteDialog::GetChoice () const
	{
		return Choice_;
	}

	PasteServiceFactory::Creator_f PasteDialog::GetCreator () const
	{
		return PasteServiceFactory ().GetInfos ().at (Ui_.ServiceCombo_->currentIndex ()).Creator_;
	}

	QString PasteDialog::GetCreatorName () const
	{
		return Ui_.ServiceCombo_->currentText ();
	}

	void PasteDialog::SetCreatorName (const QString& name)
	{
		const auto idx = Ui_.ServiceCombo_->findText (name);
		if (idx >= 0)
			Ui_.ServiceCombo_->setCurrentIndex (idx);
	}

	Highlight PasteDialog::GetHighlight () const
	{
		return static_cast<Highlight> (Ui_.HighlightCombo_->currentIndex ());
	}

	void PasteDialog::SetHighlight (Highlight highEnum)
	{
		auto high = static_cast<int> (highEnum);
		if (high >= 0 && high < Ui_.HighlightCombo_->count ())
			Ui_.HighlightCombo_->setCurrentIndex (high);
	}

	void PasteDialog::on_ButtonBox__clicked (QAbstractButton *button)
	{
		switch (Ui_.ButtonBox_->standardButton (button))
		{
		case QDialogButtonBox::Yes:
			Choice_ = Choice::Yes;
			break;
		case QDialogButtonBox::No:
			Choice_ = Choice::No;
			break;
		default:
			Choice_ = Choice::Cancel;
			break;
		}
	}
}
