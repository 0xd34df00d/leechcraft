/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QHash>

class ITabWidget;

namespace LC
{
namespace KBSwitch
{
	class KeyboardLayoutSwitcher : public QObject
	{
		Q_OBJECT

		enum class SwitchingPolicy
		{
			Global,
			Plugin,
			Tab
		};

		SwitchingPolicy CurrentSwitchingPloicy_;

		QHash<ITabWidget*, int> Widget2KBLayoutIndex_;
		QHash<QByteArray, int> TabClass2KBLayoutIndex_;

		QWidget *LastCurrentWidget_ = nullptr;
	public:
		KeyboardLayoutSwitcher (QObject *parent = nullptr);

		bool IsGlobalPolicy () const;
	private:
		void UpdateSavedState (ITabWidget*, int);
		std::optional<int> GetSavedState (ITabWidget*) const;
	public slots:
		void updateKBLayouts (QWidget *current, QWidget *prev);
	private slots:
		void setSwitchingPolicy ();
		void handleRemoveWidget (QWidget *widget);
	};
}
}
