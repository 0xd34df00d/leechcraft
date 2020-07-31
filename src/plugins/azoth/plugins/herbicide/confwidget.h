/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "ui_confwidget.h"

namespace LC
{
namespace Util
{
	class BaseSettingsManager;
}

namespace Azoth
{
namespace Herbicide
{
	class ConfWidget : public QWidget
	{
		Q_OBJECT
		
		Ui::ConfWidget Ui_;
		QList<QList<QPair<QString, QStringList>>> PredefinedQuests_;

		Util::BaseSettingsManager * const BSM_;

		mutable bool IsDirty_ = false;
	public:
		ConfWidget (Util::BaseSettingsManager*, QWidget* = nullptr);
		
		QString GetQuestion () const;
		QStringList GetAnswers () const;
	private:
		void SaveSettings () const;
		void LoadSettings ();
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_QuestStyle__currentIndexChanged (int);
		void on_QuestVariant__currentIndexChanged (int);
	signals:
		void listsChanged ();
	};
}
}
}
