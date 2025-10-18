/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "finalpage.h"
#include <QtDebug>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <util/sll/qtutil.h>
#include <util/sys/mimedetector.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/idownload.h>
#include "reportwizard.h"
#include "reporttypepage.h"
#include "bugreportpage.h"
#include "featurerequestpage.h"
#include "xmlgenerator.h"
#include "fileattachpage.h"

namespace LC
{
namespace Dolozhee
{
	FinalPage::FinalPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: QWizardPage (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);
		Ui_.UploadProgress_->hide ();
	}

	namespace
	{
		using UploadFileResult_t = Util::Either<QString, FileInfo>;

		Util::Task<UploadFileResult_t> UploadFile (FileInfo pending,
				ReportWizard *wiz, std::invocable<QString> auto progressReporter)
		{
			const auto& filename = QFileInfo { pending.Name_ }.fileName ();

			QFile file { pending.Name_ };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << "unable to open" << filename << file.errorString ();
				co_return Util::Left { FinalPage::tr ("Unable to open %1: %2.").arg (filename, file.errorString ()) };
			}

			progressReporter (FinalPage::tr ("Sending %1...").arg ("<em>" + filename + "</em>"));

			const auto reply = co_await wiz->PostRequest ("/uploads.xml",
					file.readAll (),
					"application/octet-stream");
			if (const auto err = reply.MaybeLeft ())
				co_return Util::Left { FinalPage::tr ("Unable to upload %1: %2.").arg (filename, err->Message_) };

			QDomDocument doc;
			if (!doc.setContent (reply.GetRight ()))
			{
				qWarning () << "unable to parse reply" << reply.GetRight ();
				co_return Util::Left { FinalPage::tr ("Unable to parse server reply.")};
			}

			pending.Token_ = doc.documentElement ().firstChildElement ("token").text ();
			co_return pending;
		}

		struct Error { QString Message_; };
		struct IssueId { QString Id_; };

		using IssueCreationResult = Util::Either<Error, IssueId>;

		Util::Task<IssueCreationResult> CreateIssue (QByteArray issueXml, ReportWizard *wiz)
		{
			const auto reply = co_await wiz->PostRequest ("/issues.xml",
					issueXml,
					"application/xml");
			const auto data = co_await WithHandler (reply,
					[] (const IDownload::Error& err) { return Error { err.Message_ }; });

			QDomDocument doc;
			if (!doc.setContent (data))
			{
				qWarning () << "cannot parse" << data;
				co_return Util::Left { Error { FinalPage::tr ("Unable to parse the server reply.") } };
			}

			const auto& id = doc.documentElement ().firstChildElement ("id").text ();
			if (id.isEmpty ())
			{
				qWarning () << "no issue ID" << data;
				co_return Util::Left { Error { FinalPage::tr ("The server hasn't returned the issue ID.") } };
			}

			co_return IssueId { id };
		}

		struct UploadResult
		{
			QStringList FileUploadErrors_ {};
			IssueCreationResult IssueCreationResult_;
		};

		Util::ContextTask<UploadResult> UploadPending (QList<FileInfo> pendingFiles,
				ReportWizard *wiz, std::invocable<QString> auto progressReporter)
		{
			co_await Util::AddContextObject { *wiz };

			QList<UploadFileResult_t> uploadResults;
			uploadResults.reserve (pendingFiles.size ());
			for (const auto& curUpload : pendingFiles)
				uploadResults << co_await UploadFile (curUpload, wiz, progressReporter);
			auto [uploadErrors, uploadedFiles] = Util::Partition (uploadResults);

			Util::MimeDetector detector;
			for (auto& item : uploadedFiles)
				item.Mime_ = detector (item.Name_);

			const auto typePage = wiz->GetReportTypePage ();
			const auto type = typePage->GetReportType ();
			const auto category = typePage->GetCategoryID ();

			QString title;
			QString desc;
			switch (type)
			{
			case ReportTypePage::Type::Bug:
				title = wiz->GetBugReportPage ()->GetTitle ();
				desc = wiz->GetBugReportPage ()->GetText ();
				break;
			case ReportTypePage::Type::Feature:
				title = wiz->GetFRPage ()->GetTitle ();
				desc = wiz->GetFRPage ()->GetText ();
				break;
			}

			const auto& data = XMLGenerator ().CreateIssue (title, desc,
					category, type, typePage->GetPriority (), uploadedFiles);
			const auto result = co_await CreateIssue (data, wiz);
			co_return { uploadErrors, result };
		}
	}

	void FinalPage::initializePage ()
	{
		RunUploading ();
	}

	namespace
	{
		QString MakeStatusMessage (const UploadResult& result)
		{
			return Util::Visit (result.IssueCreationResult_,
					[fileErrors = result.FileUploadErrors_] (const IssueId& id)
					{
						auto text = FinalPage::tr ("Report has been sent successfully. Thanks for your time!") +
								"<br />"_ql +
								(FinalPage::tr ("Your issue number is %1. You can view it here:") +
									" <a href='https://dev.leechcraft.org/issues/%1'>#%1</a>.<br/>"_ql +
									FinalPage::tr ("You can also track it via an Atom feed reader:") +
									" <a href='https://dev.leechcraft.org/issues/%1.atom'>Atom</a>."_ql).arg (id.Id_);
						if (!fileErrors.isEmpty ())
						{
							text += "<br /><br />"_ql;
							text += FinalPage::tr ("Although, sadly, the following files couldn't be uploaded:");
							text += "<ul><li>" + fileErrors.join ("</li><li>"_ql) + "</li></ul>";
						}
						return text;
					},
					[] (const Error& err)
					{
						return FinalPage::tr ("I'm very sorry to say that, but seems like "
								"we're unable to handle your report at the time :(") +
								"<br />" +
								err.Message_;
					});
		}
	}

	Util::ContextTask<void> FinalPage::RunUploading ()
	{
		co_await Util::AddContextObject { *this };

		auto wiz = static_cast<ReportWizard*> (wizard ());

		QList<FileInfo> files;
		for (const auto& file : wiz->GetFilePage ()->GetFiles ())
			files.push_back ({ file, QString (), QString (), QString () });

		const auto result = co_await UploadPending (files, wiz,
				[this] (const QString& text) { Ui_.Status_->setText (text); });
		Ui_.Status_->setText (MakeStatusMessage (result));
	}

	void FinalPage::on_Status__linkActivated (const QString& linkStr)
	{
		const auto& e = Util::MakeEntity (QUrl (linkStr),
					QString (),
					OnlyHandle | FromUserInitiated);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
