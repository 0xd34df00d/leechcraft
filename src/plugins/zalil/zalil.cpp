/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "zalil.h"
#include <QIcon>
#include "servicesmanager.h"
#include "pendinguploadbase.h"

namespace LC
{
namespace Zalil
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Manager_ = std::make_shared<ServicesManager> (proxy);
		connect (Manager_.get (),
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SIGNAL (fileUploaded (QString, QUrl)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Zalil";
	}

	void Plugin::Release ()
	{
		Manager_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Zalil";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Uploads files to filebin services without registration, like dump.bitcheese.net.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QStringList Plugin::GetServiceVariants () const
	{
		return Manager_->GetNames ({});
	}

	void Plugin::UploadFile (const QString& filename, const QString& service)
	{
		Manager_->Upload (filename, service, Progress_);
	}

	IJobHolderRepresentationHandler_ptr Plugin::CreateRepresentationHandler ()
	{
		return Progress_.CreateDefaultHandler ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_zalil, LC::Zalil::Plugin);
