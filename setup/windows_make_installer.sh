#!/usr/bin/env bash

# Имя пакета (не итоговое)
PACKAGE_NAME="acu.exe"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

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
cp -ar "/mingw64/bin/libgcc_s_seh-1.dll" "$ROOT/../win" || exit 1
cp -ar "/mingw64/bin/libwinpthread-1.dll" "$ROOT/../win" || exit 1
cp -ar "/mingw64/bin/libstdc++-6.dll" "$ROOT/../win" || exit 1
cp -ar "$ROOT/../package/win/install.iss" "$ROOT/../win" || exit 1


# Заполняем поля шаблона для создания конфига инсталятора
SOURCE_DIR=`echo $(cygpath -wm "$ROOT/../win")`
INSTALLER_DIR=`echo $(cygpath -wm "$ROOT/../installer")`

SOURCE_DIR=$(echo $SOURCE_DIR | sed -e 's|/|\\\\|g')
INSTALLER_DIR=$(echo $INSTALLER_DIR | sed -e 's|/|\\\\|g')

sed -i "s/@version@/${VERSION}/g" $ROOT/../win/install.iss
sed -i "s|@pwd@|${SOURCE_DIR}|g" $ROOT/../win/install.iss
sed -i "s|@name@|${PACKAGE_NAME}|g" $ROOT/../win/install.iss
sed -i "s|@installerDir@|${INSTALLER_DIR}|g" $ROOT/../win/install.iss
