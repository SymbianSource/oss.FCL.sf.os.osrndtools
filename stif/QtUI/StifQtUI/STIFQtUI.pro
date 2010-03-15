HEADERS += version.h \
    frmmain.h \
    istfqtuicontroller.h \
    stfqtuicontroller.h \
    cstfcase.h \
    cstfmodule.h \
    istfqtuimodel.h \
    stfqtuimodel.h \
    dlgoutput.h \
    uisetting.h \
    dlgsetting.h
SOURCES += frmmain.cpp \
    main.cpp \
    stfqtuimodel.cpp \
    stfqtuicontroller.cpp \
    dlgoutput.cpp \
    uisetting.cpp \
    dlgsetting.cpp
RESOURCES += 
symbian { 
    TARGET.UID3 = 0x2002BCA0
    TARGET.EPOCALLOWDLLDATA = 1
    HEADERS += ../../../inc/.
    INCLUDEPATH += /epoc32/include/mw
    INCLUDEPATH += /epoc32/include/platform
    INCLUDEPATH += /epoc32/include/platform/stifinternal
    INCLUDEPATH += /epoc32/include/domain/osextensions
    INCLUDEPATH += /epoc32/include/domain/osextensions/stif
    HEADERS += stifexecutor.h
    SOURCES += stifexecutor.cpp
    LIBS += -leuser \
        -lefsrv \
        -lstiftestinterface \
        -lstiftfwif \
        -lstiftestengine \
        -lecons \
        -lhal \
        -lflogger
    TARGET.CAPABILITY = AllFiles \
        CommDD
    
    # Export headers to SDK Epoc32/include directory
    deploy.path = $$EPOCROOT
    exportheaders.sources = $$PUBLIC_HEADERS
    exportheaders.path = epoc32/include
    for(header, exportheaders.sources)
    :BLD_INF_RULES.prj_exports += "$$header $$deploy.path$$exportheaders.path/$$basename(header)"
}
