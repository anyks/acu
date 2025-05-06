#!/usr/bin/env bash

# Имя пакета (не итоговое)
PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

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

# Устанавливаем запускаемые приложения
EXECUTABLE_FILE="$ROOT/../$EXECUTABLE_FILE"

# Задаём параметры сборки
SPEC_NAME="${PACKAGE_NAME}.spec"
PACKAGE_SOURCE_DIR="$ROOT/../package/Linux/rpm"
# Задаём адрес сборочной дирректории
WORK_PREFIX="$ROOT/../pkg-$PACKAGE_NAME"

# Получаем архитектуру машины
SYSTEM_ARCHITECTURE=$(uname -m)

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# Создаем необходимые каталоги
mkdir -p "$WORK_PREFIX/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}"

# Копируем необходимые файлы
cp "$PACKAGE_SOURCE_DIR/$SPEC_NAME" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"

# Заполняем поля в файле control необходимыми значениями
sed -i "s!@package_name@!${PACKAGE_NAME}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@work_path@!${WORK_PREFIX}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@name@!${PACKAGE_NAME}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@version@!${VERSION}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@architecture@!${SYSTEM_ARCHITECTURE}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@executable_file@!${EXECUTABLE_FILE}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"

if [ "${SYSTEM_ARCHITECTURE}" = "x86_64" ]; then
	SYSTEM_ARCHITECTURE="amd64"
fi

# Создаем RPM пакет
rpmbuild --buildroot="$WORK_PREFIX" --target="$SYSTEM_ARCHITECTURE" --define='noclean 1' --rmspec "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp" -bb || exit 1

PACKAGE=$(find $WORK_PREFIX -name "${PACKAGE_NAME}*.rpm")
cp $PACKAGE "$ROOT/../${PACKAGE_NAME}-${VERSION}-1.${SYSTEM_ARCHITECTURE}.rpm"

# Очищаем сборочную директорию
rm -rf "$WORK_PREFIX"

# Выводим сообщение об удачной сборке
echo "Successfully created rpm package"
