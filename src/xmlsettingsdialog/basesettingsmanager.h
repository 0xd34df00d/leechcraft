/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#ifndef BASESETTINGSMANAGER_H
#define BASESETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QDynamicPropertyChangeEvent>
#include "config.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

namespace LeechCraft
{
	namespace Util
	{
		/*! @brief Base class for settings manager.
		 *
		 * Facilitates creation of settings managers due to providing some
		 * frequently used features.
		 */
		class LEECHCRAFT_API BaseSettingsManager : public QObject
		{
			Q_OBJECT

			QMap<QByteArray, QPair<QObject*, QByteArray> > Properties2Object_;
			bool Initializing_;
			QSettings *Settings_;
		public:
		 /** @brief Initalizes the settings manager.
		  *
		  * Loads all settings from the QSettings created by BeginSettings and
		  * creates dynamic properties for them.
		  *
		  * @sa Release
		  */
			void Init ();
		 /** @brief Prepares the settings manager for deletion.
		  *
		  * Flushes all settigns to the QSettings created by BeginSettings
		  * to prepare settings manager object for the deletion.
		  *
		  * @sa Init
		  */
			void Release ();
		 /** @brief Subscribes object to property changes.
		  *
		  * When a property changes, a specified object is called to notify
		  * it that the property has changed.
		  *
		  * @param[in] propName The name of property object wants to
		  * subscribe to.
		  * @param[in] object The object instance that will get
		  * notifications.
		  * @param[in] funcName Name of the function that will be called.
		  * Note that it should be known to the Qt's metaobject system, so
		  * it should be a (public) slot.
		  */
		void RegisterObject (const QByteArray& propName,
				QObject* object, const QByteArray& funcName);
		 /** @brief Gets a property with default value.
		  *
		  * This is a wrapper around standart QObject::property() function.
		  * It checks whether specified property exists, and if so, it
		  * returns its value, otherwise it creates this property, sets its
		  * value to def and returns def.
		  *
		  * @param[in] propName Name of the property that should be checked
		  * and returned.
		  * @param[in] def Default value of the property.
		  * @return Resulting value of the property.
		  */
			QVariant Property (const QString& propName, const QVariant& def);
		protected:
			virtual bool event (QEvent*);
		 /*! @brief Allocates and returns a QSettings object suitable for
		  * use.
		  *
		  * @return The created QSettings object.
		  * @sa EndSettings
		  */
			virtual QSettings* BeginSettings () const = 0;
		 /*! @brief Correctly closes the QSettings object.
		  *
		  * Closes the QSettings object previously created by
		  * BeginSettings. It should NOT delete it, BaseSettignsManager's
		  * code would do that.
		  *
		  * @param[in] settings The QSettings object.
		  * @sa BeginSettings
		  */
			virtual void EndSettings (QSettings *settings) const = 0;
		};
	};
};

#endif

