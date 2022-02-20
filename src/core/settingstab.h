/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStack>
#include <QWidget>
#include "interfaces/ihavetabs.h"
#include "ui_settingstab.h"

class IHaveSettings;
class QLineEdit;
class QToolButton;

namespace LC
{
	class SettingsWidget;

	class SettingsTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::SettingsTab Ui_;
		QToolBar *Toolbar_;
		QAction *ActionBack_;
		QAction *ActionApply_;
		QAction *ActionCancel_;

		QString LastSearch_;
		QHash<QToolButton*, QObject*> Button2SettableRoot_;
		QHash<IHaveSettings*, QList<int>> Obj2SearchMatchingPages_;

		QStack<std::shared_ptr<SettingsWidget>> SettingsWidgets_;
	public:
		explicit SettingsTab (QWidget* = nullptr);

		void Initialize ();

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;
	private:
		void UpdateButtonsState ();
	public slots:
		void showSettingsFor (QObject*);
	private slots:
		void addSearchBox ();
		void handleSearch (const QString&);

		void handleBackRequested ();
	signals:
		void removeTab () override;
	};
}
