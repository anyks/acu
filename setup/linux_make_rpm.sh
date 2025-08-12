#!/usr/bin/env bash

# Имя пакета (не итоговое)
PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/../build"

# Получаем текущее значение даты
readonly CURRENT_DATE=$(LC_ALL=en_US.UTF-8 date +"%a %h %d %Y")

# Адрес электронной почты
readonly PACKAGE_EMAIL="info@anyks.com"
# URL-адрес сайта
readonly PACKAGE_URL="https://acu.anyks.com"
# Название продавца
readonly PACKAGE_DISTRIBUTION="ANYKS LLC"
# Название приложения
readonly PACKAGE_SMMARY="ANYKS - conversion utility (ACU)"
# Описание приложения
readonly PACKAGE_DESCRIPTION="Cross-platform utility for converting text formats - convert utility (ACU)."

# Выполняем получение номера релиза
RELEASE_NUMBER="$1"
# Если номер релиза не установлен
if [ ! -n "$RELEASE_NUMBER" ]; then
	# Устанавливаем номер релиза по умолчанию
	RELEASE_NUMBER="1"
fi

# Определяем является ли операционная система Linux
if ! [ $OS = "Linux" ]; then
	echo "Error: Only for Linux"
	exit 1
fi

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
mkdir -p $WORK_PREFIX/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Копируем необходимые файлы
cp "$PACKAGE_SOURCE_DIR/$SPEC_NAME" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"

# Заполняем поля в файле CONTROL необходимыми значениями
sed -i "s!@url@!${PACKAGE_URL}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@name@!${PACKAGE_NAME}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@version@!${VERSION}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@date@!${CURRENT_DATE}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@email@!${PACKAGE_EMAIL}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@summary@!${PACKAGE_SMMARY}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@release_number@!${RELEASE_NUMBER}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@description@!${PACKAGE_DESCRIPTION}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@architecture@!${SYSTEM_ARCHITECTURE}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@distribution@!${PACKAGE_DISTRIBUTION}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"

# Заполняем поля в файле control необходимыми значениями
sed -i "s!@work_path@!${WORK_PREFIX}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"
sed -i "s!@executable_file@!${EXECUTABLE_FILE}!g" "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp"

# Создаем RPM пакет
rpmbuild --buildroot="$WORK_PREFIX" --target="$SYSTEM_ARCHITECTURE" --define='noclean 1' --rmspec "$PACKAGE_SOURCE_DIR/$SPEC_NAME.tmp" -bb || exit 1

# Получаем название операционной системы
OS_NAME=$(cat /etc/os-release | grep "^NAME")
OS_NAME=${OS_NAME##*=}
OS_NAME=`echo $OS_NAME | awk '{print $1}'`
VERSION_ID=$(cat /etc/os-release | grep VERSION_ID)
VERSION_ID=${VERSION_ID##*=}
OS_NAME="${OS_NAME}${VERSION_ID}"
OS_NAME=`echo "${OS_NAME//\"}"`

# Выполняем поиск созданного RPM архива
readonly PACKAGE="$ROOT/../${PACKAGE_NAME}-${VERSION}-${RELEASE_NUMBER}.${SYSTEM_ARCHITECTURE}.rpm"

# Компенсируем название архитектуры процессора
if [ "${SYSTEM_ARCHITECTURE}" = "x86_64" ]; then
	SYSTEM_ARCHITECTURE="amd64"
fi

# Если название операционной системы получено
if [ "${OS_NAME}" = "" ]; then
	mv $PACKAGE "$ROOT/../${PACKAGE_NAME}-${VERSION}-${RELEASE_NUMBER}.${SYSTEM_ARCHITECTURE}.rpm"
# Если название операционной системы не получено
else
	mv $PACKAGE "$ROOT/../${PACKAGE_NAME}-${VERSION}-${RELEASE_NUMBER}.${OS_NAME}_${SYSTEM_ARCHITECTURE}.rpm"
fi

# Очищаем сборочную директорию
rm -rf "$WORK_PREFIX"

# Выводим сообщение об удачной сборке
echo "Successfully created rpm package"
