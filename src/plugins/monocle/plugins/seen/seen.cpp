/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "seen.h"
#include <QIcon>
#include "document.h"
#include "docmanager.h"

namespace LeechCraft
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

	void Plugin::checkMessageQueue ()
	{
		while (const ddjvu_message_t *msg = ddjvu_message_peek (Context_))
		{
			qDebug () << Q_FUNC_INFO << msg->m_any.tag;

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

LC_EXPORT_PLUGIN (leechcraft_monocle_seen, LeechCraft::Monocle::Seen::Plugin);
