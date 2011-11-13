/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_LAURE_H
#define PLUGINS_LAURE_LAURE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
namespace Laure
{
	class LaureWidget;
	class LastFMSubmitter;

	/** @brief An implementation of the Laure's plugin interface
	 *  @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class Plugin : public QObject
				, public IInfo
				, public IHaveTabs
				, public IEntityHandler
				, public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler IHaveSettings)

		TabClasses_t TabClasses_;
		QList<LaureWidget*> Others_;
		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
#ifdef HAVE_LASTFM
		LastFMSubmitter *LFSubmitter_;
#endif
	public:
		/** @brief Initializes the plugin.
		 *
		 * Init is called by the LeechCraft when it has finished
		 * initializing its core and is ready to initialize plugins.
		 * Generally, all initialization code should be placed into this
		 * method instead of plugin's instance object's constructor.
		 *
		 * After this call plugin should be in a usable state. That means
		 * that all data members should be initialized, callable from other
		 * plugins etc. That also means that in this function you can't rely
		 * on other plugins being initialized.
		 *
		 * @param[in] proxy The pointer to proxy to LeechCraft.
		 * 
		 * @sa Release
		 * @sa SecondInit
		 */
		void Init (ICoreProxy_ptr);
		
		/** @brief Performs second stage of initialization.
		 *
		 * This function is called when all the plugins are initialized by
		 * IInfo::Init() and may now communicate with others with no fear of
		 * getting init-order bugs.
		 *
		 * @sa Init
		 */
		void SecondInit ();
		
		/** @brief Returns the unique ID of the plugin.
		 *
		 * The ID should never change, event between different versions of
		 * the plugin and between renames of the plugin. It should be unique
		 * among all other plugins, thus the Vendor.AppName form is
		 * suggested. For example, Poshuku Browser plugin would return an ID
		 * like "org.LeechCraft.Poshuku", and Poshuku CleanWeb, which is
		 * Poshuku plugin, would return "org.LeechCraft.Poshuku.CleanWeb".
		 *
		 * The ID is allowed to consist of upper- and lowercase latin
		 * letters, numbers, dots¸ plus and minus sign.
		 *
		 * @return Unique and persistent ID of the plugin.
		 */
		QByteArray GetUniqueID () const;
		
		/** @brief Destroys the plugin.
		 *
		 * This function is called to notify that the plugin would be
		 * unloaded soon - either the application is closing down or the
		 * plugin is unloaded for some reason. Plugin should free its
		 * resources and especially all the GUI stuff in this function
		 * instead of plugin instance's destructor.
		 *
		 * @sa Init
		 */
		void Release ();
		
		/** @brief Returns the name of the plugin.
		 *
		 * This name is used only for the UI, internal communication is done
		 * through pointers to QObjects representing plugin instance
		 * objects.
		 *
		 * @note This function should be able to work before Init() is
		 * called.
		 *
		 * @sa GetInfo
		 * @sa GetIcon
		 */
		QString GetName () const;
		
		/** @brief Returns the information string about the plugin.
		 *
		 * Information string is only used to describe the plugin to the
		 * user.
		 *
		 * @note This function should be able to work before Init() is
		 * called.
		 *
		 * @sa GetName
		 * @sa GetInfo
		 */
		QString GetInfo () const;
		
		/** @brief Returns the plugin icon.
		 *
		 * The icon is used only in GUI stuff.
		 *
		 * @note This function should be able to work before Init() is
		 * called.
		 *
		 * @return Icon object.
		 *
		 * @sa GetName
		 * @sa GetInfo
		 */
		QIcon GetIcon () const;

		/** @brief Returns the list of tab classes provided by this plugin.
		 * 
		 * This list must not change between different calls to this
		 * function.
		 * 
		 * @note Actually, not all tab classes returned from this method
		 * have to result in a new tab being opened when called the
		 * TabOpenRequested() method. For example, the Azoth plugin returns
		 * a tab class for a fictional tab that, when passed to the
		 * TabOpenRequested() method, results in MUC join dialog appearing.
		 * 
		 * @return The list of tab classes this plugin provides.
		 * 
		 * @sa TabClassInfo, ITabWidget::GetTabClass(), TabOpenRequested().
		 */
		TabClasses_t GetTabClasses () const;
		
		/** @brief Opens the new tab from the given tabClass.
		 * 
		 * This method is called to notify the plugin that a tab of the
		 * given tabClass is requested by the user.
		 * 
		 * @note Please note that the call to this method doesn't have to
		 * result in a new tab being opened. Refer to the note in
		 * GetTabClasses() documentation for more information.
		 * 
		 * @param[in] tabClass The class of the requested tab, from the
		 * returned from GetTabClasses() list.
		 * 
		 * @sa GetTabClasses()
		 */
		void TabOpenRequested (const QByteArray& tabClass);

		/** @brief Returns whether plugin can handle given entity.
		 *
		 * This function is used to query every loaded plugin providing the
		 * IDownload interface whether it could handle the entity entered by
		 * user or generated automatically with given task parameters.
		 * Entity could be anything from file name to URL to all kinds of
		 * hashes like Magnet links.
		 *
		 * @param[in] entity A Entity structure that could be possibly
		 * handled by this plugin.
		 * @return The result of entity 
		 *
		 * @sa Handle
		 * @sa LeechCraft::Entity
		 */
		EntityTestHandleResult CouldHandle (const Entity&) const;
		
		/** @brief Notifies the plugin that it should handle the given entity.
		 *
		 * This function is called to make IEntityHandle know that it should
		 * handle the given entity. The entity is guaranteed to be checked
		 * previously against CouldHandle().
		 *
		 * @param[in] entity A Entity structure to be handled by
		 * this plugin.
		 *
		 * @sa LeechCraft::Entity
		 */
		void Handle (Entity);
		
		/** @brief Gets the settings dialog manager object from the plugin.
		 *
		 * The returned XmlSettingsDialog would be integrated into common
		 * settings dialog where user can configure all the plugins that
		 * provide this interface.
		 *
		 * @return The XmlSettingsDialog object that manages the settings
		 * dialog of the plugin.
		 */
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		LaureWidget* CreateTab ();
	signals:
		/** @brief This signal is emitted by plugin to add a new tab.
		 * 
		 * tabContents must implement the ITabWidget interface to be
		 * successfully added to the tab widget.
		 * 
		 * For the tab to have an icon, emit the changeTabIcon() signal
		 * after emitting this one.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @param[out] name The initial name of the tab.
		 * @param[out] tabContents The widget to be added, must implement
		 * ITabWidget.
		 * 
		 * @sa removeTab(), changeTabName(), changeTabIcon(),
		 * statusBarChanged(), raiseTab().
		 */
		void addNewTab (const QString&, QWidget*);
		
		/** @brief This signal is emitted by plugin when it wants to remove
		 * a tab.
		 * 
		 * tabContents must be a widget previously added by emitting the
		 * addNewTab() signal by this same plugin.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @param[out] tabContents The widget to remove from the tab widget,
		 * must be previously added with addNewTab().
		 * 
		 * @sa addNewTab()
		 */
		void removeTab (QWidget*);
		
		/** @brief This signal is emitted by plugin to change the name of
		 * the tab with the given tabContents.
		 * 
		 * The name of the tab is shown in the tab bar of the tab widget. It
		 * also may be shown in other places and contexts, like in the
		 * LeechCraft title bar when the corresponding tab is active.
		 * 
		 * The tab is identified by tabContents, which should be the widget
		 * previously added by emitting the addNewTab() signal by this same
		 * plugin.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @param[out] tabContents The widget with the contents of the tab
		 * which name should be changed, added previously with addNewTab().
		 * @param[out] name The new name of the tab with tabContents.
		 * 
		 * @sa addNewTab().
		 */
		void changeTabName (QWidget*, const QString&);
		
		/** @brief This signal is emitted by plugin to change the icon of
		 * the tab with the given tabContents.
		 * 
		 * The tab is identified by tabContents, which should be the widget
		 * previously added by emitting the addNewTab() signal by this same
		 * plugin.
		 * 
		 * Null icon object may be used to clear the icon.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @param[out] tabContents The widget with the contents of the tab
		 * which icon should be changed, added previously with addNewTab().
		 * @param[out] icon The new icon of the tab with tabContents.
		 * 
		 * @sa addNewTab().
		 */
		void changeTabIcon (QWidget*, const QIcon&);
		
		//void changeTooltip (QWidget*, QWidget*);
		
		/** @brief This signal is emitted by plugin to change the status bar
		 * text for the tab with the given tabContents.
		 * 
		 * The text set by this signal would be shown when the corresponding
		 * tab is active. To clear the status bar, this signal should be
		 * emitted with empty text.
		 * 
		 * The tab is identified by tabContents, which should be the widget
		 * previously added by emitting the addNewTab() signal by this same
		 * plugin.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @note User may choose to hide the status bar, so important
		 * information should not be presented this way.
		 * 
		 * @param[out] tabContents The widget with the contents of the tab
		 * which statusbar text should be changed, added previously with
		 * addNewTab().
		 * @param[out] text The new statusbar text of the tab with
		 * tabContents.
		 * 
		 * @sa addNewTab().
		 */
		void statusBarChanged (QWidget*, const QString&);
		
		/** @brief This signal is emitted by plugin to bring the tab with
		 * the given tabContents to the front.
		 * 
		 * The tab is identified by tabContents, which should be the widget
		 * previously added by emitting the addNewTab() signal by this same
		 * plugin.
		 * 
		 * @note This function is expected to be a signal in subclasses.
		 * 
		 * @param[out] tabContents The widget with the contents of the tab
		 * that should be brought to the front.
		 */
		void raiseTab (QWidget*);
		
		/** @brief This signal is emitted by plugin to notify the Core and
		 * other plugins about an entity.
		 *
		 * In this case, the plugin doesn't care what would happen next to
		 * the entity after the announcement and whether it would be catched
		 * by any other plugin at all. This is the opposite to the semantics
		 * of delegateEntity().
		 *
		 * This signal is typically emitted, for example, when a plugin has
		 * just finished downloading something and wants to notify other
		 * plugins about newly created files.
		 *
		 * This signal is asynchronous: the handling happends after the
		 * control gets back to the event loop.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] entity The entity.
		 */
		void gotEntity (const Entity&);
		
		/** @brief This signal is emitted by plugin to delegate the entity
		 * to an another plugin.
		 *
		 * In this case, the plugin actually cares whether the entity would
		 * be handled. This signal is typically used, for example, to
		 * delegate a download request.
		 *
		 * id and provider are used in download delegation requests. If
		 * these parameters are not NULL and the entity is handled, they are
		 * set to the task ID returned by the corresponding IDownload
		 * instance and the main plugin instance of the handling plugin,
		 * respectively. Thus, setting the id to a non-NULL value means that
		 * only downloading may occur as result but no handling.
		 *
		 * Nevertheless, if you need to enable entity handlers to handle
		 * your request as well, you may leave the id parameter as NULL and
		 * just set the provider to a non-NULL value.
		 *
		 * @note This function is expected to be a signal in subclasses.
		 *
		 * @param[out] entity The entity to delegate.
		 * @param[in] id The pointer to the variable that would contain the
		 * task ID of this delegate request, or NULL.
		 * @param[in] provider The pointer to the main plugin instance of
		 * the plugin that handles this delegate request, or NULL.
		 */
		void delegateEntity (const Entity&, int*, QObject**);
	private slots:
		void handleNeedToClose ();
	};
}
}

#endif
