/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ACETAMIDE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ACETAMIDE_H
#include <memory>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/azoth/iprotocolplugin.h>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class Plugin : public QObject
					, public IInfo
					, public IHaveSettings
					, public IPlugin2
					, public IProtocolPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IPlugin2
				LC::Azoth::IProtocolPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Acetamide")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<QTranslator> Translator_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetQObject ();
		QList<QObject*> GetProtocols () const;
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	public slots:
		void initPlugin (QObject*);
	signals:
		void gotEntity (const LC::Entity&);
		void gotNewProtocols (const QList<QObject*>&);
	};
}
}
}

QDataStream& operator<< (QDataStream& out, const QList<QStringList>& myObj);
QDataStream& operator>> (QDataStream& in, QList<QStringList>& myObj);

Q_DECLARE_METATYPE (QList<QStringList>)

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ACETAMIDE_H
