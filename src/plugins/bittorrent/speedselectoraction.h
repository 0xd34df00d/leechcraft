/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidgetAction>

class QComboBox;

namespace LC::BitTorrent
{
	class SpeedSelectorAction : public QWidgetAction
	{
		Q_OBJECT

		const QString Setting_;
		QList<QComboBox*> Boxes_;
	public:
		SpeedSelectorAction (QString, QObject*);

		int CurrentData ();
		void HandleSpeedsChanged ();
	protected:
		QWidget* createWidget (QWidget*) override;
		void deleteWidget (QWidget*) override;
	signals:
		void currentIndexChanged (int);
	};
}
