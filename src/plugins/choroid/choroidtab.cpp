/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "choroidtab.h"
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QDateTime>
#include <QVBoxLayout>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <util/util.h>

namespace LeechCraft
{
namespace Choroid
{
	class QMLItemModel : public QStandardItemModel
	{
	public:
		QMLItemModel (QObject *parent = 0)
		: QStandardItemModel (parent)
		{
		}

		using QStandardItemModel::setRoleNames;
	};

	ChoroidTab::ChoroidTab (const TabClassInfo& tc, QObject *parent)
	: TabClass_ (tc)
	, Parent_ (parent)
	, DeclView_ (new QDeclarativeView)
	, QMLFilesModel_ (new QMLItemModel)
	, FSModel_ (new QFileSystemModel (this))
	, FilesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.ViewFrame_->setLayout (new QVBoxLayout ());
		Ui_.ViewFrame_->layout ()->addWidget (DeclView_);

		connect (DeclView_,
				SIGNAL (statusChanged (QDeclarativeView::Status)),
				this,
				SLOT (handleStatusChanged (QDeclarativeView::Status)));

		LoadQML ();

		Ui_.VertSplitter_->setStretchFactor (1, 1);
		Ui_.VertSplitter_->setStretchFactor (1, 2);
		Ui_.LeftSplitter_->setStretchFactor (1, 1);
		Ui_.LeftSplitter_->setStretchFactor (1, 2);

		FSModel_->setRootPath (QDir::homePath ());
		FSModel_->setFilter (QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);
		FSModel_->sort (0);
		Ui_.DirTree_->setModel (FSModel_);
		Ui_.DirTree_->setCurrentIndex (FSModel_->index (QDir::homePath ()));

		connect (Ui_.DirTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDirTreeCurrentChanged (QModelIndex)));

		Ui_.FilesList_->setModel (FilesModel_);

		connect (Ui_.FilesList_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleFileChanged (QModelIndex)));
	}

	TabClassInfo ChoroidTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* ChoroidTab::ParentMultiTabs ()
	{
		return Parent_;
	}

	void ChoroidTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ChoroidTab::GetToolBar () const
	{
		return 0;
	}

	void ChoroidTab::LoadQML ()
	{
		DeclView_->setResizeMode (QDeclarativeView::SizeRootObjectToView);

		QHash<int, QByteArray> roles;
		roles [ILRFilename] = "filename";
		roles [ILRImage] = "image";
		roles [ILRFileSize] = "filesize";
		QMLFilesModel_->setRoleNames (roles);

		DeclView_->rootContext ()->setContextProperty ("filesListModel", QMLFilesModel_);

		QStringList candidates;
#ifdef Q_OS_WIN32
		candidates << QApplication::applicationDirPath () + "/share/qml/choroid/";
#else
		candidates << "/usr/local/share/leechcraft/qml/choroid/"
				<< "/usr/share/leechcraft/qml/choroid/";
#endif

		QString fileLocation;
		Q_FOREACH (const QString& cand, candidates)
			if (QFile::exists (cand + "ImgView.qml"))
			{
				fileLocation = cand + "ImgView.qml";
				break;
			}

		DeclView_->setSource (QUrl::fromLocalFile (fileLocation));

		QObject *item = DeclView_->rootObject ();
		connect (item,
				SIGNAL (imageSelected (QString)),
				this,
				SLOT (handleQMLImageSelected (QString)));
	}

	void ChoroidTab::ShowImage (const QString& path)
	{
		QMetaObject::invokeMethod (DeclView_->rootObject (),
				"showSingleImage",
				Q_ARG (QVariant, QUrl::fromLocalFile (path)));
	}

	void ChoroidTab::handleDirTreeCurrentChanged (const QModelIndex& index)
	{
		const QString& path = FSModel_->filePath (index);
		Ui_.PathEdit_->setText (path);

		FilesModel_->clear ();
		QMLFilesModel_->clear ();

		QStringList headers;
		headers << tr ("Name")
			<< tr ("Size")
			<< tr ("Last modified");
		FilesModel_->setHorizontalHeaderLabels (headers);

		QStringList nf;
		nf << "*.jpg"
			<< "*.png"
			<< "*.svg"
			<< "*.gif";

		Q_FOREACH (const QFileInfo& info,
				QDir (path).entryInfoList (nf, QDir::Files, QDir::Name))
		{
			const QString& absPath = info.absoluteFilePath ();

			QList<QStandardItem*> row;
			row << new QStandardItem (info.fileName ());
			row << new QStandardItem (Util::MakePrettySize (info.size ()));
			row << new QStandardItem (info.lastModified ().toString ());

			row.first ()->setData (absPath, CRFilePath);

			FilesModel_->appendRow (row);

			QStandardItem *qmlItem = new QStandardItem (info.fileName ());
			qmlItem->setData (info.fileName (), ILRFilename);
			qmlItem->setData (Util::MakePrettySize (info.size ()), ILRFileSize);
			qmlItem->setData (QUrl::fromLocalFile (absPath), ILRImage);
			QMLFilesModel_->appendRow (qmlItem);
		}
	}

	void ChoroidTab::handleFileChanged (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		const QString& path = index.sibling (index.row (), 0)
				.data (CRFilePath).toString ();
		ShowImage (path);
	}

	void ChoroidTab::handleQMLImageSelected (const QString& url)
	{
		ShowImage (QUrl (url).toLocalFile ());
	}

	void ChoroidTab::handleStatusChanged (QDeclarativeView::Status status)
	{
		if (status == QDeclarativeView::Error)
		{
			qWarning () << Q_FUNC_INFO
					<< "got errors:"
					<< DeclView_->errors ().size ();
			Q_FOREACH (const QDeclarativeError& error, DeclView_->errors ())
				qWarning () << error.toString ()
						<< "["
						<< error.description ()
						<< "]";
		}
	}
}
}
