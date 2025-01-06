SOURCES += \
    $$PWD/spdlog/src/async.cpp \
    $$PWD/spdlog/src/bundled_fmtlib_format.cpp \
    $$PWD/spdlog/src/cfg.cpp \
    $$PWD/spdlog/src/color_sinks.cpp \
    $$PWD/spdlog/src/file_sinks.cpp \
    $$PWD/spdlog/src/spdlog.cpp \
    $$PWD/spdlog/src/stdout_sinks.cpp \
    $$PWD/qspdlogger.cpp

HEADERS += \
    $$PWD/qspdlogger.h

INCLUDEPATH += \
    $$PWD/spdlog/include \
    $$PWD

DEFINES += SPDLOG_COMPILED_LIB
