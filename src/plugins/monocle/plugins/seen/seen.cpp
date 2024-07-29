/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "seen.h"
#include <QIcon>
#include <util/util.h>
#include "document.h"
#include "docmanager.h"

namespace LC
{
namespace Monocle
{
namespace Seen
{
	namespace
	{
		void MsgCallback (ddjvu_context_t*, void *closure)
		{
			auto plugin = static_cast<Plugin*> (closure);
			QMetaObject::invokeMethod (plugin,
					"checkMessageQueue",
					Qt::QueuedConnection);
		}
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("monocle_seen");

		Context_ = ddjvu_context_create ("leechcraft");
		ddjvu_message_set_callback (Context_, MsgCallback, this);

		DocMgr_ = new DocManager (Context_, this);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Seen";
	}

	void Plugin::Release ()
	{
		ddjvu_context_release (Context_);
	}

	QString Plugin::GetName () const
	{
		return "Monocle Seen";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("DjVu backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	bool Plugin::CanLoadDocument (const QString& file)
	{
		return file.toLower ().endsWith (".djvu");
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		qDebug () << Q_FUNC_INFO << "requested opening" << file;
		auto doc = DocMgr_->LoadDocument (file);
		ddjvu_message_wait (Context_);
		return doc;
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "image/vnd.djvu" };
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("DjVu files"),
				{ "djvu", "djv" }
			}
		};
	}

	void Plugin::checkMessageQueue ()
	{
		while (const auto msg = ddjvu_message_peek (Context_))
		{
			switch (msg->m_any.tag)
			{
			case DDJVU_DOCINFO:
				DocMgr_->HandleDocInfo (msg->m_any.document);
				break;
			case DDJVU_PAGEINFO:
				DocMgr_->HandlePageInfo (msg->m_any.document, msg->m_any.page);
				break;
			case DDJVU_REDISPLAY:
				DocMgr_->RedrawPage (msg->m_any.document, msg->m_any.page);
				break;
			default:
				break;
			}

			ddjvu_message_pop (Context_);
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_seen, LC::Monocle::Seen::Plugin);
