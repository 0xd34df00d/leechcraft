/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shx.h"
#include <QProcess>
#include <QtDebug>
#include <QTextDocument>
#include <QMessageBox>
#include <QIcon>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Azoth
{
namespace SHX
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Azoth_SHX">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_shx");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothshxsettings.xml");

		if (XmlSettingsManager::Instance ().property ("Command").toString ().isEmpty ())
		{
#ifdef Q_OS_WIN32
			const QString& cmd = "cmd.exe /U /S";
#else
			const QString& cmd = "/bin/sh -c";
#endif
			XmlSettingsManager::Instance ().setProperty ("Command", cmd);
		}

		ExecCommand_ = StaticCommand
		{
			{ "/exec" },
			[this] (ICLEntry *entry, const QString& text)
			{
				ExecuteProcess (entry, text.section (' ', 1));
				return true;
			},
			tr ("Executes the given command in a shell."),
			{}
		};
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.SHX";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth SHX";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to execute arbitrary shell commands and paste their result.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	StaticCommands_t Plugin::GetStaticCommands (ICLEntry*)
	{
		return { ExecCommand_ };
	}

	void Plugin::ExecuteProcess (ICLEntry *entry, const QString& text)
	{
		if (XmlSettingsManager::Instance ().property ("WarnAboutExecution").toBool ())
		{
			const auto& escaped = text.toHtmlEscaped ();
			const auto& msgText = tr ("Are you sure you want to execute this command?") +
					"<blockquote><em>" + escaped + "</em></blockquote>";
			if (QMessageBox::question (nullptr,
						"LeechCraft",
						msgText,
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				return;
		}

		auto proc = new QProcess ();
		Process2Entry_ [proc] = entry->GetQObject ();
		connect (proc,
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SLOT (handleFinished ()));

		const auto& commandParts = XmlSettingsManager::Instance ()
				.property ("Command").toString ().split (" ", Qt::SkipEmptyParts);
		const auto& command = commandParts.value (0);
		if (command.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty command";
			return;
		}
		proc->start (command, commandParts.mid (1) << text);
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	void Plugin::handleFinished ()
	{
		auto proc = qobject_cast<QProcess*> (sender ());
		proc->deleteLater ();

		const auto entryObj = Process2Entry_.take (proc);
		if (!entryObj)
		{
			qWarning () << Q_FUNC_INFO
					<< "no entry for process"
					<< proc;
			return;
		}
#ifdef Q_OS_WIN32
		auto out = QString::fromUtf16 (reinterpret_cast<const ushort*> (proc->readAllStandardOutput ().constData ()));
#else
		auto out = QString::fromUtf8 (proc->readAllStandardOutput ());
#endif
		const auto& err = proc->readAllStandardError ();

		if (!err.isEmpty ())
#ifdef Q_OS_WIN32
			out.prepend (tr ("Error: %1").arg (QString::fromUtf16 (reinterpret_cast<const ushort*> (err.constData ()))) + "\n");
#else
			out.prepend (tr ("Error: %1").arg (QString::fromUtf8 (err)) + "\n");
#endif

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		AzothProxy_->OpenChat (entry->GetEntryID (),
				entry->GetParentAccount ()->GetAccountID (),
				out);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_shx, LC::Azoth::SHX::Plugin);
