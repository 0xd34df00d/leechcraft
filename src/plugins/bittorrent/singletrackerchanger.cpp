/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "singletrackerchanger.h"
#include <QValidator>
#include <QUrl>

namespace LC
{
namespace BitTorrent
{
	class URLValidator : public QValidator
	{
	public:
		URLValidator (QObject *parent)
		: QValidator (parent)
		{
		}

		State validate (QString& input, int&) const
		{
			QUrl url (input);
			return url.isValid () ? Acceptable : Intermediate;
		}
	};

	SingleTrackerChanger::SingleTrackerChanger (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Tracker_->setValidator (new URLValidator (this));
	}

	void SingleTrackerChanger::SetTracker (const QString& tracker)
	{
		Ui_.Tracker_->setText (tracker);
	}

	void SingleTrackerChanger::SetTier (int tier)
	{
		Ui_.Tier_->setValue (tier);
	}

	QString SingleTrackerChanger::GetTracker () const
	{
		return Ui_.Tracker_->text ();
	}

	int SingleTrackerChanger::GetTier () const
	{
		return Ui_.Tier_->value ();
	}
}
}
