#!/usr/bin/env bash

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/../build"

# Определяем является ли операционная система MacOS X
if ! [ $OS = "Darwin" ]; then
	echo "Error: Only for MacOS X"
	exit 1
fi

# Устанавливаем флаги глобального использования
# export CPPFLAGS=""
export CXXFLAGS="$CXXFLAGS -fPIC -mmacosx-version-min=10.15"
export CFLAGS="-I$PREFIX/include -fPIC -mmacosx-version-min=10.15"

# Очистка сборочной директории
if [ -d $BUILD_DIR ]; then
 	rm -rf $BUILD_DIR || exit 1
fi

# Собираем приложение
mkdir $BUILD_DIR || exit 1
cd $BUILD_DIR || exit 1

# Выполняем сборку приложения
cmake \
 -DCMAKE_BUILD_TYPE=Release \
 .. || exit 1

cmake --build . || exit 1

# Имя исполнительного файла
EXECUTABLE_FILE="$PACKAGE_NAME"
EXECUTABLE_FILE=$(find . -name "$EXECUTABLE_FILE" -type f) || exit 1

# Если исполнительный файл не найден, печатаем сообщение и и прекращаем работу
if [ "${EXECUTABLE_FILE}" = "" ]; then
	echo "Error: Executable file \"$PACKAGE_NAME\" is not found! Please build the project and run again!"
	exit 1
fi

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# ID сертификата для подписи
DEV_APP_ID_SIGN=""

# Если идентификатор цифровой подписи передан
if [ -n "$1" ]; then
	DEV_APP_ID_SIGN="$1"
fi

# Адрес каталога с рабочим приложением
APP_DIR="$ROOT/../package/MacOS/application"

# Очистка рабочей директории приложения
if [ -d $APP_DIR ]; then
	rm -rf $APP_DIR || exit 1
fi

# Создаём адрес рабочего каталога
mkdir -p $APP_DIR || exit 1
mkdir -p "$APP_DIR/bin" || exit 1

# Копирование файлов для создание pkg пакета
cp "$BUILD_DIR/$EXECUTABLE_FILE" "$APP_DIR/bin/$PACKAGE_NAME" || exit 1

# Выставляем права доступа на каталог
find $APP_DIR -type d ! -perm 755 -exec chmod 755 {} \;
# Выставляем права доступа на файлы
find $APP_DIR -type f ! -perm 644 -exec chmod 644 {} \;
# Выставляем права доступа на исполняемые файлы
find "$APP_DIR/bin" -type f ! -perm 755 -exec chmod 755 {} \;

# Переходим в рабочий каталог
cd $APP_DIR || exit 1

# Подписываем бинарные файлы
function signProduct() {
    codesign \
     --deep \
     --force \
     --all-architectures \
     --timestamp \
     --options=runtime \
     --sign "${DEV_APP_ID_SIGN}" \
     "$1" 

    codesign -dv --verbose=4 "$1"
}

# Если идентификатор цифровой подписи передан
if [ -n "$DEV_APP_ID_SIGN" ]; then
	# Выполняем подпись цифровым ключом наши бинарники
	signProduct "$APP_DIR/bin/$PACKAGE_NAME"
fi

bash "$ROOT/../package/MacOS/build.sh" $VERSION || exit 1

# echo "Pkg path: $ROOT/../package/MacOS/target/pkg-signed"

# Удаляем временный каталог
rm -rf $APP_DIR

printf "\n****************************************"
printf "\n************   Success!!!   ************"
printf "\n****************************************"
printf "\n\n\n"
