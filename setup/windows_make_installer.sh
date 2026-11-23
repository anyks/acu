#!/usr/bin/env bash

# Имя пакета (не итоговое)
PACKAGE_NAME="acu.exe"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
OS=$(uname -a | awk '{print $1}')

# Окружения MSYS2: MINGW64 и UCRT64 (gcc x86-64), CLANG64 (clang x86-64), CLANGARM64 (clang ARM64)
if [[ $OS =~ ^(MINGW64|UCRT64|CLANG64|CLANGARM64)_NT ]]; then
	OS="Windows"
fi

# Определяем является ли операционная система Microsoft Windows
if ! [ $OS = "Windows" ]; then
	echo "Error: Only for Microsoft Windows"
	exit 1
fi

# Удаляем директории если существуют
rm -rf "$ROOT/../win"
rm -rf "$ROOT/../installer"

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/../build"

# Очистка сборочной директории
if [ -d $BUILD_DIR ]; then
	rm -rf $BUILD_DIR || exit 1
fi

# Собираем приложение
mkdir $BUILD_DIR || exit 1
cd $BUILD_DIR || exit 1

# Выполняем сборку приложения
cmake \
 -G "MSYS Makefiles" \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_SYSTEM_NAME=Windows \
 ..

cmake --build . || exit 1

# Переходим в корневой каталог обратно
cd $ROOT/../

# Имя исполнительного файла
EXECUTABLE_FILE=$PACKAGE_NAME
EXECUTABLE_FILE=$(find . -name "$EXECUTABLE_FILE" -type f) || exit 1

# Если исполнительный файл не найден, печатаем сообщение и и прекращаем работу
if [ "${EXECUTABLE_FILE}" = "" ]; then
	echo "Error: Executable file not found! Please build the project and run again!"
	exit 1
fi

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# Создаем директории для сборки инсталятора
mkdir -p "$ROOT/../win" || exit 1
mkdir -p "$ROOT/../installer" || exit 1

# Копируем все необходимые файлы
cp -ar $EXECUTABLE_FILE "$ROOT/../win" || exit 1
cp -ar "$ROOT/../icons/icon.ico" "$ROOT/../win" || exit 1
cp -ar "$ROOT/../icons/setup.ico" "$ROOT/../win" || exit 1
cp -ar "$ROOT/../package/Windows/install.iss" "$ROOT/../win" || exit 1

##
 # Собираем библиотеки времени выполнения
 #
 # У gcc это libstdc++, libgcc_s и libwinpthread, у clang - libc++ и libunwind, и на ARM64
 # их имена отличаются от x86-64. Поэтому перечень не пишется руками, а снимается с самого
 # приложения: в установщик берутся только библиотеки окружения MSYS2 (/mingw64, /clangarm64),
 # системные (kernel32 и прочие) установщик не трогает
 ##
mkdir -p "$ROOT/../win/runtime" || exit 1
for i in $(ldd "$EXECUTABLE_FILE" | awk '{print $3}' | sort -u); do
	if [[ $i == "$MINGW_PREFIX/bin/"* ]]; then
		cp -a "$i" "$ROOT/../win/runtime"/ || exit 1
	fi
done

# Архитектура установщика для Inno Setup и суффикс имени файла: x86-64 без суффикса, как прежде
if [ "$MSYSTEM_CARCH" = "aarch64" ]; then
	ARCH="arm64"
	SUFFIX="-arm64"
else
	ARCH="x64compatible"
	SUFFIX=""
fi


# Заполняем поля шаблона для создания конфига инсталятора
SOURCE_DIR=`echo $(cygpath -wm "$ROOT/../win")`
INSTALLER_DIR=`echo $(cygpath -wm "$ROOT/../installer")`

SOURCE_DIR=$(echo $SOURCE_DIR | sed -e 's|/|\\\\|g')
INSTALLER_DIR=$(echo $INSTALLER_DIR | sed -e 's|/|\\\\|g')

sed -i "s/@version@/${VERSION}/g" $ROOT/../win/install.iss
sed -i "s|@pwd@|${SOURCE_DIR}|g" $ROOT/../win/install.iss
sed -i "s|@name@|${PACKAGE_NAME}|g" $ROOT/../win/install.iss
sed -i "s|@installerDir@|${INSTALLER_DIR}|g" $ROOT/../win/install.iss
sed -i "s|@arch@|${ARCH}|g" $ROOT/../win/install.iss
sed -i "s|@suffix@|${SUFFIX}|g" $ROOT/../win/install.iss
