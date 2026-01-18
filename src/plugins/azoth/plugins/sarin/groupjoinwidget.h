/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/azoth/imucjoinwidget.h>
#include "ui_groupjoinwidget.h"

namespace LC::Azoth::Sarin
{
	class GroupJoinWidget
		: public QWidget
		, public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::GroupJoinWidget Ui_;
	public:
		explicit GroupJoinWidget ();

		void AccountSelected (QObject *account) override;
		void Join (QObject *account) override;
		void Cancel () override;
		QVariantMap GetIdentifyingData () const override;
		void SetIdentifyingData (const QVariantMap& data) override;
	private:
		void CheckValidity ();
	signals:
		void validityChanged (bool) override;
	};
}
