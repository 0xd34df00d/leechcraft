/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "modnok.h"
#include <QIcon>
#include <QTextDocument>
#include <QProcess>
#include <QTranslator>
#include <QRegularExpression>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/sll/qtutil.h>
#include <util/sll/void.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Azoth
{
namespace Modnok
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Azoth_Modnok">;

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_modnok");

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothmodnoksettings.xml");

		XmlSettingsManager::Instance ().RegisterObject ("RenderCacheSize",
				this, "handleCacheSize");
		handleCacheSize ();

		XmlSettingsManager::Instance ().RegisterObject ({
					"HorizontalDPI",
					"VerticalDPI",
					"TextColor"
				},
				this, "clearCaches");

		const QStringList candidates
		{
			"/usr/local/bin",
			"/usr/bin",
			"/usr/local/share/leechcraft/azoth",
			"/usr/share/leechcraft/azoth"
		};

		for (const auto& dir : candidates)
		{
			const auto& path = dir + "/lc_azoth_modnok_latexconvert.sh";
			QFileInfo info { path };
			if (info.exists () &&
					info.isReadable () &&
					info.isExecutable ())
			{
				ConvScriptPath_ = path;
				break;
			}
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Modnok";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Modnok";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth Modnok allows one to render LaTeX formulas and display them in the chat window.");
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
		return SettingsDialog_;
	}

	namespace
	{
		bool IsSecure (const QString& formula)
		{
			static QRegularExpression rx ("\\\\(def|let|futurelet|newcommand|renewcomment|else|fi|write|input|include"
					"|chardef|catcode|makeatletter|noexpand|toksdef|every|errhelp|errorstopmode|scrollmode|nonstopmode|batchmode"
					"|read|csname|newhelp|relax|afterground|afterassignment|expandafter|noexpand|special|command|loop|repeat|toks"
					"|output|line|mathcode|name|item|section|mbox|DeclareRobustCommand)[^a-zA-Z]");
			return !formula.contains (rx);
		}
	}

	QImage Plugin::GetRenderedImage (const QString& formula)
	{
		if (FormulasCache_.contains (formula))
			return *FormulasCache_.object (formula);

		const QString& filename = Util::GetTemporaryName ("lc_azoth_modnok.XXXXXX.png");

		const int dpiX = XmlSettingsManager::Instance ().property ("HorizontalDPI").toInt ();
		const int dpiY = XmlSettingsManager::Instance ().property ("VerticalDPI").toInt ();
		const auto& color = XmlSettingsManager::Instance ().property ("TextColor").toString ();

		QStringList args;
		args << QString ("-r %1x%2").arg (dpiX).arg (dpiY);
		args << QString ("-o %1").arg (filename);
		args << "-t" << color;
		if (color == "white")
			args << "-b" << "black";

		// TODO
		const QString& incPath = QString ();
		if (!incPath.isEmpty ())
			args << (QString ("-x ") + incPath);

		args << formula;

		QProcess process;
		process.start (ConvScriptPath_, args);
		process.waitForFinished (5000);

		QImage img (filename);
		FormulasCache_.insert (formula, new QImage (filename), img.sizeInBytes () / 1024);

		QFile (filename).remove ();

		return img;
	}

	std::optional<QString> Plugin::NormalizeAndRender (QString formula)
	{
		formula = formula
				.remove ("$$")
				.replace ("&lt;", "<")
				.replace ("&gt;", ">")
				.replace ("&quot;", "\"")
				.replace ("&amp;", "&")
				.trimmed ();
		if (formula.isEmpty () || !IsSecure (formula))
			return {};

		const auto& rendered = GetRenderedImage (formula);
		if (rendered.isNull ())
			return {};

		return Util::GetAsBase64Src (rendered);
	}

	QString Plugin::HandleBody (QString body)
	{
		QMap<QString, QString> replaceMap;
		for (const auto& match : QRegularExpression { "\\$\\$.+?\\$\\$" }.globalMatch (body))
		{
			const auto& formula = match.captured (0);
			if (!replaceMap.contains (formula))
				NormalizeAndRender (formula)
					.transform ([&] (const QString& rendered)
						{
							replaceMap [formula] = rendered;
							return Util::Void {};
						});
		}

		for (const auto& [formula, image] : Util::Stlize (replaceMap))
		{
			auto escFormula = formula;
			escFormula.replace ('\"', "&quot;");
			escFormula.remove ("$$");
			const QString& imgTag = QString ("<img src=\"%1\" alt=\"%2\" style=\"vertical-align: middle;\" />")
					.arg (image)
					.arg (escFormula.trimmed ().simplified ());
			body.replace (formula, imgTag);
		}

		return body;
	}

	void Plugin::hookFormatBodyEnd (IHookProxy_ptr proxy, QObject*)
	{
		if (ConvScriptPath_.isEmpty ())
			return;

		if (!XmlSettingsManager::Instance ()
				.property ("OnDisplayRendering").toBool ())
			return;

		const QString& body = proxy->GetValue ("body").toString ();

		if (!body.contains ("$$"))
			return;

		const QString newBody = HandleBody (body);
		if (body != newBody)
			proxy->SetValue ("body", newBody);
	}

	void Plugin::hookGonnaHandleSmiles (IHookProxy_ptr proxy,
			QString body, QString)
	{
		if (ConvScriptPath_.isEmpty ())
			return;

		if (!XmlSettingsManager::Instance ()
				.property ("OnDisplayRendering").toBool ())
			return;

		if (!body.contains ("$$"))
			return;

		proxy->CancelDefault ();
	}

	void Plugin::hookMessageCreated (IHookProxy_ptr, QObject*, QObject *msgObj)
	{
		if (ConvScriptPath_.isEmpty ())
			return;

		if (!XmlSettingsManager::Instance ()
				.property ("SubstituteOutgoing").toBool ())
			return;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "message"
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		IRichTextMessage *richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		if (!richMsg)
			return;

		QString body = richMsg->GetRichBody ();
		if (body.isEmpty ())
			body = msg->GetBody ();

		if (!body.contains ("$$"))
			return;

		const QString newBody = HandleBody (body);
		if (newBody == body)
			return;

		if (XmlSettingsManager::Instance ()
				.property ("WarnInOutgoing").toBool ())
			msg->SetBody (msg->GetBody () + " (this message contains "
						"inline formulas, enable XHTML-IM to view them)");

		richMsg->SetRichBody (newBody);
	}

	void Plugin::clearCaches ()
	{
		FormulasCache_.clear ();
	}

	void Plugin::handleCacheSize ()
	{
		const int mibs = XmlSettingsManager::Instance ()
				.property ("RenderCacheSize").toInt ();
		FormulasCache_.setMaxCost (mibs * 1024);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_modnok, LC::Azoth::Modnok::Plugin);
