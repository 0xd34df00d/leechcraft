/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <util/sys/paths.h>

class QQuickImageProvider;

namespace LC
{
	/** @brief Describes a single quark.
	 *
	 * A single quark can be loaded in several different views at the
	 * same time. Each view defines a quark context. For example,
	 * the same panel of SB2 will have different contexts for different
	 * windows.
	 *
	 * \section cppcomm Communicating with C++
	 *
	 * Quarks may want to communicate with the C++ part of the providing
	 * plugin. Quark properties serve exactly this purpose.
	 *
	 * \section ctxprops Context-dependent and shareable properties
	 *
	 * Different views can share some properties and may need to have
	 * different others according to the quark context. Shareable
	 * properties should be listed in DynamicProps_ and StaticProps_,
	 * while unique per-view properties should be listed in
	 * ContextProps_. Context per-view properties will be destroyed if
	 * the quark is removed from the view.
	 *
	 * In other words, ownership of shareable properties is left for
	 * the quark component provider, while ownership of the
	 * context-dependent properties is transferred to the calling view.
	 *
	 * @sa IQuarkComponentProvider
	 */
	class QuarkComponent
	{
	public:
		/** @brief URL of the main QML of this file.
		 *
		 * This file will be loaded to the corresponding declarative view
		 * via a Loader or similar methods. It can be both a local file
		 * and a remote resource.
		 *
		 * Url_ + ".manifest" is also expected to exist, and it is a
		 * manifest file that describes the quark.
		 */
		QUrl Url_;

		/** @brief Dynamic properties to be exposed to the engine.
		 */
		QList<QPair<QString, QObject*>> DynamicProps_;

		/** @brief Context-depended properties to be exposed to the engine.
		 */
		std::vector<std::pair<QString, std::unique_ptr<QObject>>> ContextProps_;

		/** @brief Statis properties to be exposed to the engine.
		 *
		 * These properties are set once upon adding quark to a view,
		 * and they are not modified further.
		 */
		QList<QPair<QString, QVariant>> StaticProps_;

		/** @brief The image providers to be exposed to the engine.
		 *
		 * The list contains pairs of a QString and an image provider.
		 * Each image provider is added to the engine under the name in
		 * the corresponding QString upon addng the quark to the view.
		 */
		QList<QPair<QString, QQuickImageProvider*>> ImageProviders_;

		/** @brief Initializes a null quark component.
		 */
		QuarkComponent () = default;

		/** @brief Move-constructs this quark component from \em other.
		 *
		 * The \em other quark component is in unspecified state after the
		 * move and can only be destructed.
		 *
		 * @param[in] other The quark component to move from.
		 */
		QuarkComponent (QuarkComponent&& other) = default;

		/** @brief Move-assigns this quark component from \em other.
		 *
		 * The \em other quark component is in unspecified state after the
		 * move and can only be destructed.
		 *
		 * @param[in] other The quark component to move from.
		 * @return A reference to this quark component.
		 */
		QuarkComponent& operator= (QuarkComponent&& other) = default;

		/** @brief Initializes a quark component for the given file path.
		 *
		 * This utility constructor provides an easy way to create a
		 * quark component for a QML file located in the
		 * Util::SysPath::QML location under the given \em subdir and
		 * \em filename.
		 *
		 * For example, if a plugin installs its quark file
		 * \em QuarkName.qml contained in directory \em pluginName via
		 * \code install (DIRECTORY pluginName DESTINATION ${LC_QML_DEST}) \endcode
		 * then proper QuarkComponent will be initialized by calling
		 * \code{.cpp} QuarkComponent { "pluginName", "QuarkName.qml" } \endcode.
		 *
		 * @sa Util::GetSysPath()
		 */
		QuarkComponent (const QString& subdir, const QString& filename)
		: Url_ (Util::GetSysPathUrl (Util::SysPath::QML, subdir, filename))
		{
		}
	};

	/** @brief A shared pointer to a quark.
	 */
	typedef std::shared_ptr<QuarkComponent> QuarkComponent_ptr;

	/** @brief A list of quarks pointers.
	 */
	typedef QList<QuarkComponent_ptr> QuarkComponents_t;
}

/** @brief Interface for plugins providing quark components.
 */
class Q_DECL_EXPORT IQuarkComponentProvider
{
public:
	/** @brief Virtual destructor.
	 */
	virtual ~IQuarkComponentProvider () {}

	/** @brief Returns the list of quarks provided by this plugin.
	 *
	 * This function returns the list of smart pointers to quarks provided
	 * by this plugin. The caller will remove the pointers once he doesn't
	 * need them anymore. Thus, if the plugin doesn't retain the pointers,
	 * the corresponding QuarkComponent objects will be destroyed.
	 *
	 * Dynamic context-independent context properties
	 * (QuarkComponent::DynamicProps_) should be the same objects for
	 * each result of each invocation. Context-dependent properties (those
	 * in QuarkComponent::ContextProps_) should be created on each
	 * invocation of this method.
	 *
	 * @return The list of quark components provided by this plugin.
	 */
	virtual LC::QuarkComponents_t GetComponents () const = 0;
};

Q_DECLARE_INTERFACE (IQuarkComponentProvider, "org.Deviant.LeechCraft.IQuarkComponentProvider/1.0")
