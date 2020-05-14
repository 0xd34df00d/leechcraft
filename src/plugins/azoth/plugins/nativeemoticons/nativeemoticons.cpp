/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "nativeemoticons.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/util.h>
#include "nativeemoticonssource.h"
#include "kopeteemoticonssource.h"
#include "psiplusemoticonssource.h"

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_nativeemoticons");

		ResourceSources_ << new NativeEmoticonsSource ()
				<< new KopeteEmoticonsSource ()
				<< new PsiPlusEmoticonsSource ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.NativeEmoticons";
	}

	QString Plugin::GetName () const
	{
		return "Azoth NativeEmoticons";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for native Azoth emoticons packs as well as Kopete and Psi+ packs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin";
		return result;
	}

	QList<QObject*> Plugin::GetResourceSources () const
	{
		return ResourceSources_;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_nativeemoticons, LC::Azoth::NativeEmoticons::Plugin);
