INCLUDE_DIRECTORIES (
	${LAURE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
QT4_WRAP_CPP (VOLUMESLIDERTEST_MOC "volumeslidertest.h")
QT4_WRAP_CPP (VOLUMESLIDER_MOC ${LAURE_SOURCE_DIR}/volumeslider.h)
ADD_EXECUTABLE (lc_laure_volumeslidertest
	volumeslidertest.cpp
	${LAURE_SOURCE_DIR}/volumeslider.cpp
	${VOLUMESLIDER_MOC}
	${MOC_RCCS}
	${VOLUMESLIDERTEST_MOC}
)
TARGET_LINK_LIBRARIES (lc_laure_volumeslidertest
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
)

ADD_TEST (VolumeSlider lc_laure_volumeslidertest)