/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "checktab.h"
#include <QStandardItemModel>
#include <QToolBar>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/standardnamfactory.h>
#include <util/qml/qmlerrorwatcher.h>
#include <util/sll/prelude.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilocalcollection.h>
#include "checkmodel.h"
#include "checker.h"

namespace LC
{
namespace LMP
{
namespace BrainSlugz
{
	namespace
	{
		class MissingModel final : public QSortFilterProxyModel
		{
		public:
			MissingModel (QAbstractItemModel *source, QObject *parent)
			: QSortFilterProxyModel { parent }
			{
				setSourceModel (source);
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex&) const override
			{
				const auto& idx = sourceModel ()->index (row, 0);
				return idx.data (CheckModel::MissingCount).toInt ();
			}
		};

		auto SortArtists (Collection::Artists_t artists)
		{
			std::sort (artists.begin (), artists.end (), Util::ComparingBy (&Collection::Artist::Name_));
			return artists;
		}
	}

	CheckTab::CheckTab (const ILMPProxy_ptr& lmpProxy,
			const ICoreProxy_ptr& coreProxy,
			const TabClassInfo& tc,
			QObject* plugin)
	: CheckView_ { new QQuickWidget }
	, CoreProxy_ { coreProxy }
	, TC_ (tc)
	, Plugin_ { plugin }
	, Toolbar_ { new QToolBar { this } }
	, Model_ { new CheckModel { SortArtists (lmpProxy->GetLocalCollection ()->GetAllArtists ()),
				coreProxy, lmpProxy, this } }
	, CheckedModel_ { new MissingModel { Model_, this } }
	{
		Ui_.setupUi (this);
		Ui_.CheckViewWidget_->layout ()->addWidget (CheckView_);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			CheckView_->engine ()->addImportPath (cand);
		CheckView_->engine ()->addImageProvider ("ThemeIcons",
				new Util::ThemeImageProvider { coreProxy });

		CheckView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		const auto root = CheckView_->rootContext ();
		root->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy { coreProxy->GetColorThemeManager (), this });
		root->setContextProperty ("artistsModel", Model_);
		root->setContextProperty ("checkedModel", CheckedModel_);
		root->setContextProperty ("checkingState", "");

		new Util::StandardNAMFactory
		{
			"lmp/qml",
			[] { return 50 * 1024 * 1024; },
			CheckView_->engine ()
		};

		Util::WatchQmlErrors (CheckView_);

		const auto& filename = Util::GetSysPath (Util::SysPath::QML, "lmp/brainslugz", "CheckView.qml");
		CheckView_->setSource (QUrl::fromLocalFile (filename));

		SetupToolbar ();
	}

	TabClassInfo CheckTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* CheckTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void CheckTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* CheckTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void CheckTab::SetupToolbar ()
	{
		const auto startAction = Toolbar_->addAction (tr ("Start"));
		startAction->setProperty ("ActionIcon", "system-run");
		connect (startAction,
				SIGNAL (triggered ()),
				this,
				SLOT (handleStart ()));

		connect (this,
				SIGNAL (runningStateChanged (bool)),
				startAction,
				SLOT (setDisabled (bool)));
	}

	void CheckTab::on_SelectAll__released ()
	{
		Model_->selectAll ();
	}

	void CheckTab::on_SelectNone__released ()
	{
		Model_->selectNone ();
	}

	void CheckTab::handleStart ()
	{
		QList<Media::ReleaseInfo::Type> types;

		auto check = [&types] (QCheckBox *box, Media::ReleaseInfo::Type type)
		{
			if (box->checkState () == Qt::Checked)
				types << type;
		};
		check (Ui_.Album_, Media::ReleaseInfo::Type::Standard);
		check (Ui_.EP_, Media::ReleaseInfo::Type::EP);
		check (Ui_.Single_, Media::ReleaseInfo::Type::Single);
		check (Ui_.Compilation_, Media::ReleaseInfo::Type::Compilation);
		check (Ui_.Live_, Media::ReleaseInfo::Type::Live);
		check (Ui_.Soundtrack_, Media::ReleaseInfo::Type::Soundtrack);
		check (Ui_.Other_, Media::ReleaseInfo::Type::Other);

		Model_->RemoveUnscheduled ();

		const auto checker = new Checker { Model_, types, CoreProxy_, this };
		connect (checker,
				SIGNAL (finished ()),
				this,
				SLOT (handleCheckFinished ()));
		emit checkStarted (checker);

		CheckView_->rootContext ()->setContextProperty ("checkingState", "checking");

		IsRunning_ = true;
		emit runningStateChanged (IsRunning_);
	}

	void CheckTab::handleCheckFinished ()
	{
		IsRunning_ = false;
		emit runningStateChanged (IsRunning_);
	}
}
}
}
