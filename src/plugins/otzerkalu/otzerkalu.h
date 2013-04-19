/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#ifndef PLUGINS_OTZERKALU_OTZERKALU_H
#define PLUGINS_OTZERKALU_OTZERKALU_H
#include <QObject>
#include <QUrl>
#include <QStandardItemModel>
#include <util/idpool.h>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include "otzerkaludownloader.h"

namespace LeechCraft
{
namespace Otzerkalu
{
	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IJobHolder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IJobHolder)

		QStandardItemModel *RepresentationModel_;
		ICoreProxy_ptr Proxy_;
		Util::IDPool<int> MirrorIDPool_;
		enum Roles
		{
			RMirrorId = Qt::UserRole + 1
		};
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		EntityTestHandleResult CouldHandle (const Entity& entity) const;
		void Handle (Entity entity);
		QAbstractItemModel* GetRepresentation () const;
	private slots:
		void handleFileDownloaded (int id, int count);
		void handleMirroringFinished (int id);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&,
				int*, QObject**);
	};
}
}

#endif

