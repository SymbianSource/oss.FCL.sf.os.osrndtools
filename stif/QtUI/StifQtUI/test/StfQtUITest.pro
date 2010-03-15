HEADERS += ..\frmmain.h \
    ..\istfqtuicontroller.h \
    ..\stfqtuicontroller.h \
    ..\cstfcase.h \
    ..\cstfmodule.h \
    ..\istfqtuimodel.h \
    ..\stfqtuimodel.h \
    ..\dlgoutput.h \
    testcontroller.h 
SOURCES += ..\frmmain.cpp \
    ..\stfqtuimodel.cpp \
    ..\stfqtuicontroller.cpp \
    ..\dlgoutput.cpp \
    testcontroller.cpp \
    main.cpp 

CONFIG += qtestlib

RESOURCES +=
symbian {
	   TARGET.UID3 = 0xA000C60B
    #include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
    TARGET.EPOCALLOWDLLDATA = 1
    HEADERS += ../../../../inc/.
    include(.\platform\stifinternal)
    HEADERS += ../stifexecutor.h
    SOURCES += ../stifexecutor.cpp
    LIBS += -leuser \
        -lefsrv \
        -lstiftestinterface \
        -lstiftfwif \
        -lstiftestengine \
        -lecons \
        -lhal \
        -lflogger
    TARGET.CAPABILITY = ReadUserData \
        WriteUserData
    
    # Export headers to SDK Epoc32/include directory
    deploy.path = $$EPOCROOT
    exportheaders.sources = $$PUBLIC_HEADERS
    exportheaders.path = epoc32/include
    for(header, exportheaders.sources)
    :BLD_INF_RULES.prj_exports += "$$header $$deploy.path$$exportheaders.path/$$basename(header)"
}
