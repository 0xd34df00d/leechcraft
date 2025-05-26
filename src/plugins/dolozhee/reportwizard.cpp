/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reportwizard.h"
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "chooseuserpage.h"
#include "userstatuspage.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "fileattachpage.h"
#include "previewpage.h"
#include "finalpage.h"

namespace LC
{
namespace Dolozhee
{
	ReportWizard::ReportWizard (ICoreProxy_ptr proxy, QWidget *parent)
	: QWizard (parent)
	, Proxy_ (proxy)
	, ChooseUser_ (new ChooseUserPage (proxy))
	, ReportType_ (new ReportTypePage (proxy))
	, BugReportPage_ (new BugReportPage (proxy))
	, FRPage_ (new FeatureRequestPage)
	, FilePage_ (new FileAttachPage)
	, PreviewPage_ (new PreviewPage)
	{
		setWindowTitle (tr ("Issue reporter"));

		setPage (PageID::ChooseUser, ChooseUser_);
		setPage (PageID::UserStatus, new UserStatusPage ());
		setPage (PageID::ReportType, ReportType_);
		setPage (PageID::BugDetails, BugReportPage_);
		setPage (PageID::FeatureDetails, FRPage_);
		setPage (PageID::PreviewRequestPage, PreviewPage_);
		setPage (PageID::FilePage, FilePage_);
		setPage (PageID::Final, new FinalPage (proxy));
	}

	Util::Task<Util::TempDownload_t> ReportWizard::PostRequest (const QString& address,
			const QByteArray& data, const QByteArray& contentType)
	{
		const auto& user = ChooseUser_->GetLogin ().toUtf8 ();
		const auto& pass = ChooseUser_->GetPassword ().toUtf8 ();

		QVariantMap additional;
		additional ["HttpHeaders"] = QVariantMap {
					{ "Content-Type", contentType },
					{ "Authorization", "Basic " + (user + ':' + pass).toBase64 () }
				};
		additional ["UploadData"] = data;
		additional ["Operation"] = static_cast<int> (QNetworkAccessManager::PostOperation);

		return Util::DownloadAsTemporary (*Proxy_->GetEntityManager (),
				QUrl { "https://dev.leechcraft.org"_ql + address },
				Util::DownloadParams { .Additional_ = additional });
	}

	ChooseUserPage* ReportWizard::GetChooseUserPage () const
	{
		return ChooseUser_;
	}

	ReportTypePage* ReportWizard::GetReportTypePage () const
	{
		return ReportType_;
	}

	BugReportPage* ReportWizard::GetBugReportPage () const
	{
		return BugReportPage_;
	}

	FeatureRequestPage* ReportWizard::GetFRPage () const
	{
		return FRPage_;
	}

	FileAttachPage* ReportWizard::GetFilePage () const
	{
		return FilePage_;
	}
}
}
