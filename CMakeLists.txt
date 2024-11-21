cmake_minimum_required(VERSION 3.15)

# TODO: вписать рабочее название проекта
project(acu LANGUAGES CXX)

# Устанавливаем название приложения
set(PROJECT_VENDOR_LONG "ANYKS - conversion utility (ACU)")

# Опции сборки
set(PROJECT_COPYRIGHT "Copyright (c) ${CURRENT_YEAR} ${PROJECT_VENDOR_LONG}")

# Тип сборки
# cmake -DCMAKE_BUILD_TYPE=YES ..
if(NOT CMAKE_BUILD_TYPE)
    # Активируем режим отладки
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

# Выводим сообщение о типе сборки
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}\n")
# Выводим информацию каталога где выполняется сброка
message(STATUS "Build dir: ${CMAKE_CURRENT_BINARY_DIR}\n")

# Получаем архитектуру
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message(STATUS "Architecture: ${ARCHITECTURE}")

# Проверка версии OS
string(REGEX MATCH "Linux" PROJECT_OS_LINUX ${CMAKE_SYSTEM_NAME})
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Detected OS: Linux")
    set(DEBUGGER "-ggdb3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if(NOT MACOS_VERSION)
        EXECUTE_PROCESS(COMMAND sw_vers -productVersion COMMAND tr -d '\n' OUTPUT_VARIABLE MACOS_VERSION)
    endif()
    message(STATUS "Detected OS: MacOS X v${MACOS_VERSION}")
    set(DEBUGGER "-glldb")    
    set(CMAKE_OSX_DEPLOYMENT_TARGET ${MACOS_VERSION})
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    message(STATUS "Detected OS: FreeBSD")
    set(DEBUGGER "-ggdb3")
else()
    set(DEBUGGER "")
endif()

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Устанавливаем опции компилятора
set(CMAKE_CXX_COMPILER_ID "clang++")
message(STATUS "Using C compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\n")

# Информация и времени компиляции
string(TIMESTAMP CURRENT_YEAR "%Y")
string(TIMESTAMP CURRENT_TIME "%d.%m.%Y %H:%M:%S")

# Опции компилятора
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(ACU_PARAMS "-municode -Wno-cpp -Wno-deprecated")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(ACU_PARAMS "-Wno-cpp -Wno-deprecated")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(ACU_PARAMS "-framework Carbon -Wno-cpp -Wno-deprecated -Wno-unused-command-line-argument")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    set(ACU_PARAMS "-lutil -Wno-cpp -Wno-deprecated -Wno-unused-command-line-argument")
else()
    set(ACU_PARAMS "-Wno-unknown-attributes")
endif()

if(${ARCHITECTURE} MATCHES "aarch64" OR ${ARCHITECTURE} MATCHES "arm*")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-permissive -Wno-pedantic -Wno-narrowing -Wno-deprecated-declarations -Wno-exceptions -Wno-unused-result ${ACU_PARAMS}")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -mrdrnd -march=core2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-permissive -Wno-pedantic -Wno-narrowing -Wno-deprecated-declarations -Wno-exceptions -Wno-unused-result ${ACU_PARAMS}")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG_MODE ${DEBUGGER}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "C Flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}\n")
else()
    message(STATUS "C Flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\n")
endif()

# Если нужно собрать и динамическую библиотеку
if(CMAKE_SHARED_BUILD_LIB)
    if(MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif(MSVC)
endif(CMAKE_SHARED_BUILD_LIB)

# Разрешаем сборку IDN
set(CMAKE_BUILD_IDN ON)
add_compile_definitions(AWH_IDN)

# Добавляем модули cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Выполняем поиск нужных библиотек
find_package(AWH REQUIRED)
find_package(XML REQUIRED)
find_package(YAML REQUIRED)

# Провекра XML
if (XML_FOUND)
    message(STATUS "XML lib found: " ${XML_LIBRARY})
    message(STATUS "XML include dir found: " ${XML_INCLUDE_DIR})
else(XML_FOUND)
    message(FATAL_ERROR "XML not found")
endif(XML_FOUND)

# Провекра YAML
if (YAML_FOUND)
    message(STATUS "YAML lib found: " ${YAML_LIBRARY})
    message(STATUS "YAML include dir found: " ${YAML_INCLUDE_DIR})
else(YAML_FOUND)
    message(FATAL_ERROR "YAML not found")
endif(YAML_FOUND)

# Провекра AWH
if(AWH_FOUND)
    message(STATUS "AWH include dir found: " ${AWH_INCLUDE_DIR})
    message(STATUS "CITY include dir found: " ${CITY_INCLUDE_DIR})
    message(STATUS "JSON include dir found: " ${JSON_INCLUDE_DIR})
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
    if(CMAKE_BUILD_IDN AND (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
        message(STATUS "IDN2 include dir found: " ${IDN2_INCLUDE_DIR})
        message(STATUS "ICONV include dir found: " ${ICONV_INCLUDE_DIR})
    endif()
else(AWH_FOUND)
    message(FATAL_ERROR "Dependence not found")
endif(AWH_FOUND)

# Выполняем подключение хидеров
include_directories(SYSTEM ${XML_INCLUDE_DIR})
include_directories(SYSTEM ${AWH_INCLUDE_DIRS})
include_directories(SYSTEM ${YAML_INCLUDE_DIR})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/contrib/include)
include_directories(SYSTEM ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# Выполняем подключение исходников
set(SOURCE_FILES
    "${SOURCE_FILES}"
    "${CMAKE_SOURCE_DIR}/src/cef.cpp"
    "${CMAKE_SOURCE_DIR}/src/csv.cpp"
    "${CMAKE_SOURCE_DIR}/src/grok.cpp"
    "${CMAKE_SOURCE_DIR}/src/syslog.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser.cpp"
    "${CMAKE_SOURCE_DIR}/src/server.cpp"
)

# Задаем именя библиотеки и бинарника
set(SERVER_APP_NAME server)
set(ACU_LIB_NAME ${PROJECT_NAME})
set(ACU_APP_NAME ${PROJECT_NAME}_bin)

# Если нужно собрать и динамическую библиотеку
if(CMAKE_SHARED_BUILD_LIB)
    # Добавляем динамическую библиотеку
    add_library(${ACU_LIB_NAME} SHARED ${SOURCE_FILES})
    target_compile_definitions(${ACU_LIB_NAME} PRIVATE ACU_SHARED_LIBRARY_EXPORT)
# Если нужно собрать и статическую библиотеку
else(CMAKE_SHARED_BUILD_LIB)
    # Активируем сборку статической библиотеки
    add_compile_definitions(ACU_STATICLIB)
    # Добавляем статическу библиотеку
    add_library(${ACU_LIB_NAME} STATIC ${SOURCE_FILES})
endif(CMAKE_SHARED_BUILD_LIB)

# Если операционной системой является Windows
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # export MSYS="error_start:%WD%dumper.exe -d -c %1 %2"
    # Устанавливаем необходимые зависимости для MS Windows
    set(WINFLAGS -lws2_32 -lgdi32 -lcrypt32 -lIphlpapi -lbcrypt -flto=auto)

    # Устанавливаем иконку для windows под MinGW.
    set(RES_FILES "acu.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
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

# Если нужно собрать и динамическую библиотеку
if(CMAKE_SHARED_BUILD_LIB)
    # Если операционной системой является Windows
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

        # Выполняем сборку динамической библиотеки
        target_link_libraries(
            ${ACU_LIB_NAME}
            PUBLIC
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
            ${WINFLAGS}
            ${AWH_IDN}
        )

        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${WINFLAGS}
            ${AWH_IDN}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
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
        )
        
        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
        )

    endif()
# Если нужно собрать и статическую библиотеку
else(CMAKE_SHARED_BUILD_LIB)
    # Если операционной системой является Windows
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

        # Выполняем сборку приложения утилиты
        target_link_libraries(
            ${ACU_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
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
        )

        # Выполняем сборку приложения сервера
        target_link_libraries(
            ${SERVER_APP_NAME}
            ${ACU_LIB_NAME}
            ${XML_LIBRARY}
            ${YAML_LIBRARY}
            ${AWH_LIBRARIES}
        )

    endif()
endif(CMAKE_SHARED_BUILD_LIB)

# Активация формирования дампа ядра в реже отладки под MacOS X
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(entitlements_file "${CMAKE_SOURCE_DIR}/segv.entitlements")
        
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
    if(CMAKE_ENABLED_PVS)
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
    endif(CMAKE_ENABLED_PVS)
endif()

# Устанавливаем адрес установки, запрет установки в /usr/local
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local/")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
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

# Устанавливаем собранную библиотеку утилиты
install(TARGETS ${ACU_LIB_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

# Устанавливаем исполнительные файлы
install(TARGETS ${ACU_APP_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/${CMAKE_PROJECT_NAME}")
install(TARGETS ${SERVER_APP_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/${CMAKE_PROJECT_NAME}")
