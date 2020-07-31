/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QCoreApplication>
#include <QtDebug>

#ifdef Q_OS_WIN32
#include <vmime/platforms/windows/windowsHandler.hpp>
#else
#include <vmime/platforms/posix/posixHandler.hpp>
#endif

#include <util/sys/resourceloader.h>
#include <interfaces/core/ientitymanager.h>
#include "progressmanager.h"
#include "accountfoldermanager.h"
#include "composemessagetab.h"

namespace LC
{
namespace Snails
{
	Core::Core ()
	: ProgressManager_ { new ProgressManager { this } }
	, MsgView_ { new Util::ResourceLoader { "snails/msgview" } }
	{
#ifdef Q_OS_WIN32
		vmime::platform::setHandler<vmime::platforms::windows::windowsHandler> ();
#else
		vmime::platform::setHandler<vmime::platforms::posix::posixHandler> ();
#endif

		MsgView_->AddGlobalPrefix ();
		MsgView_->AddLocalPrefix ();

		qRegisterMetaType<size_t> ("size_t");
		qRegisterMetaType<AttDescr> ("LC::Snails::AttDescr");
		qRegisterMetaType<AttDescr> ("AttDescr");
		qRegisterMetaType<QList<QStringList>> ("QList<QStringList>");
		qRegisterMetaType<QList<QByteArray>> ("QList<QByteArray>");
		qRegisterMetaType<Folder> ("LC::Snails::Folder");
		qRegisterMetaType<QList<Folder>> ("QList<LC::Snails::Folder>");

		qRegisterMetaTypeStreamOperators<AttDescr> ();
		qRegisterMetaTypeStreamOperators<Folder> ();
		qRegisterMetaTypeStreamOperators<QList<Folder>> ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		MsgView_.reset ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const Entity& e)
	{
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	ProgressManager* Core::GetProgressManager () const
	{
		return ProgressManager_;
	}

	QString Core::GetMsgViewTemplate () const
	{
		auto dev = MsgView_->Load ("default/msgview.css");
		if (!dev || !dev->open (QIODevice::ReadOnly))
			return {};

		return QString::fromUtf8 (dev->readAll ());
	}
}
}
