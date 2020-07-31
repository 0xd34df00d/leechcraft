/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reportwizard.h"
#include <QNetworkAccessManager>
#include <QtDebug>
#include <QMessageBox>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include <util/xpc/downloadhelpers.h>
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
		auto final = new FinalPage (proxy);
		setPage (PageID::Final, final);
	}

	void ReportWizard::PostRequest (const QString& address,
			const QByteArray& data, const QByteArray& contentType,
			const std::function<void (QByteArray)>& dataHandler,
			const std::function<void (IDownload::Error)>& errorHandler)
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

		auto res = Util::DownloadAsTemporary (Proxy_->GetEntityManager (),
				QUrl { "https://dev.leechcraft.org" + address },
				Util::DownloadParams
				{
					.Additional_ = additional,
					.Context_ = this
				});
		if (!res)
		{
			errorHandler (IDownload::Error { IDownload::Error::Type::LocalError, "no plugins to handle" });
			return;
		}

		Util::Sequence (this, *res) >> Util::Visitor { dataHandler, errorHandler };
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
