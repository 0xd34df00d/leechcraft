/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sourceerrorhandler.h"
#include <QFileInfo>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "engine/sourceobject.h"

namespace LC
{
namespace LMP
{
	SourceErrorHandler::SourceErrorHandler (SourceObject *source, IEntityManager *iem)
	: QObject { source }
	, Source_ { source }
	, IEM_ { iem }
	{
		connect (Source_,
				SIGNAL (error (QString, SourceError)),
				this,
				SLOT (handleSourceError (QString, SourceError)));
	}

	void SourceErrorHandler::handleSourceError (const QString& sourceText, SourceError error)
	{
		QString text;

		const auto& curSource = Source_->GetCurrentSource ();
		const auto& curPath = curSource.ToUrl ().path ();
		const auto& filename = "<em>" + QFileInfo { curPath }.fileName () + "</em>";
		switch (error)
		{
		case SourceError::MissingPlugin:
			text = tr ("Cannot find a proper audio decoder for file %1. "
					"You probably don't have all the codec plugins installed.")
					.arg (filename);
			text += "<br/>" + sourceText;
			emit nextTrack ();
			break;
		case SourceError::SourceNotFound:
			text = tr ("Audio source %1 not found, playing next track...")
					.arg (filename);
			emit nextTrack ();
			break;
		case SourceError::CannotOpenSource:
			text = tr ("Cannot open source %1, playing next track...")
					.arg (filename);
			emit nextTrack ();
			break;
		case SourceError::InvalidSource:
			text = tr ("Audio source %1 is invalid, playing next track...")
					.arg (filename);
			emit nextTrack ();
			break;
		case SourceError::DeviceBusy:
			text = tr ("Cannot play %1 because the output device is busy.")
					.arg (filename);
			break;
		case SourceError::Other:
			text = sourceText;
			break;
		}

		IEM_->HandleEntity (Util::MakeNotification ("LMP", text, Priority::Critical));
	}
}
}
