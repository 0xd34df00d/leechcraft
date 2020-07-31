/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scripthandler.h"
#include <QTimer>
#include <QFileSystemWatcher>
#include <QtDebug>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iscriptloader.h>
#include "scriptobject.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	ScriptHandler::ScriptHandler (IPluginsManager *ipm, QObject *parent)
	: QObject { parent }
	, IPM_ { ipm }
	, DelayTimer_ { new QTimer { this } }
	, FileWatcher_ { new QFileSystemWatcher { this } }
	{
		connect (DelayTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (reevaluate ()));

		connect (FileWatcher_,
				SIGNAL (fileChanged (QString)),
				this,
				SLOT (reload ()));
	}

	void ScriptHandler::SetScriptPath (const QString& path)
	{
		if (path == Path_)
			return;

		if (!Path_.isEmpty ())
			FileWatcher_->removePath (Path_);

		Path_ = path;
		reload ();

		if (!Path_.isEmpty ())
			FileWatcher_->addPath (Path_);
	}

	QList<Effect_t> ScriptHandler::GetEffects () const
	{
		return Effects_;
	}

	void ScriptHandler::reload ()
	{
		CurrentScript_.reset ();
		Effects_.clear ();

		if (Path_.isEmpty ())
			return;

		const auto loaders = IPM_->GetAllCastableTo<IScriptLoader*> ();
		if (loaders.isEmpty ())
			return;

		const auto loader = loaders.at (0)->CreateScriptLoaderInstance ({});
		if (!loader)
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to create a script loader";
			return;
		}

		CurrentScript_ = loader->LoadScript ("/home/lctest/test.js");
		if (!CurrentScript_)
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to load script";
			return;
		}

		CurrentScript_->AddQObject (new ScriptObject, "Effects");

		reevaluate ();
	}

	void ScriptHandler::reevaluate ()
	{
		DelayTimer_->stop ();

		auto handleVarList = [&] (const QVariantList& list)
		{
			QList<Effect_t> newEffects;
			for (const auto& var : list)
				if (var.canConvert<Effect_t> ())
					newEffects << var.value<Effect_t> ();
				else
					qWarning () << Q_FUNC_INFO
							<< "variant is not an effect:"
							<< var;

			if (newEffects != Effects_)
			{
				using std::swap;
				swap (newEffects, Effects_);
				emit effectsListChanged ();
			}
		};

		const auto& scriptResult = CurrentScript_->InvokeMethod ("getEffects");

		switch (scriptResult.type ())
		{
		case QVariant::List:
			handleVarList (scriptResult.toList ());
			break;
		case QVariant::Map:
		{
			const auto& map = scriptResult.toMap ();
			handleVarList (map.value ("effects").toList ());

			if (map.contains ("delay"))
			{
				if (const auto delay = map ["delay"].toDouble ())
					DelayTimer_->start (delay * 1000);
				else
					qWarning () << Q_FUNC_INFO
							<< "non-positive delay:"
							<< map ["delay"];
			}

			break;
		}
		default:
			qWarning () << Q_FUNC_INFO
					<< "unexpected script result:"
					<< scriptResult;
			break;
		}
	}
}
}
}
