/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QToolButton>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <util/util.h>

Q_DECLARE_METATYPE (QFileInfo);

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

	ChoroidTab::ChoroidTab (const TabClassInfo& tc, ICoreProxy_ptr proxy, QObject *parent)
	: TabClass_ (tc)
	, Parent_ (parent)
	, Proxy_ (proxy)
	, DeclView_ (new QDeclarativeView)
	, QMLFilesModel_ (new QMLItemModel)
	, FSModel_ (new QFileSystemModel (this))
	, FilesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		auto lay = new QVBoxLayout ();
		lay->setContentsMargins (0, 0, 0, 0);
		Ui_.ViewFrame_->setLayout (lay);
		lay->addWidget (DeclView_);

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

		Bar_ = new QToolBar;

		SetSortMenu ();
	}

	ChoroidTab::~ChoroidTab ()
	{
		delete Bar_;
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
		return Bar_;
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

	void ChoroidTab::SetSortMenu ()
	{
		auto sortGroup = new QActionGroup (this);

		SortMenu_ = new QMenu;
		auto byName = SortMenu_->addAction (tr ("By name"),
				this, SLOT (sortByName ()));
		byName->setCheckable (true);
		byName->setChecked (true);
		sortGroup->addAction (byName);

		auto byDate = SortMenu_->addAction (tr ("By date"),
				this, SLOT (sortByDate ()));
		byDate->setCheckable (true);
		sortGroup->addAction (byDate);

		auto bySize = SortMenu_->addAction (tr ("By size"),
				this, SLOT (sortBySize ()));
		bySize->setCheckable (true);
		sortGroup->addAction (bySize);

		auto byNumber = SortMenu_->addAction (tr ("By number"),
				this, SLOT (sortByNumber ()));
		byNumber->setCheckable (true);
		sortGroup->addAction (byNumber);

		sortByName ();

		auto sortModeButton = new QToolButton ();
		sortModeButton->setIcon (Proxy_->GetIcon ("view-sort-ascending"));
		sortModeButton->setText (tr ("Sort mode"));
		sortModeButton->setPopupMode (QToolButton::InstantPopup);
		sortModeButton->setMenu (SortMenu_);

		Bar_->addWidget (sortModeButton);
	}

	void ChoroidTab::ShowImage (const QString& path)
	{
		QMetaObject::invokeMethod (DeclView_->rootObject (),
				"showSingleImage",
				Q_ARG (QVariant, QUrl::fromLocalFile (path)));
	}

	void ChoroidTab::sortByName ()
	{
		CurrentSorter_ = [] (const QFileInfo& left, const QFileInfo& right)
		{
			return QString::localeAwareCompare (left.fileName (), right.fileName ()) < 0;
		};
		reload ();
	}

	void ChoroidTab::sortByDate ()
	{
		CurrentSorter_ = [] (const QFileInfo& left, const QFileInfo& right)
			{ return left.created () < right.created (); };
		reload ();
	}

	void ChoroidTab::sortBySize ()
	{
		CurrentSorter_ = [] (const QFileInfo& left, const QFileInfo& right)
			{ return left.size () < right.size (); };
		reload ();
	}

	void ChoroidTab::sortByNumber ()
	{
		CurrentSorter_ = [] (const QFileInfo& left, const QFileInfo& right) -> bool
		{
			auto toNumber = [] (const QString& string) -> int
			{
				QString result;
				std::copy_if (string.begin (), string.end (), std::back_inserter (result),
						[] (decltype (string.at (0)) c) { return c.isDigit (); });
				return result.toInt ();
			};
			return toNumber (left.fileName ()) < toNumber (right.fileName ());
		};
		reload ();
	}

	void ChoroidTab::reload ()
	{
		handleDirTreeCurrentChanged (Ui_.DirTree_->currentIndex ());
	}

	void ChoroidTab::handleDirTreeCurrentChanged (const QModelIndex& index)
	{
		const QString& path = FSModel_->filePath (index);
		Ui_.PathEdit_->setText (path);

		FilesModel_->clear ();
		QMLFilesModel_->clear ();

		FilesModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Size"), tr ("Last modified") });

		if (!index.isValid ())
			return;

		QList<QStandardItem*> qmlItems;

		const QStringList nf { "*.jpg", "*.png", "*.svg", "*.gif" };
		auto entries = QDir (path).entryInfoList (nf, QDir::Files, QDir::Name | QDir::IgnoreCase);
		std::sort (entries.begin (), entries.end (), CurrentSorter_);

		for (const auto& info : entries)
		{
			const QString& absPath = info.absoluteFilePath ();

			QList<QStandardItem*> row
			{
				new QStandardItem (info.fileName ()),
				new QStandardItem (Util::MakePrettySize (info.size ())),
				new QStandardItem (info.lastModified ().toString ())
			};

			row.first ()->setData (absPath, CRFilePath);

			for (auto item : row)
				item->setEditable (false);

			FilesModel_->appendRow (row);

			auto qmlItem = new QStandardItem (info.fileName ());
			qmlItem->setData (info.fileName (), ILRFilename);
			qmlItem->setData (Util::MakePrettySize (info.size ()), ILRFileSize);
			qmlItem->setData (QUrl::fromLocalFile (absPath), ILRImage);
			qmlItem->setData (QVariant::fromValue (info), ILRFileInfo);
			qmlItems << qmlItem;
		}

		QMLFilesModel_->invisibleRootItem ()->appendRows (qmlItems);
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
			for (const QDeclarativeError& error : DeclView_->errors ())
				qWarning () << error.toString ()
						<< "["
						<< error.description ()
						<< "]";
		}
	}
}
}
