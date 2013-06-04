include_directories (
	${LAURE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (VOLUMESLIDERTEST_MOC "volumeslidertest.h")
	QT4_WRAP_CPP (VOLUMESLIDER_MOC ${LAURE_SOURCE_DIR}/volumeslider.h)
endif ()
add_executable (lc_laure_volumeslidertest
	volumeslidertest.cpp
	${LAURE_SOURCE_DIR}/volumeslider.cpp
	${VOLUMESLIDER_MOC}
	${MOC_RCCS}
	${VOLUMESLIDERTEST_MOC}
)
target_link_libraries (lc_laure_volumeslidertest
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
)

add_test (VolumeSlider lc_laure_volumeslidertest)