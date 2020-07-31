/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QSet>
#include "interfaces/blogique/iaccount.h"
#include "ui_commentswidget.h"

class QStandardItem;
class QQuickWidget;

namespace LC
{
namespace Blogique
{
	class CommentsModel;
	class SortCommentsProxyModel;

	class CommentsWidget : public QWidget
	{
		Q_OBJECT

		Ui::CommentsWidget Ui_;
		QQuickWidget * const View_;
		CommentsModel * const CommentsModel_;
		SortCommentsProxyModel * const ProxyModel_;
		QHash<QStandardItem*, CommentEntry> Item2RecentComment_;
		QSet<CommentEntry> RecentComments_;
	public:
		struct CommentID
		{
			QByteArray AccountID_;
			qint64 CommentID_;

			CommentID ()
			: CommentID_ (-1)
			{}

			bool operator== (const CommentID& otherComment) const
			{
				return CommentID_ == otherComment.CommentID_ &&
				AccountID_ == otherComment.AccountID_;
			}
		};
		typedef QList<CommentID> CommentIDs_t;
	private:
		QSet<CommentID> ReadComments_;

	public:
		CommentsWidget (QWidget *parent = 0);

		QString GetName () const;
		CommentEntry GetRecentCommentFromIndex (const QModelIndex& index) const;
	private:
		void FillModel ();
		void AddItemsToModel (const QList<CommentEntry>& comments);
		CommentEntry GetComment (const QString& accountId, int commentId) const;

	private slots:
		void handleLinkActivated (const QString& url);
		void handleDeleteComment (const QString& accountId, int commentId);
		void handleMarkCommentAsRead (const QString& accountId, int commentId);
		void handleAddComment (const QString& accountId, int entryId, int commentId);
		void handleCommentsUpdated ();
	};

	QDataStream& operator>> (QDataStream& in, LC::Blogique::CommentsWidget::CommentID& comment);
	QDataStream& operator<< (QDataStream& out, const LC::Blogique::CommentsWidget::CommentID& comment);
	uint qHash (const LC::Blogique::CommentsWidget::CommentID& cid);
}
}

Q_DECLARE_METATYPE (LC::Blogique::CommentsWidget::CommentIDs_t)
