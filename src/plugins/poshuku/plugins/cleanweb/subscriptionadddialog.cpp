/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "subscriptionadddialog.h"
#include <QStandardItemModel>
#include <QDomDocument>
#include <QDir>
#include <QUrl>
#include <util/util.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	SubscriptionAddDialog::SubscriptionAddDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		QDir subscrListFileDir;
		try
		{
			subscrListFileDir = Util::CreateIfNotExists ("data/poshuku/cleanweb/");
		}
		catch (const std::exception& e)
		{
			// that's ok, the directory just doesn't
			// exist, skip it at all
			qDebug () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		if (!subscrListFileDir.exists ("subscriptionslist.xml"))
			return;

		QFile file (subscrListFileDir.filePath ("subscriptionslist.xml"));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "could not open file"
					<< file.fileName ()
					<< "for reading:"
					<< file.errorString ();
			return;
		}
		QByteArray fileData = file.readAll ();
		QString errMsg;
		int errLine = 0;
		int errColumn = 0;
		QDomDocument doc;
		if (!doc.setContent (fileData, &errMsg, &errLine, &errColumn))
		{
			qWarning () << Q_FUNC_INFO
					<< "could not parse document at err:line "
					<< errLine
					<< errColumn
					<< "with"
					<< errMsg
					<< "; original contents follow:"
					<< fileData;
			return;
		}

		Ui_.PredefinedSubscriptions_->setEnabled (true);

		QStandardItemModel *model = new QStandardItemModel (this);
		QStringList labels;
		labels << tr ("Name")
				<< tr ("Purpose")
				<< tr ("URL");
		model->setHorizontalHeaderLabels (labels);

		QDomElement docElem = doc.documentElement ();
		Iterate (doc.documentElement (), model->invisibleRootItem ());

		Ui_.PredefinedSubscriptions_->setModel (model);

		Ui_.PredefinedSubscriptions_->expandAll ();
	}

	QString SubscriptionAddDialog::GetURL() const
	{
		return Ui_.URLEdit_->text ();
	}

	QString SubscriptionAddDialog::GetName () const
	{
		return Ui_.TitleEdit_->text ();
	}

	QList<QUrl> SubscriptionAddDialog::GetAdditionalSubscriptions() const
	{
		QList<QUrl> result;
		for (const auto item : Items_)
			if (item->checkState () == Qt::Checked)
				result << QUrl::fromEncoded (item->data ().toString ().toUtf8 ());
		return result;
	}

	void SubscriptionAddDialog::Iterate (const QDomElement& subParent,
				QStandardItem *parent)
	{
		QDomElement subscr = subParent.firstChildElement ("subscription");
		while (!subscr.isNull ())
		{
			QString url = subscr.attribute ("url");
			QString name = subscr.attribute ("name");
			QString purpose = subscr.attribute ("purpose");

			QStandardItem *nameItem = new QStandardItem (name);
			nameItem->setCheckable (true);
			nameItem->setCheckState (Qt::Unchecked);
			nameItem->setData (url);
			Items_ << nameItem;

			QList<QStandardItem*> items;
			items << nameItem;
			items << new QStandardItem (purpose);
			items << new QStandardItem (url);
			parent->appendRow (items);

			Iterate (subscr, nameItem);

			subscr = subscr.nextSiblingElement ("subscription");
		}
	}
}
}
}
