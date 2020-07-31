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
#include <util/xpc/util.h>
#include <util/sys/mimedetector.h>
#include <util/sll/functional.h>
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

	void FinalPage::initializePage ()
	{
		auto wiz = static_cast<ReportWizard*> (wizard ());

		for (const auto& file : wiz->GetFilePage ()->GetFiles ())
			PendingFiles_.push_back ({ file, QString (), QString (), QString () });

		UploadPending ();
	}

	void FinalPage::UploadPending ()
	{
		auto wiz = static_cast<ReportWizard*> (wizard ());

		if (!PendingFiles_.isEmpty ())
		{
			CurrentUpload_ = PendingFiles_.takeFirst ();
			QFile file (CurrentUpload_.Name_);
			if (!file.open (QIODevice::ReadOnly))
				return UploadPending ();

			const auto& filename = QFileInfo { CurrentUpload_.Name_ }.fileName ();

			Ui_.Status_->setText (tr ("Sending %1...").arg ("<em>" + filename + "</em>"));

			wiz->PostRequest ("/uploads.xml",
					file.readAll (),
					"application/octet-stream",
					Util::BindMemFn (&FinalPage::HandleUploadReplyData, this),
					[this, filename] (const IDownload::Error&)
					{
						QMessageBox::critical (this,
								"LeechCraft",
								tr ("Unable to upload %1.")
									.arg ("<em>" + filename + "</em>"));
					});

			return;
		}

		Util::MimeDetector detector;
		for (auto& item : UploadedFiles_)
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
				category, type, typePage->GetPriority (), UploadedFiles_);
		wiz->PostRequest ("/issues.xml",
				data,
				"application/xml",
				Util::BindMemFn (&FinalPage::HandleReportPostedData, this),
				[this] (const IDownload::Error&) { ShowRegrets (); });
	}

	void FinalPage::HandleUploadReplyData (const QByteArray& replyData)
	{
		QDomDocument doc;
		if (!doc.setContent (replyData))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply"
					<< replyData;
			UploadPending ();
			return;
		}

		CurrentUpload_.Token_ = doc
				.documentElement ()
				.firstChildElement ("token")
				.text ();
		UploadedFiles_ << CurrentUpload_;

		UploadPending ();
	}

	void FinalPage::HandleReportPostedData (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse"
					<< data;
			ShowRegrets ();
			return;
		}

		auto root = doc.documentElement ();
		const auto& id = root.firstChildElement ("id").text ();
		auto text = tr ("Report has been sent successfully. Thanks for your time!");
		if (!id.isEmpty ())
		{
			text += "<br />";
			text += (tr ("Your issue number is %1. You can view it here:") +
						" <a href='https://dev.leechcraft.org/issues/%1'>#%1</a>.<br/>" +
						tr ("You can also track it via an Atom feed reader:") +
						" <a href='https://dev.leechcraft.org/issues/%1.atom'>Atom</a>.").arg (id);
		}
		Ui_.Status_->setText (text);
	}

	void FinalPage::ShowRegrets()
	{
		const auto& text = tr ("I'm very sorry to say that, but seems like "
				"we're unable to handle your report at the time :(");
		Ui_.Status_->setText (text);
	}

	void FinalPage::handleUploadProgress (qint64 done)
	{
		Ui_.UploadProgress_->setValue (done);
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
