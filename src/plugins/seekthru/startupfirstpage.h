/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_startupfirstpage.h"

namespace LC::SeekThru
{
	struct EngineInfo
	{
		QString ResourceFileName_;
		QString DefaultTags_;
		QString Name_;

		EngineInfo (const QString&, const QString&, const QString&);
	};
	typedef QList<EngineInfo> EngineInfos_t;

	class StartupFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::SeekThruStartupFirstPageWidget Ui_;
		QMap<QString, EngineInfos_t> Sets_;
		enum
		{
			RoleSet = Qt::UserRole + 127,
			RoleFile
		};
	public:
		explicit StartupFirstPage (QWidget* = nullptr);

		void initializePage () override;
	private:
		void Populate (const QString&);
	private slots:
		void handleAccepted ();
		void handleCurrentIndexChanged (const QString&);
	};
}
