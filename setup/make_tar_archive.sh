#!/usr/bin/env bash

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Сборочная дирректория
readonly BUILD_DIR="$ROOT/../${PACKAGE_NAME}_build"

# Очистка сборочной директории
if [ -d "$ROOT/../build" ]; then
	rm -rf "$ROOT/../build" || exit 1
fi

# Собираем приложение
mkdir "$ROOT/../build" || exit 1
cd "$ROOT/../build" || exit 1

# Выполняем сборку приложения
cmake \
 -DCMAKE_BUILD_TYPE=Release \
 .. || exit 1

cmake --build . || exit 1

# Переходим в корневой каталог обратно
cd $ROOT/../

# Имя исполнительного файла
EXECUTABLE_FILE="$PACKAGE_NAME"
EXECUTABLE_FILE=$(find . -name "$EXECUTABLE_FILE" -type f) || exit 1

# Если исполнительный файл не найден, печатаем сообщение и и прекращаем работу
if [ "${EXECUTABLE_FILE}" = "" ]; then
	echo "Error: Executable file \"$PACKAGE_NAME\" is not found! Please build the project and run again!"
	exit 1
fi

# Получаем архитектуру машины 
SYSTEM_ARCHITECTURE=$(uname -m)
if [ "${SYSTEM_ARCHITECTURE}" = "x86_64" ]; then
	SYSTEM_ARCHITECTURE="amd64"
fi

# Получаем полное имя ОС
if [ "$OS" = "FreeBSD" ]; then
	OS_NAME="FreeBSD"
elif [ "$OS" = "SunOS" ]; then
	OS_NAME="Solaris"
elif [ "$OS" = "Darwin" ]; then
	OS_NAME="MacOSX"
elif [ "$OS" = "Linux" ]; then
	OS_NAME=$(cat /etc/os-release | grep "^NAME")
	OS_NAME=${OS_NAME##*=}
	OS_NAME=`echo $OS_NAME | awk '{print $1}'`
	VERSION_ID=$(cat /etc/os-release | grep VERSION_ID)
	VERSION_ID=${VERSION_ID##*=}
	OS_NAME="${OS_NAME}${VERSION_ID}"
	OS_NAME=`echo "${OS_NAME//\"}"`
# Для всех остальных операционных систем
else
	# Выводим сообщение об ошибке
	echo "Operating system not supported"
	exit 1
fi

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# Удаляем ранее собранный каталог
rm -rf $BUILD_DIR || exit 1
# Создаём сборочный каталог
mkdir -p $BUILD_DIR || exit 1

# Если операционная система FreeBSD
if [ "$OS" = "FreeBSD" ]; then
	# Выполняем создание каталога
	mkdir -p "$BUILD_DIR/usr/local/bin" || exit 1
	# Выполняем копирование приложений
	cp "$EXECUTABLE_FILE" "$BUILD_DIR/usr/local/bin/" || exit 1
# Если операционная система Solaris
elif [ "$OS" = "SunOS" ]; then
	# Выполняем создание каталога
	mkdir -p "$BUILD_DIR/opt/$PACKAGE_NAME/bin" || exit 1
	# Выполняем копирование приложений
	cp "$EXECUTABLE_FILE" "$BUILD_DIR/opt/$PACKAGE_NAME/bin/" || exit 1
# Если операционная система MacOS X
elif [ "$OS" = "Darwin" ]; then
	# Выполняем создание каталога
	mkdir -p "$BUILD_DIR/usr/local/bin" || exit 1
	# Выполняем копирование приложений
	cp "$EXECUTABLE_FILE" "$BUILD_DIR/usr/local/bin/" || exit 1
# Если операционная система Linux
elif [ "$OS" = "Linux" ]; then
	# Выполняем создание каталога
	mkdir -p "$BUILD_DIR/usr/bin" || exit 1
	# Выполняем копирование приложений
	cp "$EXECUTABLE_FILE" "$BUILD_DIR/usr/bin/" || exit 1
# Для всех остальных операционных систем
else
	# Выводим сообщение об ошибке
	echo "Operating system not supported"
	exit 1
fi

# Заходим в каталог
cd $BUILD_DIR || exit 1
# Создаем имя архива
ARCH_NAME="${PACKAGE_NAME}_${VERSION}_${OS_NAME}_${SYSTEM_ARCHITECTURE}.tar.gz"
# Выполняем создание архива
tar -czf ../$ARCH_NAME . || exit 1

# Удаляем ранее собранный каталог
rm -rf $BUILD_DIR || exit 1

# Выводим сообщение о результате
printf "\n****************************************"
printf "\n************   Success!!!   ************"
printf "\n****************************************"
printf "\n\n\n"
