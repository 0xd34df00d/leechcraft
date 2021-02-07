/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidgetAction>
#include <QCoreApplication>

class QComboBox;

namespace LC::BitTorrent
{
	class SessionSettingsManager;

	class SpeedSelectorAction : public QWidgetAction
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::SpeedSelectorAction)

		using Setter_t = void (SessionSettingsManager::*) (int);

		SessionSettingsManager * const SSM_;
		const Setter_t Setter_;

		const QString Setting_;
		QList<QComboBox*> Boxes_;
	public:
		SpeedSelectorAction (SessionSettingsManager*, Setter_t, const QString&, QObject*);

		void HandleSpeedsChanged ();
	protected:
		QWidget* createWidget (QWidget*) override;
		void deleteWidget (QWidget*) override;
	};
}
