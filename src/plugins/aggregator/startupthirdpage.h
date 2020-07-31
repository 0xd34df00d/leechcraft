/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include "ui_startupthirdpage.h"

namespace LC
{
namespace Aggregator
{
	class StartupThirdPage : public QWizardPage
	{
		Q_OBJECT

		Ui::StartupThirdPageWidget Ui_;

		struct FeedInfo
		{
			QString Name_;
			QStringList DefaultTags_;
			QString URL_;
		};

		typedef QList<FeedInfo> FeedInfos_t;
		QMap<QString, FeedInfos_t> Sets_;
	public:
		struct SelectedFeed
		{
			QString URL_;
			QString Tags_;
		};

		explicit StartupThirdPage (QWidget* = nullptr);

		void initializePage () override;
	private:
		void ParseFeedsSets ();
		void Populate (const QString&);
		void HandleAccepted ();
		void HandleCurrentIndexChanged (const QString&);
	private slots:
		void on_SelectAll__released ();
		void on_DeselectAll__released ();
	signals:
		void feedsSelected (const QList<SelectedFeed>&);
		void reinitStorageRequested ();
	};
}
}
