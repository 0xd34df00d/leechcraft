/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "settingstab.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QToolButton>
#include "plugininterface/flowlayout.h"
#include "xmlsettingsdialog/xmlsettingsdialog.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/iplugin2.h"
#include "interfaces/ipluginready.h"
#include "core.h"
#include "coreinstanceobject.h"

namespace LeechCraft
{
	SettingsTab::SettingsTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Settings bar")))
	, ActionBack_ (new QAction (tr ("Back"), this))
	, ActionApply_ (new QAction (tr ("Apply"), this))
	, ActionCancel_ (new QAction (tr ("Cancel"), this))
	, CurrentIHS_ (0)
	{
		Ui_.setupUi (this);
		Ui_.ListContents_->setLayout (new QVBoxLayout);
		Ui_.DialogContents_->setLayout (new QVBoxLayout);
		
		ActionBack_->setProperty ("ActionIcon", "back");
		connect (ActionBack_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleBackRequested ()));
		
		ActionApply_->setProperty ("ActionIcon", "apply");
		connect (ActionApply_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleApply ()));
		
		ActionCancel_->setProperty ("ActionIcon", "cancel");
		connect (ActionCancel_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCancel ()));
	}
	
	namespace
	{
		QStringList GetFirstClassPlugins (IPlugin2 *ip2)
		{
			const QSet<QByteArray>& classes = ip2->GetPluginClasses ();
			const QObjectList& settables = Core::Instance ()
					.GetPluginManager ()->GetAllCastableRoots<IPluginReady*> ();
			QStringList result;
			Q_FOREACH (QObject *obj, settables)
			{
				IPluginReady *ipr = qobject_cast<IPluginReady*> (obj);
				if (ipr->GetExpectedPluginClasses ().intersect (classes).isEmpty ())
					continue;
				
				IInfo *ii = qobject_cast<IInfo*> (obj);
				result << SettingsTab::tr ("Plugins for %1")
						.arg (ii->GetName ());
			}
			if (result.isEmpty ())
				result << SettingsTab::tr ("General second-level plugins");
			return result;
		}
		
		QMap<QObject*, QStringList> BuildGroups (const QObjectList& settables)
		{
			QMap<QObject*, QStringList> result;
			Q_FOREACH (QObject *obj, settables)
			{
				IPlugin2 *ip2 = qobject_cast<IPlugin2*> (obj);
				if (obj == Core::Instance ().GetCoreInstanceObject ())
					result [obj] << "LeechCraft";
				else if (ip2)
					result [obj] = GetFirstClassPlugins (ip2);
				else
					result [obj] << SettingsTab::tr ("General plugins");
			}
			
			return result;
		}
	}
	
	void SettingsTab::Initialize ()
	{
		const QObjectList& settables = Core::Instance ()
				.GetPluginManager ()->GetAllCastableRoots<IHaveSettings*> ();

		const QMap<QObject*, QStringList>& obj2groups = BuildGroups (settables);
		QSet<QString> allGroups;
		Q_FOREACH (const QStringList& value, obj2groups.values ())
			allGroups += QSet<QString>::fromList (value);
		
		QMap<QString, QGroupBox*> group2box;
		Q_FOREACH (const QString& group, allGroups)
		{
			QGroupBox *box = new QGroupBox (group);
			box->setLayout (new Util::FlowLayout);
			group2box [group] = box;
		}
		
		QStringList keys = group2box.keys ();
		if (keys.contains (tr ("General plugins")))
		{
			keys.removeAll (tr ("General plugins"));
			keys.prepend (tr ("General plugins"));
		}
		
		if (keys.removeAll ("LeechCraft"))
			keys.prepend ("LeechCraft");

		Q_FOREACH (const QString& key, keys)
			Ui_.ListContents_->layout ()->addWidget (group2box [key]);
			
		Q_FOREACH (QObject *obj, settables)
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			const QIcon& icon = ii->GetIcon ().isNull () ?
					QIcon (":/resources/images/defaultpluginicon.svg") :
					ii->GetIcon ();
			Q_FOREACH (const QString& group, obj2groups [obj])
			{
				QToolButton *butt = new QToolButton;
				butt->setToolButtonStyle (Qt::ToolButtonTextUnderIcon);
				butt->setText (ii->GetName ());
				butt->setToolTip (ii->GetInfo ());
				butt->setIconSize (QSize (64, 64));
				butt->setIcon (icon);
				butt->setProperty ("SettableObject",
						QVariant::fromValue<QObject*> (obj));
				connect (butt,
						SIGNAL (released ()),
						this,
						SLOT (handleSettingsCalled ()));
				group2box [group]->layout ()->addWidget (butt);
			}
		}
		
		qobject_cast<QBoxLayout*> (Ui_.ListContents_->layout ())->addStretch ();
	}
	
	TabClassInfo SettingsTab::GetTabClassInfo () const
	{
		TabClassInfo setInfo =
		{
			"org.LeechCraft.SettingsPane",
			tr ("Settings"),
			tr ("LeechCraft-wide settings dashboard"),
			Core::Instance ().GetCoreInstanceObject ()->GetIcon (),
			0,
			static_cast<TabFeatures> (TFSingle | TFOpenableByRequest)
		};
		return setInfo;
	}
	
	QObject* SettingsTab::ParentMultiTabs ()
	{
		return Core::Instance ().GetCoreInstanceObject ();
	}
	
	void SettingsTab::Remove ()
	{
		emit remove (this);
	}
	
	QToolBar* SettingsTab::GetToolBar () const
	{
		return Toolbar_;
	}
	
	void SettingsTab::handleSettingsCalled ()
	{
		QObject *obj = sender ()->property ("SettableObject").value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty object"
					<< sender ();
			return;
		}
		
		CurrentIHS_ = obj;
		
		IInfo *ii = qobject_cast<IInfo*> (obj);
		Ui_.SectionName_->setText (tr ("Settings for %1")
				.arg (ii->GetName ()));
		
		IHaveSettings *ihs = qobject_cast<IHaveSettings*> (obj);
		Ui_.DialogContents_->layout ()->addWidget (ihs->GetSettingsDialog ().get ());
		ihs->GetSettingsDialog ()->show ();
		
		Q_FOREACH (const QString& page, ihs->GetSettingsDialog ()->GetPages ())
			Ui_.Cats_->addTopLevelItem (new QTreeWidgetItem (QStringList (page)));
		
		Ui_.StackedWidget_->setCurrentIndex (1);
		Toolbar_->addAction (ActionBack_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (ActionApply_);
		Toolbar_->addAction (ActionCancel_);
	}
	
	void SettingsTab::handleBackRequested ()
	{
		Toolbar_->clear ();
		Ui_.StackedWidget_->setCurrentIndex (0);

		Ui_.Cats_->clear ();
		if (Ui_.DialogContents_->layout ()->count ())
		{
			QLayoutItem *item = Ui_.DialogContents_->layout ()->takeAt (0);
			item->widget ()->hide ();
			delete item;
		}
		
		CurrentIHS_ = 0;
	}
	
	void SettingsTab::handleApply ()
	{
		qobject_cast<IHaveSettings*> (CurrentIHS_)->GetSettingsDialog ()->accept ();
	}
	
	void SettingsTab::handleCancel()
	{
		qobject_cast<IHaveSettings*> (CurrentIHS_)->GetSettingsDialog ()->reject ();
	}
	
	void SettingsTab::on_Cats__currentItemChanged (QTreeWidgetItem *current)
	{
		if (!CurrentIHS_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CurrentIHS_";
			return;
		}
		
		const int idx = Ui_.Cats_->indexOfTopLevelItem (current);
		qobject_cast<IHaveSettings*> (CurrentIHS_)->GetSettingsDialog ()->SetPage (idx);
	}
}
