SET(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

# Если операцинная система относится к MS Windows
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif()

# Поиск пути к заголовочным файлам
find_path(YAML_INCLUDE_DIR NAMES yaml.h PATHS ${CMAKE_SOURCE_DIR}/third_party/include/yaml-cpp NO_DEFAULT_PATH)
# Поиск библиотеки YAML
find_library(YAML_LIBRARY NAMES yaml-cpp PATHS ${CMAKE_SOURCE_DIR}/third_party/lib NO_DEFAULT_PATH)

# Подключаем 'FindPackageHandle' для использования модуля поиска (find_package(<PackageName>))
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YAML REQUIRED_VARS
    YAML_LIBRARY
    YAML_INCLUDE_DIR

    FAIL_MESSAGE "Missing YAML. Run ./build_third_party.sh first"
)
