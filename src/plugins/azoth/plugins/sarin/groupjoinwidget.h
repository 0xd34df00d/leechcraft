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
#include "types.h"

namespace LC::Azoth::Sarin
{
	class GroupJoinWidget
		: public QWidget
		, public IMUCJoinWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCJoinWidget)

		Ui::GroupJoinWidget Ui_;

		// Legacy conference identifying data (if it has been set).
		// Since there's nothing for the user to edit there, we just return it as is.
		std::optional<QVariantMap> ConfIdent_;
	public:
		explicit GroupJoinWidget ();

		void AccountSelected (QObject *account) override;
		void Join (QObject *account) override;
		void Cancel () override;

		QVariantMap GetIdentifyingData () const override;
		void SetIdentifyingData (const QVariantMap& data) override;

		static QVariantMap GetConfIdentifyingData (const QByteArray& cookie, ConfType type, uint32_t friendNum);
	private:
		bool IsJoiningGroup () const;
		void CheckValidity ();
	signals:
		void validityChanged (bool) override;
	};
}
