/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewsmanager.h"
#include <QAction>
#include <QtDebug>
#include "effectprocessor.h"
#include "xmlsettingsmanager.h"
#include "scripthandler.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	ViewsManager::ViewsManager (IPluginsManager *ipm, QObject *parent)
	: QObject { parent }
	, ScriptHandler_ { new ScriptHandler { ipm, this } }
	{
		XmlSettingsManager::Instance ().RegisterObject ({
					"NightModeThreshold",
					"ReduceLightnessFactor",
					"SingleEffect",
					"ColorTemperature"
				},
				this, "handleEffectsChanged");

		connect (ScriptHandler_,
				SIGNAL (effectsListChanged ()),
				this,
				SLOT (handleEffectsChanged ()));
	}

	void ViewsManager::AddView (QWidget *view)
	{
		const auto effect = new EffectProcessor { view };
		view->setGraphicsEffect (effect);

		View2Effect_ [view] = effect;

		connect (view,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleViewDestroyed (QObject*)));

		const auto enable = XmlSettingsManager::Instance ()
				.property ("EnableNightModeByDefault").toBool ();
		effect->setEnabled (enable);

		const auto enableAct = new QAction { tr ("Night mode"), view };
		view->addAction (enableAct);
		enableAct->setShortcut (QString { "Ctrl+Shift+I" });
		enableAct->setCheckable (true);
		enableAct->setChecked (enable);
		connect (enableAct,
				SIGNAL (toggled (bool)),
				effect,
				SLOT (setEnabled (bool)));
		View2EnableAction_ [view] = enableAct;

		effect->SetEffects (GetCurrentEffects ());
	}

	QAction* ViewsManager::GetEnableAction (QWidget *view) const
	{
		return View2EnableAction_.value (view);
	}

	QList<Effect_t> ViewsManager::GetCurrentEffects ()
	{
		const auto effectStr = XmlSettingsManager::Instance ()
				.property ("SingleEffect").toString ();
		if (effectStr == "Invert")
		{
			const auto threshold = XmlSettingsManager::Instance ()
					.property ("NightModeThreshold").toInt ();
			return { InvertEffect { threshold } };
		}
		else if (effectStr == "ReduceLightness")
		{
			const auto factor = XmlSettingsManager::Instance ()
					.property ("ReduceLightnessFactor").toDouble ();
			return { LightnessEffect { factor } };
		}
		else if (effectStr == "ColorTemp")
		{
			const auto temp = XmlSettingsManager::Instance ()
					.property ("ColorTemperature").toInt ();
			return { ColorTempEffect { temp } };
		}
		else if (effectStr == "Script")
		{
			const auto& scriptPath = XmlSettingsManager::Instance ()
					.property ("ScriptPath").toString ();
			ScriptHandler_->SetScriptPath (scriptPath);
			return ScriptHandler_->GetEffects ();
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown effect"
					<< effectStr;
			return {};
		}
	}

	void ViewsManager::handleViewDestroyed (QObject *view)
	{
		View2Effect_.remove (view);
		View2EnableAction_.remove (view);
	}

	void ViewsManager::handleEffectsChanged ()
	{
		const auto& effects = GetCurrentEffects ();
		for (const auto proc : View2Effect_)
			proc->SetEffects (effects);
	}
}
}
}
