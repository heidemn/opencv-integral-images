QMAKE_CXXFLAGS += -Wall

SOURCES += \
	main.cpp ifeatures.cpp


CONFIG+=link_pkgconfig

PKGCONFIG += opencv

INCLUDEPATH += /usr/include/opencv/


