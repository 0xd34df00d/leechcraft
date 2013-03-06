/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "shx.h"
#include <QProcess>
#include <QtDebug>
#include <QTextDocument>
#include <QMessageBox>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace SHX
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
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
		static QIcon icon (":/azoth/shx/resources/images/shx.svg");
		return icon;
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

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
			QObject *chatTab, QObject*, int, QString)
	{
		QString text = proxy->GetValue ("text").toString ();

		const QString marker = XmlSettingsManager::Instance ()
				.property ("Marker").toString ();
		if (!text.startsWith (marker))
			return;

		text = text.mid (marker.size ());
		if (XmlSettingsManager::Instance ().property ("WarnAboutExecution").toBool ())
		{
			const auto& msgText = tr ("Are you sure you want to execute this command?") +
					"<blockquote><em>" + Qt::escape (text) + "</em></blockquote>";
			if (QMessageBox::question (0, "LeechCraft",
						msgText,
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				return;
		}

		proxy->CancelDefault ();

		auto proc = new QProcess ();
		Process2Chat_ [proc] = chatTab;
		connect (proc,
				SIGNAL (finished (int, QProcess::ExitStatus)),
				this,
				SLOT (handleFinished ()));

		const auto& commandParts = XmlSettingsManager::Instance ()
				.property ("Command").toString ().split (" ", QString::SkipEmptyParts);
		const auto& command = commandParts.value (0);
		if (command.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty command";
			return;
		}
		proc->start (command, commandParts.mid (1) << text);
	}

	void Plugin::handleFinished ()
	{
		auto proc = qobject_cast<QProcess*> (sender ());
		proc->deleteLater ();

		if (!Process2Chat_.contains (proc))
		{
			qWarning () << Q_FUNC_INFO
					<< "no chat for process"
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

		QMetaObject::invokeMethod (Process2Chat_.take (proc),
				"prepareMessageText",
				Q_ARG (QString, out));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_shx, LeechCraft::Azoth::SHX::Plugin);
