/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wkfontswidget.h"
#include <QTimer>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <interfaces/iwkfontssettable.h>
#include "ui_wkfontswidget.h"
#include "massfontchangedialog.h"

namespace LC
{
namespace Util
{
	WkFontsWidget::WkFontsWidget (BaseSettingsManager *bsm, QWidget *parent)
	: QWidget { parent }
	, Ui_ { std::make_shared<Ui::WkFontsWidget> () }
	, BSM_ { bsm }
	{
		Ui_->setupUi (this);

		Family2Chooser_ [IWkFontsSettable::FontFamily::StandardFont] = Ui_->StandardChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::FixedFont] = Ui_->FixedChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::SerifFont] = Ui_->SerifChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::SansSerifFont] = Ui_->SansSerifChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::CursiveFont] = Ui_->CursiveChooser_;
		Family2Chooser_ [IWkFontsSettable::FontFamily::FantasyFont] = Ui_->FantasyChooser_;

		Family2Name_ [IWkFontsSettable::FontFamily::StandardFont] = "StandardFont";
		Family2Name_ [IWkFontsSettable::FontFamily::FixedFont] = "FixedFont";
		Family2Name_ [IWkFontsSettable::FontFamily::SerifFont] = "SerifFont";
		Family2Name_ [IWkFontsSettable::FontFamily::SansSerifFont] = "SansSerifFont";
		Family2Name_ [IWkFontsSettable::FontFamily::CursiveFont] = "CursiveFont";
		Family2Name_ [IWkFontsSettable::FontFamily::FantasyFont] = "FantasyFont";

		ResetFontChoosers ();

		for (const auto& pair : Util::Stlize (Family2Chooser_))
			connect (pair.second,
					&FontChooserWidget::fontChanged,
					[this, pair] { PendingFontChanges_ [pair.first] = pair.second->GetFont (); });

		Size2Spinbox_ [IWkFontsSettable::FontSize::DefaultFontSize] = Ui_->SizeDefault_;
		Size2Spinbox_ [IWkFontsSettable::FontSize::DefaultFixedFontSize] = Ui_->SizeFixedWidth_;
		Size2Spinbox_ [IWkFontsSettable::FontSize::MinimumFontSize] = Ui_->SizeMinimum_;

		Size2Name_ [IWkFontsSettable::FontSize::DefaultFontSize] = "FontSize";
		Size2Name_ [IWkFontsSettable::FontSize::DefaultFixedFontSize] = "FixedFontSize";
		Size2Name_ [IWkFontsSettable::FontSize::MinimumFontSize] = "MinimumFontSize";

		ResetSizeChoosers ();

		for (const auto& pair : Util::Stlize (Size2Spinbox_))
			connect (pair.second,
					qOverload<int> (&QSpinBox::valueChanged),
					[this, pair] { PendingSizeChanges_ [pair.first] = pair.second->value (); });
	}

	void WkFontsWidget::RegisterSettable (IWkFontsSettable *settable)
	{
		Settables_ << settable;
		connect (settable->GetQObject (),
				&QObject::destroyed,
				[this, settable] { Settables_.removeOne (settable); });

		for (const auto& pair : Util::Stlize (Family2Chooser_))
			settable->SetFontFamily (pair.first, pair.second->GetFont ());

		for (const auto& pair : Util::Stlize (Size2Spinbox_))
			settable->SetFontSize (pair.first, pair.second->value ());
	}

	void WkFontsWidget::SetSize (IWkFontsSettable::FontSize type, int size)
	{
		Size2Spinbox_ [type]->setValue (size);
		PendingSizeChanges_ [type] = size;

		QTimer::singleShot (1000, this, [this] { ApplyPendingSizeChanges (); });
	}

	void WkFontsWidget::ResetFontChoosers ()
	{
		for (const auto& pair : Util::Stlize (Family2Chooser_))
		{
			const auto& option = Family2Name_ [pair.first];
			pair.second->SetFont (BSM_->property (option.data ()).value<QFont> ());
		}
	}

	void WkFontsWidget::ResetSizeChoosers ()
	{
		for (const auto& pair : Util::Stlize (Size2Spinbox_))
		{
			const auto& option = Size2Name_ [pair.first];
			pair.second->setValue (BSM_->Property (option, 10).toInt ());
		}
	}

	void WkFontsWidget::ApplyPendingSizeChanges ()
	{
		for (const auto& pair : Util::Stlize (PendingSizeChanges_))
		{
			BSM_->setProperty (Size2Name_ [pair.first].data (), pair.second);
			emit sizeChanged (pair.first, pair.second);

			for (const auto settable : Settables_)
				settable->SetFontSize (pair.first, pair.second);
		}

		PendingSizeChanges_.clear ();
	}

	void WkFontsWidget::on_ChangeAll__released ()
	{
		QHash<QString, QList<IWkFontsSettable::FontFamily>> families;
		for (const auto& pair : Util::Stlize (Family2Chooser_))
			families [pair.second->GetFont ().family ()] << pair.first;

		const auto& stlized = Util::Stlize (families);
		const auto& maxPair = *std::max_element (stlized.begin (), stlized.end (),
				ComparingBy ([] (auto pair) { return pair.second.size (); }));

		const auto dialog = new MassFontChangeDialog { maxPair.first, maxPair.second, this };
		dialog->show ();
		connect (dialog,
				&QDialog::finished,
				[dialog, this] (int result)
				{
					if (result == QDialog::Rejected)
						return;

					const auto& font = dialog->GetFont ();
					for (const auto family : dialog->GetFamilies ())
					{
						PendingFontChanges_ [family] = font;
						Family2Chooser_ [family]->SetFont (font);
					}
				});
	}

	void WkFontsWidget::accept ()
	{
		ApplyPendingSizeChanges ();

		for (const auto& pair : Util::Stlize (PendingFontChanges_))
		{
			BSM_->setProperty (Family2Name_ [pair.first].data (), pair.second);
			emit fontChanged (pair.first, pair.second);

			for (const auto settable : Settables_)
				settable->SetFontFamily (pair.first, pair.second);
		}

		PendingFontChanges_.clear ();
	}

	void WkFontsWidget::reject ()
	{
		ResetFontChoosers ();
		ResetSizeChoosers ();

		PendingFontChanges_.clear ();
		PendingSizeChanges_.clear ();
	}
}
}
