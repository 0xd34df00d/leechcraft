/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rulesstorage.h"
#include <QFile>
#include <QtDebug>
#include <QStringList>
#include <QDir>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

namespace LC
{
namespace KBSwitch
{
	namespace
	{
		QString FindX11Dir ()
		{
			static const QStringList dirs
			{
				"/etc/X11",
				"/usr/share/X11",
				"/usr/local/share/X11",
				"/usr/X11R6/lib/X11",
				"/usr/X11R6/lib64/X11",
				"/usr/local/X11R6/lib/X11",
				"/usr/local/X11R6/lib64/X11",
				"/usr/lib/X11",
				"/usr/lib64/X11",
				"/usr/local/lib/X11",
				"/usr/local/lib64/X11",
				"/usr/pkg/share/X11",
				"/usr/pkg/xorg/lib/X11"
			};

			for (const auto& dir : dirs)
				if (QFile::exists (dir + "/xkb/rules"))
					return dir;

			return {};
		}

		QString FindRulesFile (Display *display, const QString& x11dir)
		{
			XkbRF_VarDefsRec vd;
			char *path = 0;
			if (XkbRF_GetNamesProp (display, &path, &vd) && path)
			{
				const auto& pathStr = QFile::decodeName (path);
				free (path);
				return x11dir + "/xkb/rules/" + pathStr;
			}

			static const QStringList rfNames { "base", "xorg", "xfree86" };
			for (const auto& rfName : rfNames)
			{
				const auto& path = x11dir + "/xkb/rules/" + rfName;
				if (QFile::exists (path))
					return path;
			}

			return {};
		}

		struct VarDesc
		{
			QString Name_;
			QString Desc_;
		};

		QList<VarDesc> Decode (const XkbRF_DescribeVarsRec& vars)
		{
			QList<VarDesc> result;
			result.reserve (vars.num_desc);
			for (int i = 0; i < vars.num_desc; ++i)
			{
				const auto& desc = vars.desc [i];
				result.push_back ({ .Name_ = QString::fromUtf8 (desc.name), .Desc_ = QString::fromUtf8 (desc.desc) });
			}
			return result;
		}
	}

	RulesStorage::RulesStorage (Display *dpy, QObject *parent)
	: QObject (parent)
	, Display_ (dpy)
	, X11Dir_ (FindX11Dir ())
	{
		const auto& rf = FindRulesFile (Display_, X11Dir_);
		if (rf.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "rules file wasn't found";
			return;
		}

		char *locale = { 0 };
		const auto xkbRules = XkbRF_Load (QFile::encodeName (rf).data (),
				locale, true, true);
		if (!xkbRules)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot load rules from"
					<< rf;
			return;
		}

		for (const auto& [name, desc] : Decode (xkbRules->layouts))
		{
			LayName2Desc_ [name] = desc;
			LayDesc2Name_ [desc] = name;
		}

		for (const auto& [name, desc] : Decode (xkbRules->variants))
		{
			const auto& split = desc.split (':', Qt::SkipEmptyParts);
			const auto& varName = split.value (0).trimmed ();
			const auto& hrName = split.value (1).trimmed ();

			LayName2Variants_ [varName] << name;

			VarLayHR2NameVarPair_ [hrName] = std::make_pair (varName, name);
		}

		QStringList pcModelStrings;
		for (const auto& [name, desc] : Decode (xkbRules->models))
		{
			KBModels_ [name] = desc;

			const auto& kbString = QString ("%1 (%2)").arg (desc, name);
			if (name.startsWith ("pc10"))
				pcModelStrings << kbString;
			else
				KBModelsStrings_ << kbString;

			KBModelString2Code_ [kbString] = name;
		}
		pcModelStrings.sort ();
		KBModelsStrings_.sort ();
		KBModelsStrings_ = pcModelStrings + KBModelsStrings_;

		for (auto [name, desc] : Decode (xkbRules->options))
		{
			name = name.toLower ();

			const auto colonIdx = name.indexOf (':');
			if (colonIdx <= 0)
			{
				if (name == "compat")
					name = "numpad";

				if (name != "currencysign")
					Options_ [name] [{}] = desc;

				continue;
			}

			const auto& group = name.left (colonIdx);
			const auto& option = name.mid (colonIdx + 1);

			desc.replace ("&lt;", "<");
			desc.replace ("&gt;", ">");
			Options_ [group] [option] = desc;
		}

		XkbRF_Free (xkbRules, True);
	}

	const QHash<QString, QString>& RulesStorage::GetLayoutsD2N () const
	{
		return LayDesc2Name_;
	}

	const QHash<QString, QString>& RulesStorage::GetLayoutsN2D () const
	{
		return LayName2Desc_;
	}

	const QHash<QString, std::pair<QString, QString>>& RulesStorage::GetVariantsD2Layouts () const
	{
		return VarLayHR2NameVarPair_;
	}

	QStringList RulesStorage::GetLayoutVariants (const QString& layout) const
	{
		return LayName2Variants_ [layout];
	}

	const QHash<QString, QString>& RulesStorage::GetKBModels () const
	{
		return KBModels_;
	}

	const QStringList& RulesStorage::GetKBModelsStrings () const
	{
		return KBModelsStrings_;
	}

	QString RulesStorage::GetKBModelCode (const QString& string) const
	{
		return KBModelString2Code_ [string];
	}

	const QMap<QString, QMap<QString, QString>>& RulesStorage::GetOptions () const
	{
		return Options_;
	}
}
}
