QT       += core gui opengl
#network
QT += core gui network
#playe模块
QT += multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
    INCLUDEPATH += $$PWD/include
    #LIBS += -L$$PWD/lib -lavformat-59 -lavfilter-8 -lavcodec-59 -lswresample-4 -lswscale-6 -lavutil-57
    LIBS += -L$$PWD/lib -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil
    LIBS += -L$$PWD/lib -lopencv_core452 -lopencv_imgproc452 -lopencv_highgui452 -lopencv_imgcodecs452 -lopencv_videoio452
    LIBS += -L$$PWD/lib -lWatermarkDecrypt
}
unix {
    QMAKE_LFLAGS += "-Wl,-Bsymbolic,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-Bsymbolic,-rpath,\'\$$ORIGIN/lib\'"
    QMAKE_LFLAGS += "-Wl,-Bsymbolic,-rpath,\'\$$ORIGIN/../lib\'"
    LIBS += -L$$PWD/lib -lWatermarkDecrypt
    #链接FFmpeg库文件
    LIBS += -lavformat -lavfilter -lavcodec -lswresample -lswscale -lavutil

    # 设置 OpenCV 头文件路径
    INCLUDEPATH += /usr/include/opencv4

    # 设置 OpenCV 库文件路径
    LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
}

SOURCES += \
    ImageArea.cpp \
    ffmpeg/FFmpegDecode.cpp \
    ffmpeg/FFmpegInclude.cpp \
    ffmpeg/FFmpegInsertWatermark.cpp \
    ffmpeg/FFmpegPlayWidget.cpp \
    ffmpeg/FFmpegRead.cpp \
    ffmpeg/FFmpegReadThread.cpp \
    homewindow.cpp \
    main.cpp \
    loginwindow.cpp \
    watermark/Dct.cpp \
    widget/imageLabel/roundimagelabel.cpp \
    widget/titlebar/homeTitleBar/hometitlebar.cpp \
    widget/titlebar/loginTitleBar/logintitlebar.cpp \
    widget/underline/underlinewidget.cpp

HEADERS += \
    ImageArea.h \
    entity/DecryptData.h \
    ffmpeg/FFmpegDecode.h \
    ffmpeg/FFmpegInclude.h \
    ffmpeg/FFmpegInsertWatermark.h \
    ffmpeg/FFmpegPlayWidget.h \
    ffmpeg/FFmpegRead.h \
    ffmpeg/FFmpegReadThread.h \
    homewindow.h \
    loginwindow.h \
    utils/DecryptUtil.h \
    utils/ParseUtil.h \
    watermark/Dct.h \
    watermark/ErrCode.h \
    watermarkdecrypt.h \
    widget/imageLabel/roundimagelabel.h \
    widget/titlebar/homeTitleBar/hometitlebar.h \
    widget/titlebar/loginTitleBar/logintitlebar.h \
    widget/underline/underlinewidget.h

FORMS += \
    homewindow.ui \
    loginwindow.ui \
    widget/imageLabel/roundimagelabel.ui \
    widget/titlebar/homeTitleBar/hometitlebar.ui \
    widget/titlebar/loginTitleBar/logintitlebar.ui \
    widget/underline/underlinewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
