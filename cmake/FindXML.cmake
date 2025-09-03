SET(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

# Если операцинная система относится к MS Windows
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif()

# Поиск пути к заголовочным файлам
find_path(XML_INCLUDE_DIR NAMES libxml/xmlmemory.h PATHS ${CMAKE_SOURCE_DIR}/third_party/include/libxml2 NO_DEFAULT_PATH)
# Поиск библиотеки CEF
find_library(XML_LIBRARY NAMES xml2 PATHS ${CMAKE_SOURCE_DIR}/third_party/lib NO_DEFAULT_PATH)

# Подключаем 'FindPackageHandle' для использования модуля поиска (find_package(<PackageName>))
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XML REQUIRED_VARS
    XML_LIBRARY
    XML_INCLUDE_DIR

    FAIL_MESSAGE "Missing XML. Run ./build_third_party.sh first"
)
