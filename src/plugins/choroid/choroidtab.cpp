/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "choroidtab.h"
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QDateTime>
#include <QVBoxLayout>
#include <QGraphicsObject>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QFileInfo>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <util/models/rolenamesmixin.h>
#include <util/sys/paths.h>
#include <util/util.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC
{
namespace Choroid
{
	class QMLItemModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
	public:
		enum ImagesListRoles
		{
			ILRFilename = Qt::UserRole + 1,
			ILRImage,
			ILRFileSize
		};

		QMLItemModel (QObject *parent = nullptr)
		: Util::RoleNamesMixin<QStandardItemModel> (parent)
		{
			QHash<int, QByteArray> roles;
			roles [ILRFilename] = "filename";
			roles [ILRImage] = "image";
			roles [ILRFileSize] = "filesize";
			setRoleNames (roles);
		}
	};

	ChoroidTab::ChoroidTab (const TabClassInfo& tc, ICoreProxy_ptr proxy, QObject *parent)
	: TabClass_ (tc)
	, Parent_ (parent)
	, Proxy_ (proxy)
	, DeclView_ (new QQuickWidget)
	, QMLFilesModel_ (new QMLItemModel)
	, FSModel_ (new QFileSystemModel (this))
	, FilesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		auto lay = new QVBoxLayout ();
		lay->setContentsMargins (0, 0, 0, 0);
		Ui_.ViewFrame_->setLayout (lay);
		lay->addWidget (DeclView_);

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

		Bar_->addSeparator ();

		auto up = Bar_->addAction (tr ("Up"),
				this, SLOT (goUp ()));
		up->setProperty ("ActionIcon", "go-up");

		auto prev = Bar_->addAction (tr ("Previous"),
				this, SLOT (showNextImage ()));
		prev->setProperty ("ActionIcon", "go-previous");

		auto next = Bar_->addAction (tr ("Next"),
				this, SLOT (showNextImage ()));
		next->setProperty ("ActionIcon", "go-next");
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
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* ChoroidTab::GetToolBar () const
	{
		return Bar_;
	}

	void ChoroidTab::LoadQML ()
	{
		DeclView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		DeclView_->rootContext ()->setContextProperty ("filesListModel", QMLFilesModel_);

		DeclView_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "choroid", "ImgView.qml"));

		auto item = DeclView_->rootObject ();
		connect (item,
				SIGNAL (imageSelected (QString)),
				this,
				SLOT (handleQMLImageSelected (QString)));
		connect (item,
				SIGNAL (nextImageRequested ()),
				this,
				SLOT (showNextImage ()));
		connect (item,
				SIGNAL (prevImageRequested ()),
				this,
				SLOT (showPrevImage ()));
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
		sortModeButton->setIcon (Proxy_->GetIconThemeManager ()->
					GetIcon ("view-sort-ascending"));
		sortModeButton->setText (tr ("Sort mode"));
		sortModeButton->setPopupMode (QToolButton::InstantPopup);
		sortModeButton->setMenu (SortMenu_);

		Bar_->addWidget (sortModeButton);
	}

	void ChoroidTab::ShowImage (const QString& path)
	{
		ShowImage (QUrl::fromLocalFile (path));
	}

	void ChoroidTab::ShowImage (const QUrl& url)
	{
		QMetaObject::invokeMethod (DeclView_->rootObject (),
				"showSingleImage",
				Q_ARG (QVariant, url));
		CurrentImage_ = url;
	}

	QStandardItem* ChoroidTab::FindFileItem (const QString& filename)
	{
		for (int i = 0; i < QMLFilesModel_->rowCount (); ++i)
		{
			auto item = QMLFilesModel_->item (i);
			if (item->data (QMLItemModel::ILRFilename).toString () == filename)
				return item;
		}
		return 0;
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
						[] (const auto& c) { return c.isDigit (); });
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
			qmlItem->setData (info.fileName (), QMLItemModel::ILRFilename);
			qmlItem->setData (Util::MakePrettySize (info.size ()), QMLItemModel::ILRFileSize);
			qmlItem->setData (QUrl::fromLocalFile (absPath), QMLItemModel::ILRImage);
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
		ShowImage (QUrl (url));
	}

	void ChoroidTab::showNextImage ()
	{
		auto current = FindFileItem (QFileInfo (CurrentImage_.path ()).fileName ());
		if (!current)
			return;

		const auto rc = QMLFilesModel_->rowCount ();
		const auto& url = QMLFilesModel_->item ((current->row () + 1) % rc)->data (QMLItemModel::ILRImage).value<QUrl> ();
		ShowImage (url);
	}

	void ChoroidTab::showPrevImage ()
	{
		auto current = FindFileItem (QFileInfo (CurrentImage_.path ()).fileName ());
		if (!current)
			return;

		auto prev = current->row () - 1;
		if (prev < 0)
			prev = QMLFilesModel_->rowCount () - 1;
		const auto& url = QMLFilesModel_->item (prev)->data (QMLItemModel::ILRImage).value<QUrl> ();
		ShowImage (url);
	}

	void ChoroidTab::goUp ()
	{
		if (!CurrentImage_.isEmpty ())
			ShowImage (QUrl ());
		else
		{
			const auto& parentIdx = Ui_.DirTree_->currentIndex ().parent ();
			Ui_.DirTree_->setCurrentIndex (parentIdx);
		}
	}
}
}
