/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/idownload.h>

namespace LC
{
struct Entity;

namespace Dolozhee
{
	class ChooseUserPage;
	class ReportTypePage;
	class BugReportPage;
	class FeatureRequestPage;
	class FileAttachPage;
	class PreviewPage;

	class ReportWizard : public QWizard
	{
		ICoreProxy_ptr Proxy_;

		ChooseUserPage *ChooseUser_;
		ReportTypePage *ReportType_;
		BugReportPage *BugReportPage_;
		FeatureRequestPage *FRPage_;
		FileAttachPage *FilePage_;
		PreviewPage *PreviewPage_;
	public:
		enum PageID
		{
			ChooseUser,
			UserStatus,
			ReportType,
			BugDetails,
			FeatureDetails,
			FilePage,
			PreviewRequestPage,
			Final
		};

		explicit ReportWizard (ICoreProxy_ptr, QWidget* = nullptr);

		void PostRequest (const QString&, const QByteArray&, const QByteArray&,
				const std::function<void (QByteArray)>&,
				const std::function<void (IDownload::Error)>&);

		ChooseUserPage* GetChooseUserPage () const;
		ReportTypePage* GetReportTypePage () const;
		BugReportPage* GetBugReportPage () const;
		FeatureRequestPage* GetFRPage () const;
		FileAttachPage* GetFilePage () const;
	};
}
}
