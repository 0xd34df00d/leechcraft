/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidgetAction>
#include <QComboBox>

namespace LC
{
namespace BitTorrent
{
	class SpeedSelectorAction : public QWidgetAction
	{
		Q_OBJECT

		QString Setting_;
	public:
		SpeedSelectorAction (const QString&, QObject*);

		int CurrentData ();
	protected:
		QWidget* createWidget (QWidget*) override;
		void deleteWidget (QWidget*) override;
	public slots:
		void handleSpeedsChanged ();
	private slots:
		void syncSpeeds (int);
	private:
		template<typename F>
		void Call (F&&);
	signals:
		void currentIndexChanged (int);
	};
}
}
