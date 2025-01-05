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
#include <util/qml/util.h>
#include <util/sll/prelude.h>
#include <util/sll/udls.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilocalcollection.h>
#include "checkmodel.h"
#include "checker.h"

namespace LC::LMP::BrainSlugz
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
			const TabClassInfo& tc,
			QObject* plugin)
	: CheckView_ { new QQuickWidget }
	, TC_ (tc)
	, Plugin_ { plugin }
	, Toolbar_ { new QToolBar { this } }
	, Model_ { new CheckModel { SortArtists (lmpProxy->GetLocalCollection ()->GetAllArtists ()), lmpProxy, this } }
	, CheckedModel_ { new MissingModel { Model_, this } }
	{
		Ui_.setupUi (this);
		Ui_.CheckViewWidget_->layout ()->addWidget (CheckView_);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			CheckView_->engine ()->addImportPath (cand);
		CheckView_->engine ()->addImageProvider (QStringLiteral ("ThemeIcons"),
				new Util::ThemeImageProvider { GetProxyHolder () });

		CheckView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		constexpr auto obj = [] (QObject *obj) { return QVariant::fromValue (obj); };
		CheckView_->rootContext ()->setContextProperties ({
					{ QStringLiteral ("colorProxy"), obj (new Util::ColorThemeProxy { GetProxyHolder ()->GetColorThemeManager (), this }) },
					{ QStringLiteral ("artistsModel"), obj (Model_) },
					{ QStringLiteral ("checkedModel"), obj (CheckedModel_) },
					{ QStringLiteral ("checkingState"), QString {} },
				});

		new Util::StandardNAMFactory
		{
			QStringLiteral ("lmp/qml"),
			[] { return 50_mib; },
			CheckView_->engine ()
		};

		Util::WatchQmlErrors (*CheckView_);

		const auto& filename = Util::GetSysPath (Util::SysPath::QML,
				QStringLiteral ("lmp/brainslugz"),
				QStringLiteral ("CheckView.qml"));
		CheckView_->setSource (QUrl::fromLocalFile (filename));

		SetupToolbar ();

		connect (Ui_.SelectAll_,
				&QPushButton::released,
				Model_,
				&CheckModel::SelectAll);
		connect (Ui_.SelectNone_,
				&QPushButton::released,
				Model_,
				&CheckModel::SelectNone);
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
		const auto startAction = Toolbar_->addAction (tr ("Start"), this, &CheckTab::Start);
		startAction->setProperty ("ActionIcon", "system-run");

		connect (this,
				&CheckTab::runningStateChanged,
				startAction,
				&QAction::setDisabled);
	}

	void CheckTab::Start ()
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

		const auto checker = new Checker { Model_, types, this };
		connect (checker,
				&Checker::finished,
				this,
				[this]
				{
					IsRunning_ = false;
					emit runningStateChanged (IsRunning_);
				});
		emit checkStarted (checker);

		CheckView_->rootContext ()->setContextProperty ("checkingState", QStringLiteral ("checking"));

		IsRunning_ = true;
		emit runningStateChanged (IsRunning_);
	}
}
