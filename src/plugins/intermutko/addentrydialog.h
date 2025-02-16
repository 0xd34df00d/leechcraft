/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ui_addentrydialog.h"
#include <util/sys/loggingfilter.h>
#include "localeentry.h"

namespace LC::Intermutko
{
	struct LocaleEntry;

	class AddEntryDialog : public QDialog
	{
		Ui::AddEntryDialog Ui_;
		Util::LoggingFilter OpenTypeMissingFilter_;
	public:
		explicit AddEntryDialog (QWidget *parent = nullptr);

		struct Entries
		{
			LocaleEntry Specific_;
			std::optional<LocaleEntry> AnyCountry_ {};
		};

		Entries GetEntries () const;
	private:
		void SetAcceptEnabled (bool);
	};
}
