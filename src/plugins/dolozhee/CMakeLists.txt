cmake_minimum_required (VERSION 3.3)
project (leechcraft_dolozhee)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dolozhee.cpp
		xmlgenerator.cpp
		reportwizard.cpp
		chooseuserpage.cpp
		userstatuspage.cpp
		reporttypepage.cpp
		bugreportpage.cpp
		featurerequestpage.cpp
		fileattachpage.cpp
		finalpage.cpp
		previewpage.cpp
	QT_COMPONENTS Network Widgets Xml
	)
