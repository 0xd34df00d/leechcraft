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
#include "interfaces/ihavesettings.h"
#include "interfaces/iplugin2.h"
#include "interfaces/ipluginready.h"
#include "core.h"
#include "coreinstanceobject.h"
#include "flowlayout.h"
#include "plugins/eiskaltdcpp/eiskaltdcpp-qt/src/FlowLayout.h"

namespace LeechCraft
{
	SettingsTab::SettingsTab (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.ListContents_->setLayout (new QVBoxLayout);
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
					result [obj] << SettingsTab::tr ("LeechCraft");
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

		Q_FOREACH (const QString& key, keys)
			Ui_.ListContents_->layout ()->addWidget (group2box [key]);
			
		Q_FOREACH (QObject *obj, settables)
		{
			IHaveSettings *ihs = qobject_cast<IHaveSettings*> (obj);
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
				group2box [group]->layout ()->addWidget (butt);
			}
		}
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
		return 0;
	}
}
