cmake_minimum_required(VERSION 3.15)

# TODO: вписать рабочее название проекта
project(acu LANGUAGES CXX)

# Устанавливаем название приложения
SET(PROJECT_VENDOR_LONG "ANYKS - conversion utility (ACU)")

# Опции сборки
SET(PROJECT_COPYRIGHT "Copyright (c) ${CURRENT_YEAR} ${PROJECT_VENDOR_LONG}")

# Тип сборки
# cmake -DCMAKE_BUILD_TYPE=YES ..
if (NOT CMAKE_BUILD_TYPE)
    # Активируем режим отладки
    SET(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

# Выводим сообщение о типе сборки
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}\n")
# Выводим информацию каталога где выполняется сброка
message(STATUS "Build dir: ${CMAKE_CURRENT_BINARY_DIR}\n")

# Получаем архитектуру
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message(STATUS "Architecture: ${ARCHITECTURE}")

# Проверка версии OS
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected OS: Linux")
    SET(DEBUGGER "-ggdb3")
    # Используем Clang
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
     # Используем GCC
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")
    # Используем Intel C++
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # Используем Visual Studio C++
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
# Если операционной системой является Solaris
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
    if (NOT OS_VERSION)
        EXECUTE_PROCESS(COMMAND uname -a COMMAND gawk "{print $4}" COMMAND tr -d '\n' OUTPUT_VARIABLE OS_VERSION)
    endif()
    message(STATUS "Detected OS: Solaris v${OS_VERSION}")
    SET(DEBUGGER "-ggdb3")
     # Используем Clang
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
     # Используем GCC
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")
    endif()
# Если операционной системой является MacOS X
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if (NOT OS_VERSION)
        EXECUTE_PROCESS(COMMAND sw_vers -productVersion COMMAND tr -d '\n' OUTPUT_VARIABLE OS_VERSION)
    endif()
    message(STATUS "Detected OS: MacOS X v${OS_VERSION}")
    SET(DEBUGGER "-glldb")    
    SET(CMAKE_OSX_DEPLOYMENT_TARGET ${OS_VERSION})
# Если операционной системой является FreeBSD
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    if (NOT OS_VERSION)
        EXECUTE_PROCESS(COMMAND uname -a COMMAND awk "{print $3}" COMMAND tr -d '\n' OUTPUT_VARIABLE OS_VERSION)
    endif()
    message(STATUS "Detected OS: FreeBSD v${OS_VERSION}")
    SET(DEBUGGER "-ggdb3")
# Если операционной системой является NetBSD
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD")
    message(STATUS "Detected OS: NetBSD")
    SET(DEBUGGER "-ggdb3")
# Если операционной системой является OpenBSD
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    message(STATUS "Detected OS: OpenBSD")
    SET(DEBUGGER "-ggdb3")
# Если операционной системой является Windows
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Detected OS: Microsoft Windows")
    SET(DEBUGGER "-ggdb3")
# Если операционная система не определена
else()
    SET(DEBUGGER "")
endif()

# Enable C++17
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Используем Clang
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Устанавливаем опции компилятора
    SET(CMAKE_C_COMPILER_ID "clang")
    SET(CMAKE_CXX_COMPILER_ID "clang++")
# Используем GCC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Устанавливаем опции компилятора
    SET(CMAKE_C_COMPILER_ID "GNU")
    SET(CMAKE_CXX_COMPILER_ID "GNU")
# Используем Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # Устанавливаем опции компилятора
    SET(CMAKE_C_COMPILER_ID "Intel")
    SET(CMAKE_CXX_COMPILER_ID "Intel")
# Используем Visual Studio C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Устанавливаем опции компилятора
    SET(CMAKE_C_COMPILER_ID "MSVC")
    SET(CMAKE_CXX_COMPILER_ID "MSVC")
endif()

# Выводим информацию об установленных компиляторах
message(STATUS "Using C compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\n")

# Информация и времени компиляции
string(TIMESTAMP CURRENT_YEAR "%Y")
string(TIMESTAMP CURRENT_TIME "%d.%m.%Y %H:%M:%S")

# Опции компилятора
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Устанавливаем параметры игнорирования ошибок для Windows
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(ACU_PARAMS "-municode -Wno-cpp -Wno-deprecated")
# Устанавливаем параметры игнорирования ошибок для Linux
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    SET(ACU_PARAMS "-Wno-cpp -Wno-deprecated")
# Устанавливаем параметры игнорирования ошибок для Solaris
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
    SET(ACU_PARAMS "-lsocket -lnsl -lc -Wno-cpp -Wno-deprecated")
# Устанавливаем параметры игнорирования ошибок для MacOS X
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    SET(ACU_PARAMS "-framework Carbon -Wno-cpp -Wno-deprecated -Wno-unused-command-line-argument")
# Устанавливаем параметры игнорирования ошибок для FreeBSD, NetBSD и OpenBSD
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD" OR ${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD" OR ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
    SET(ACU_PARAMS "-lutil -Wno-cpp -Wno-deprecated -Wno-unused-command-line-argument")
# Если операционная система не определена
else()
    SET(ACU_PARAMS "-Wno-unknown-attributes")
endif()

if (${ARCHITECTURE} MATCHES "aarch64" OR ${ARCHITECTURE} MATCHES "arm*")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -DNDEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-permissive -Wno-pedantic -Wno-narrowing -Wno-deprecated-declarations -Wno-exceptions -Wno-unused-result ${ACU_PARAMS}")
else()
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -mrdrnd -march=core2 -DNDEBUG")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-permissive -Wno-pedantic -Wno-narrowing -Wno-deprecated-declarations -Wno-exceptions -Wno-unused-result ${ACU_PARAMS}")
endif()

# Устанавливаем параметры компиляции для режима отладки
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG_MODE ${DEBUGGER}")

# Если активирован режим отладки
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "C Flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}\n")
# Если активирован режим релиза
else()
    message(STATUS "C Flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\n")
endif()

# Если нужно собрать и динамическую библиотеку
if (CMAKE_SHARED_BUILD_LIB)
    if (MSVC)
        SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif(MSVC)
endif(CMAKE_SHARED_BUILD_LIB)

# Разрешаем сборку IDN
SET(CMAKE_BUILD_IDN ON)
add_compile_definitions(AWH_IDN)

# Добавляем модули cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmake")

# Выполняем поиск нужных библиотек
find_package(AWH REQUIRED)
find_package(XML REQUIRED)
find_package(YAML REQUIRED)
find_package(TcMalloc REQUIRED)

# Провекра XML
if (XML_FOUND)
    message(STATUS "XML lib found: " ${XML_LIBRARY})
    message(STATUS "XML include dir found: " ${XML_INCLUDE_DIR})
else (XML_FOUND)
    message(FATAL_ERROR "XML not found")
endif (XML_FOUND)

# Провекра YAML
if (YAML_FOUND)
    message(STATUS "YAML lib found: " ${YAML_LIBRARY})
    message(STATUS "YAML include dir found: " ${YAML_INCLUDE_DIR})
else (YAML_FOUND)
    message(FATAL_ERROR "YAML not found")
endif (YAML_FOUND)

# Провекра TcMalloc
if (TcMalloc_FOUND)
    message(STATUS "TcMalloc lib found: " ${TCMALLOC_LIBRARY} " " ${TCMALLOC_COMMON_LIBRARY})
else (TcMalloc_FOUND)
    message(FATAL_ERROR "TcMalloc not found")
endif (TcMalloc_FOUND)

# Провекра AWH
if (AWH_FOUND)
    message(STATUS "AWH include dir found: " ${AWH_INCLUDE_DIR})
    message(STATUS "CITY include dir found: " ${CITY_INCLUDE_DIR})
    message(STATUS "LZ4 include dir found: " ${LZ4_INCLUDE_DIR})
    message(STATUS "BZ2 include dir found: " ${BZ2_INCLUDE_DIR})
    message(STATUS "ZSTD include dir found: " ${ZSTD_INCLUDE_DIR})
    message(STATUS "LZMA include dir found: " ${LZMA_INCLUDE_DIR})
    message(STATUS "ZLib include dir found: " ${ZLIB_INCLUDE_DIR})
    message(STATUS "PCRE2 include dir found: " ${PCRE_INCLUDE_DIR})
    message(STATUS "NgHttp2 include dir found: " ${NGHTTP2_INCLUDE_DIR})
    message(STATUS "OpenSSL include dir found: " ${OPENSSL_INCLUDE_DIR})
    message(STATUS "Brotli include dir found: " ${BROTLI_INCLUDE_ENCODE_DIR} " " ${BROTLI_INCLUDE_DECODE_DIR})

    # Сборка модуля AWH_IDN, если операционной системой не является Windows
    if (CMAKE_BUILD_IDN AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
        message(STATUS "IDN2 include dir found: " ${IDN2_INCLUDE_DIR})
        message(STATUS "ICONV include dir found: " ${ICONV_INCLUDE_DIR})
    endif()
else (AWH_FOUND)
    message(FATAL_ERROR "Dependence not found")
endif (AWH_FOUND)

# Выполняем подключение хидеров
include_directories(BEFORE ${XML_INCLUDE_DIR})
include_directories(BEFORE ${AWH_INCLUDE_DIRS})
include_directories(BEFORE ${YAML_INCLUDE_DIR})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/include)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/contrib/include)
include_directories(SYSTEM ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Выполняем подключение исходников
SET(SOURCE_FILES
    "${SOURCE_FILES}"
    "${CMAKE_SOURCE_DIR}/src/cef.cpp"
    "${CMAKE_SOURCE_DIR}/src/csv.cpp"
    "${CMAKE_SOURCE_DIR}/src/grok.cpp"
    "${CMAKE_SOURCE_DIR}/src/syslog.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser.cpp"
    "${CMAKE_SOURCE_DIR}/src/server.cpp"
)

# Задаем именя библиотеки и бинарника
SET(SERVER_APP_NAME server)
SET(ACU_LIB_NAME ${PROJECT_NAME})
SET(ACU_APP_NAME ${PROJECT_NAME}_bin)

# Если нужно собрать и динамическую библиотеку
if (CMAKE_SHARED_BUILD_LIB)
    # Добавляем динамическую библиотеку
    add_library(${ACU_LIB_NAME} SHARED ${SOURCE_FILES})
    # Выполняем экспорт методов доступных извне в динамических библиотеках
    target_compile_definitions(${ACU_LIB_NAME} PRIVATE CITYHASH_DLL_EXPORTS)
    target_compile_definitions(${ACU_LIB_NAME} PRIVATE ACU_SHARED_LIBRARY_EXPORT)
    target_compile_definitions(${ACU_LIB_NAME} PRIVATE AWH_SHARED_LIBRARY_EXPORT)
# Если нужно собрать и статическую библиотеку
else (CMAKE_SHARED_BUILD_LIB)
    # Активируем сборку статической библиотеки
    add_compile_definitions(ACU_STATICLIB)
    # Добавляем статическу библиотеку
    add_library(${ACU_LIB_NAME} STATIC ${SOURCE_FILES})
endif (CMAKE_SHARED_BUILD_LIB)

# Если операционной системой является Windows
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # export MSYS="error_start:%WD%dumper.exe -d -c %1 %2"
    # Устанавливаем необходимые зависимости для MS Windows
    SET(WINFLAGS -lws2_32 -lgdi32 -lcrypt32 -lIphlpapi -lbcrypt -flto=auto)

    # Устанавливаем иконку для windows под MinGW.
    SET(RES_FILES ${CMAKE_PROJECT_NAME}.rc)
    SET(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")

    # Выполняем создание приложений
    add_executable(${ACU_APP_NAME} app/acu.cpp ${RES_FILES})
    add_executable(${SERVER_APP_NAME} app/server.cpp ${RES_FILES})
# Если операционной системой является Linux, FreeBSD, MacOS X
else()
    # Выполняем создание приложений
    add_executable(${ACU_APP_NAME} app/acu.cpp)
    add_executable(${SERVER_APP_NAME} app/server.cpp)
endif()

# Выполняем восстановление названия приложения
set_property(TARGET ${PROJECT_NAME}_bin PROPERTY OUTPUT_NAME ${PROJECT_NAME})

# Если активирован режим отладки
if (CMAKE_BUILD_TYPE MATCHES Debug)
    # Удаляем библиотеку TCMalloc
    SET(TCMALLOC_LIBRARIES "")
endif()

# Если нужно собрать и динамическую библиотеку
if (CMAKE_SHARED_BUILD_LIB)
    # Если операционной системой является Windows
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

        # Выполняем сборку динамической библиотеки
        target_link_libraries(
            ${ACU_LIB_NAME}
            PUBLIC
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${TCMALLOC_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
            ${TCMALLOC_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

    # Если операционной системой является Linux, FreeBSD, MacOS X
    else()

        # Выполняем сборку динамической библиотеки
        target_link_libraries(
            ${ACU_LIB_NAME}
            PUBLIC
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
        )
        
        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${TCMALLOC_LIBRARIES}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
            ${TCMALLOC_LIBRARIES}
        )

    endif()
# Если нужно собрать и статическую библиотеку
else (CMAKE_SHARED_BUILD_LIB)
    # Если операционной системой является Windows
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

    # Если операционной системой является Linux, FreeBSD, MacOS X
    else()

        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${TCMALLOC_LIBRARIES}
        )

    endif()
endif (CMAKE_SHARED_BUILD_LIB)

# Активация формирования дампа ядра в реже отладки под MacOS X
if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        SET(entitlements_file "${CMAKE_SOURCE_DIR}/segv.entitlements")
        
        add_custom_command(TARGET "${ACU_APP_NAME}" POST_BUILD
            COMMAND bash "${CMAKE_SOURCE_DIR}/core_dump.sh" "$<TARGET_FILE:${ACU_APP_NAME}>" "${entitlements_file}"
            COMMENT "Sign target \"${ACU_APP_NAME}\" to enable core dump on MacOS X"
            VERBATIM
        )

        add_custom_command(TARGET "${SERVER_APP_NAME}" POST_BUILD
            COMMAND bash "${CMAKE_SOURCE_DIR}/core_dump.sh" "$<TARGET_FILE:${SERVER_APP_NAME}>" "${entitlements_file}"
            COMMENT "Sign target \"${SERVER_APP_NAME}\" to enable core dump on MacOS X"
            VERBATIM
        )

    endif()
    # Сборка модуля CMAKE_ENABLED_PVS
    if (CMAKE_ENABLED_PVS)
        include(FetchContent)
        FetchContent_Declare(
            PVS_CMakeModule
            GIT_REPOSITORY "https://github.com/viva64/pvs-studio-cmake-module.git"
            GIT_TAG        "master" 
        )
        FetchContent_MakeAvailable(PVS_CMakeModule)
        include("${pvs_cmakemodule_SOURCE_DIR}/PVS-Studio.cmake")
        # Выполняем анализ собранных приложений
        pvs_studio_add_target(TARGET ${ACU_APP_NAME}.analyze ALL OUTPUT FORMAT json ANALYZE ${ACU_APP_NAME} MODE GA:1,2 LOG target.err ARGS -e ${CMAKE_CURRENT_BINARY_DIR})
        pvs_studio_add_target(TARGET ${SERVER_APP_NAME}.analyze ALL OUTPUT FORMAT json ANALYZE ${SERVER_APP_NAME} MODE GA:1,2 LOG target.err ARGS -e ${CMAKE_CURRENT_BINARY_DIR})
    endif (CMAKE_ENABLED_PVS)
endif()

# Устанавливаем адрес установки, запрет установки в /usr/local
if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local/")
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif()

# Устанавливаем хидеры сторонних библиотек
install(DIRECTORY "${XML_INCLUDE_DIR}" DESTINATION "${CMAKE_INSTALL_PREFIX}/include" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "${YAML_INCLUDE_DIR}" DESTINATION "${CMAKE_INSTALL_PREFIX}/include" FILES_MATCHING PATTERN "*.h")

# Устанавливаем хидеры библиотеки
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}" FILES_MATCHING PATTERN "*.hpp")

# Устанавливаем хидеры сторонних библиотек
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/contrib/include/" DESTINATION "${CMAKE_INSTALL_PREFIX}/include" FILES_MATCHING PATTERN "*.h" PATTERN "*.hh" PATTERN "*.hpp")

# Устанавливаем стороние библиотеки
install(FILES ${XML_LIBRARY} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES ${YAML_LIBRARY} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
# Устанавливаем статическую библиотеку TcMalloc
install(FILES ${TCMALLOC_LIBRARIES} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

# Устанавливаем собранную библиотеку утилиты
install(TARGETS ${ACU_LIB_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

# Устанавливаем исполнительные файлы
install(TARGETS ${ACU_APP_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/${CMAKE_PROJECT_NAME}")
install(TARGETS ${SERVER_APP_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/${CMAKE_PROJECT_NAME}")
