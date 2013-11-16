/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 *  Boost Software License - Version 1.0 - August 17th, 2003
 *
 *  Permission is hereby granted, free of charge, to any person or organization
 *  obtaining a copy of the software and accompanying documentation covered by
 *  this license (the "Software") to use, reproduce, display, distribute,
 *  execute, and transmit the Software, and to prepare derivative works of the
 *  Software, and to permit third-parties to whom the Software is furnished to
 *  do so, all subject to the following:
 *
 *  The copyright notices in the Software and this entire statement, including
 *  the above license grant, this restriction and the following disclaimer,
 *  must be included in all copies of the Software, in whole or in part, and
 *  all derivative works of the Software, unless such copies or derivative
 *  works are solely in the form of machine-executable object code generated by
 *  a source language processor.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 *  SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 *  FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 **********************************************************************/

#include "commentswidget.h"
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QMessageBox>
#include <boost/concept_check.hpp>
#include <interfaces/core/ientitymanager.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/tooltipitem.h>
#include <util/qml/themeimageprovider.h>
#include <util/sys/paths.h>
#include <util/util.h>
#include "commentsmanager.h"
#include "commentsmodel.h"
#include "core.h"
#include "sortcommentsproxymodel.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	CommentsWidget::CommentsWidget (QWidget *parent)
	: QWidget (parent)
	, CommentsModel_ (new CommentsModel (this))
	, ProxyModel_ (new SortCommentsProxyModel (this, this))
	{
		Ui_.setupUi (this);

		ProxyModel_->setSourceModel (CommentsModel_);

		Ui_.CommentsView_->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		auto context = Ui_.CommentsView_->rootContext ();
		context->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ()
						.GetCoreProxy ()->GetColorThemeManager (), this));
		context->setContextProperty ("commentsModel", ProxyModel_);
		context->setContextProperty ("parentWidget", this);
		auto engine = Ui_.CommentsView_->engine ();
		engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (Core::Instance ().GetCoreProxy ()));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine->addImportPath (cand);
		Ui_.CommentsView_->setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML,
				"blogique", "commentsview.qml")));
		connect (Ui_.CommentsView_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLinkActivated (QString)));
		connect (Ui_.CommentsView_->rootObject (),
				SIGNAL (deleteComment (QString, int)),
				this,
				SLOT (handleDeleteComment (QString, int)));
		connect (Ui_.CommentsView_->rootObject (),
				SIGNAL (markCommentAsRead (QString, int)),
				this,
				SLOT (handleMarkCommentAsRead (QString, int)));
		ProxyModel_->sort (0, Qt::AscendingOrder);

		ReadComments_ = XmlSettingsManager::Instance ().property ("ReadComments")
				.value<CommentIDs_t> ().toSet ();

		FillModel ();

		connect (Core::Instance ().GetCommentsManager (),
				SIGNAL (gotNewComments (QList<CommentEntry>)),
				this,
				SLOT (handleGotNewComments (QList<CommentEntry>)));
	}

	QString CommentsWidget::GetName () const
	{
		return tr ("Comments");
	}

	CommentEntry CommentsWidget::GetRecentCommentFromIndex (const QModelIndex& index) const
	{
		return Item2RecentComment_.value (CommentsModel_->itemFromIndex (index), CommentEntry ());
	}

	void CommentsWidget::FillModel ()
	{
		AddItemsToModel (Core::Instance ().GetCommentsManager ()->GetComments ());
	}

	void CommentsWidget::AddItemsToModel (const QList<CommentEntry>& comments)
	{
		for (const auto& comment : comments)
		{
			if (RecentComments_.contains (comment))
				continue;

			CommentID cid;
			cid.AccountID_ = comment.AccountID_;
			cid.CommentID_ = comment.CommentID_;

			if (ReadComments_.contains (cid))
				continue;

			QStandardItem *item = new QStandardItem;
			item->setData (comment.AccountID_, CommentsModel::AccountID);
			item->setData (comment.EntrySubject_, CommentsModel::EntrySubject);
			item->setData (comment.EntryUrl_, CommentsModel::EntryUrl);
			item->setData (comment.EntryID_, CommentsModel::EntryID);
			item->setData (comment.CommentSubject_, CommentsModel::CommentSubject);
			item->setData (comment.CommentText_, CommentsModel::CommentBody);
			item->setData (comment.CommentAuthor_, CommentsModel::CommentAuthor);
			item->setData (comment.CommentDateTime_.toString (Qt::DefaultLocaleShortDate),
						   CommentsModel::CommentDate);
			item->setData (comment.CommentUrl_, CommentsModel::CommentUrl);
			item->setData (comment.CommentID_, CommentsModel::CommentID);

			Item2RecentComment_ [item] = comment;
			RecentComments_ << comment;

			CommentsModel_->appendRow (item);
		}
	}

	CommentEntry CommentsWidget::GetComment (const QString& accountId, int commentId) const
	{
		for (const auto& comment : RecentComments_)
			if (comment.AccountID_ == accountId.toUtf8 () &&
					comment.CommentID_ == commentId)
				return comment;
		return CommentEntry ();
	}

	void CommentsWidget::handleLinkActivated (const QString& url)
	{
		Core::Instance ().GetCoreProxy ()->GetEntityManager ()->
				HandleEntity (Util::MakeEntity (url,
						QString (),
						OnlyHandle | FromUserInitiated));
	}

	void CommentsWidget::handleDeleteComment (const QString& accountId, int commentId)
	{
		auto comment = GetComment (accountId, commentId);
		if (!comment.isValid ())
			return;

		if (auto account = Core::Instance ().GetAccountFromID (comment.AccountID_))
		{
			auto res = QMessageBox::question (this, "LeechCraft",
					tr ("Do you want to delete whole comment thread too?"),
					QMessageBox::Yes | QMessageBox::No);
			bool deleteThread = false;
			if (res == QMessageBox::Yes)
				deleteThread = true;

			account->DeleteComment (commentId, deleteThread);
		}
	}

	void CommentsWidget::handleMarkCommentAsRead (const QString& accountId, int commentId)
	{
		auto comment = GetComment (accountId, commentId);
		if (!comment.isValid ())
			return;

		CommentID cid;
		cid.AccountID_ = accountId.toUtf8 ();
		cid.CommentID_ = commentId;
		ReadComments_.insert (cid);

		XmlSettingsManager::Instance ().setProperty ("ReadComments",
				QVariant::fromValue<CommentIDs_t> (ReadComments_.toList ()));

		CommentEntry ce;
		ce.AccountID_ = comment.AccountID_;
		ce.EntryID_ =  comment.EntryID_;
		ce.CommentID_ = comment.CommentID_;
		if (auto item = Item2RecentComment_.key (ce))
			CommentsModel_->removeRow (item->index ().row ());

	}

	void CommentsWidget::handleGotNewComments (const QList<CommentEntry>& comments)
	{
		AddItemsToModel (comments);
	}

	void CommentsWidget::setItemCursor (QGraphicsObject *object, const QString& shape)
	{
		Q_ASSERT (object);

		Qt::CursorShape cursor = (shape == "PointingHandCursor") ?
			Qt::PointingHandCursor :
			Qt::ArrowCursor;

		object->setCursor (QCursor (cursor));
	}

	QDataStream& operator<< (QDataStream& out, const LeechCraft::Blogique::CommentID& comment)
	{
		out << static_cast<qint8> (1)
				<< comment.AccountID_
				<< comment.CommentID_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, LeechCraft::Blogique::CommentID& comment)
	{
		qint8 version = 0;
		in >> version;
		if (version > 0)
			in >> comment.AccountID_
					>> comment.CommentID_;
		return in;
	}

	uint  qHash (const CommentID& cid)
	{
		return qHash (cid.AccountID_) + ::qHash (cid.CommentID_);
	}

}
}
