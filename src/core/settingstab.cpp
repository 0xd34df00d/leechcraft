/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingstab.h"
#include <algorithm>
#include <numeric>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QToolButton>
#include <QToolBar>
#include <QLineEdit>
#include "util/sll/containerconversions.h"
#include "util/sll/qtutil.h"
#include "util/gui/flowlayout.h"
#include "util/gui/clearlineeditaddon.h"
#include "xmlsettingsdialog/xmlsettingsdialog.h"
#include "xmlsettingsdialog/basesettingsmanager.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/iplugin2.h"
#include "interfaces/ipluginready.h"
#include "core.h"
#include "coreinstanceobject.h"
#include "coreproxy.h"
#include "settingswidget.h"

namespace LC
{
	const int ButtonWidth = 96;

	SettingsTab::SettingsTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Settings bar")))
	, ActionBack_ (new QAction (tr ("Back"), this))
	, ActionApply_ (new QAction (tr ("Apply"), this))
	, ActionCancel_ (new QAction (tr ("Cancel"), this))
	{
		Ui_.setupUi (this);
		Ui_.ListContents_->setLayout (new QVBoxLayout);

		ActionBack_->setProperty ("ActionIcon", "go-previous");
		connect (ActionBack_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleBackRequested ()));

		ActionApply_->setProperty ("ActionIcon", "dialog-ok");
		connect (ActionApply_,
				&QAction::triggered,
				this,
				[this]
				{
					for (const auto& widget : SettingsWidgets_)
						widget->Accept ();
				});

		ActionCancel_->setProperty ("ActionIcon", "dialog-cancel");
		connect (ActionCancel_,
				&QAction::triggered,
				this,
				[this]
				{
					for (const auto& widget : SettingsWidgets_)
						widget->Reject ();
				});

		QTimer::singleShot (1000,
				this,
				SLOT (addSearchBox ()));

		Toolbar_->addAction (ActionBack_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (ActionApply_);
		Toolbar_->addAction (ActionCancel_);

		UpdateButtonsState ();
	}

	namespace
	{
		bool IsProperPlugin2 (QObject *obj, const QObjectList& settables)
		{
			auto ip2 = qobject_cast<IPlugin2*> (obj);
			if (!ip2)
				return false;

			auto classes = ip2->GetPluginClasses ();
			classes.subtract (Core::Instance ().GetCoreInstanceObject ()->GetExpectedPluginClasses ());
			if (classes.isEmpty ())
				return false;

			for (const auto settable : settables)
				if (const auto ipr = qobject_cast<IPluginReady*> (settable))
					if (!ipr->GetExpectedPluginClasses ().intersect (classes).isEmpty ())
						return true;

			return false;
		}

		QString NameForGroup (const QString& origName, const QString& group)
		{
			auto origSplit = origName.split (' ');
			auto groupSplit = group.splitRef (' ');

			while (origSplit.value (0) == groupSplit.value (0) &&
					origSplit.size () > 1)
			{
				origSplit.removeFirst ();
				if (!groupSplit.isEmpty ())
					groupSplit.removeFirst ();
			}

			const auto& fm = QApplication::fontMetrics ();
			const int pad = 3;
			for (auto& str : origSplit)
				if (fm.horizontalAdvance (str) > ButtonWidth - 2 * pad)
					str = fm.elidedText (str, Qt::ElideRight, ButtonWidth - 2 * pad);

			return origSplit.join ("\n");
		}

		QMap<QObject*, QList<QPair<QString, QString>>> BuildGroups (const QObjectList& settables)
		{
			QMap<QObject*, QList<QPair<QString, QString>>> result;
			for (const auto obj : settables)
			{
				if (obj == Core::Instance ().GetCoreInstanceObject ())
					result [obj].push_back ({ "LeechCraft", {} });
				else if (!IsProperPlugin2 (obj, settables))
					result [obj].push_back ({ SettingsTab::tr ("General plugins"), {} });
			}

			return result;
		}
	}

	void SettingsTab::Initialize ()
	{
		const QObjectList& settables = Core::Instance ().GetPluginManager ()->GetAllCastableRoots<IHaveSettings*> ();

		const auto& obj2groups = BuildGroups (settables);
		QSet<QPair<QString, QString>> allGroups;
		for (const auto& list : obj2groups)
			allGroups += Util::AsSet (list);

		QMap<QString, QGroupBox*> group2box;
		for (const auto& pair : allGroups)
		{
			auto box = new QGroupBox (pair.first);
			box->setLayout (new Util::FlowLayout);
			group2box [pair.first] = box;
		}

		QStringList keys = group2box.keys ();
		if (keys.removeAll (tr ("General plugins")))
			keys.prepend (tr ("General plugins"));
		if (keys.removeAll ("LeechCraft"))
			keys.prepend ("LeechCraft");

		for (const auto& key : keys)
			Ui_.ListContents_->layout ()->addWidget (group2box [key]);

		QMap<QString, QList<QToolButton*>> group2buttons;
		for (auto obj : settables)
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			const QIcon& icon = ii->GetIcon ().isNull () ?
					QIcon ("lcicons:/resources/images/defaultpluginicon.svg") :
					ii->GetIcon ();
			for (const auto& pair : obj2groups [obj])
			{
				auto butt = new QToolButton;
				butt->setToolButtonStyle (Qt::ToolButtonTextUnderIcon);

				const QString& name = NameForGroup (ii->GetName (), pair.second);
				butt->setText (name);

				butt->setToolTip (ii->GetInfo ());
				butt->setIconSize (QSize (72, 72));
				butt->setIcon (icon);
				butt->setFixedWidth (ButtonWidth);
				butt->setAutoRaise (true);
				Button2SettableRoot_ [butt] = obj;

				connect (butt,
						&QToolButton::released,
						this,
						[this, obj] { showSettingsFor (obj); });
				group2box [pair.first]->layout ()->addWidget (butt);
				group2buttons [pair.first] << butt;
			}

			const auto ihs = qobject_cast<IHaveSettings*> (obj);
			const auto& dialog = ihs->GetSettingsDialog ();

			connect (dialog.get (),
					&Util::XmlSettingsDialog::showPageRequested,
					this,
					[this, obj]
					{
						Core::Instance ().GetCoreInstanceObject ()->TabOpenRequested (GetTabClassInfo ().TabClass_);
						showSettingsFor (obj);
					});
		}

		qobject_cast<QBoxLayout*> (Ui_.ListContents_->layout ())->addStretch ();

		for (const auto& pair : Util::Stlize (group2buttons))
		{
			const auto& buttons = pair.second;
			const auto height = std::accumulate (buttons.begin (), buttons.end (), 0,
					[] (int height, QToolButton *button) { return std::max (height, button->sizeHint ().height ()); });
			for (const auto button : buttons)
				button->setFixedHeight (height);
		}
	}

	TabClassInfo SettingsTab::GetStaticTabClassInfo ()
	{
		static TabClassInfo setInfo
		{
			"org.LeechCraft.SettingsPane",
			tr ("Settings"),
			tr ("LeechCraft-wide settings dashboard"),
			Core::Instance ().GetCoreInstanceObject ()->GetIcon (),
			0,
			TFSingle | TFOpenableByRequest
		};
		return setInfo;
	}

	TabClassInfo SettingsTab::GetTabClassInfo () const
	{
		return GetStaticTabClassInfo ();
	}

	QObject* SettingsTab::ParentMultiTabs ()
	{
		return Core::Instance ().GetCoreInstanceObject ();
	}

	void SettingsTab::Remove ()
	{
		emit removeTab ();
	}

	QToolBar* SettingsTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void SettingsTab::UpdateButtonsState ()
	{
		const auto enable = !SettingsWidgets_.isEmpty ();
		ActionBack_->setEnabled (enable);
		ActionApply_->setEnabled (enable);
		ActionCancel_->setEnabled (enable);
	}

	void SettingsTab::addSearchBox ()
	{
		auto widget = new QWidget ();
		auto lay = new QHBoxLayout;
		widget->setLayout (lay);

		auto box = new QLineEdit ();
		box->setPlaceholderText (tr ("Search..."));
		box->setMaximumWidth (200);
		box->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
		box->setText (LastSearch_);
		lay->addStretch ();
		lay->addWidget (box, 0, Qt::AlignRight);
		new Util::ClearLineEditAddon (CoreProxy::UnsafeWithoutDeps (), box);

		Toolbar_->addWidget (widget);

		connect (box,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handleSearch (QString)));
	}

	namespace
	{
		QObjectList FindSubplugins (QObject *obj)
		{
			QObjectList result;

			auto ihp = qobject_cast<IPluginReady*> (obj);
			if (!ihp)
				return result;

			const auto& expected = ihp->GetExpectedPluginClasses ();
			for (auto settableObj : Core::Instance ().GetPluginManager ()->GetAllCastableRoots<IHaveSettings*> ())
			{
				auto ip2 = qobject_cast<IPlugin2*> (settableObj);
				if (ip2 && expected.intersects (ip2->GetPluginClasses ()))
					result << settableObj;
			}
			return result;
		}

		QObjectList FindSubpluginsRec (QObject *obj)
		{
			QObjectList result;

			auto subs = FindSubplugins (obj);
			while (!subs.isEmpty ())
			{
				auto sub = subs.takeFirst ();
				subs = FindSubplugins (sub) + subs;
				result << sub;
			}

			return result;
		}
	}

	void SettingsTab::showSettingsFor (QObject *obj)
	{
		auto ihs = qobject_cast<IHaveSettings*> (obj);
		if (!ihs)
			return;

		const auto widget = std::make_shared<SettingsWidget> (obj,
				FindSubpluginsRec (obj),
				[this] { return Obj2SearchMatchingPages_; });
		SettingsWidgets_ << widget;
		Ui_.StackedWidget_->addWidget (widget.get ());
		Ui_.StackedWidget_->setCurrentIndex (Ui_.StackedWidget_->count () - 1);

		UpdateButtonsState ();
	}

	void SettingsTab::handleSearch (const QString& text)
	{
		if (text == LastSearch_)
			return;

		LastSearch_ = text;
		for (const auto& pair : Util::Stlize (Button2SettableRoot_))
		{
			auto toolButton = pair.first;
			auto rootObj = pair.second;

			bool foundMatching = false;
			auto objs = FindSubplugins (rootObj);
			objs.prepend (rootObj);
			for (auto obj : objs)
			{
				auto ihs = qobject_cast<IHaveSettings*> (obj);
				const auto& list = ihs->GetSettingsDialog ()->HighlightMatches (text);
				Obj2SearchMatchingPages_ [ihs] = list;

				if (!list.isEmpty ())
					foundMatching = true;
			}

			toolButton->setEnabled (foundMatching);
		}

		for (const auto& widget : SettingsWidgets_)
			widget->UpdateSearchHighlights ();
	}

	void SettingsTab::handleBackRequested ()
	{
		if (SettingsWidgets_.isEmpty ())
			return;

		const auto& widget = SettingsWidgets_.pop ();
		Ui_.StackedWidget_->removeWidget (widget.get ());

		UpdateButtonsState ();
	}
}
