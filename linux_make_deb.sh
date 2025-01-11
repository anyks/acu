#!/usr/bin/env bash

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/build"

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
cd $ROOT

# Имя исполнительного файла
EXECUTABLE_FILE="$PACKAGE_NAME"
EXECUTABLE_FILE=$(find . -name "$EXECUTABLE_FILE" -type f) || exit 1

# Если исполнительный файл не найден, печатаем сообщение и и прекращаем работу
if [ "${EXECUTABLE_FILE}" = "" ]; then
	echo "Error: Executable file \"$PACKAGE_NAME\" is not found! Please build the project and run again!"
	exit 1
fi

# Задаем необходимые параметры
FINAL_PREFIX_DIRECTORY="usr"
EXECUTABLE_PATH="$FINAL_PREFIX_DIRECTORY/bin/"
WORK_PREFIX="$ROOT/pkg-${PACKAGE_NAME}"
PACKAGE_SOURCE_DIR="$ROOT/package/deb"
PACKAGE_DEST_DIR="$WORK_PREFIX/DEBIAN"
CONTROL_NAME="control"

# Получаем архитектуру машины
SYSTEM_ARCHITECTURE=$(dpkg --print-architecture)

# Получаем имя ОС
OS_NAME=$(cat /etc/os-release | grep VERSION_CODENAME)
OS_NAME=${OS_NAME##*=}

# Получаем версию библиотеки
get_lib_version () {
	local LIB_VER=$(dpkg -s $1 | grep '^Version: ')
	LIB_VER=${LIB_VER%-*}
	LIB_VER=${LIB_VER:9}

	echo "$LIB_VER"
}

# Получаем список нужных нам библиотек
DEPENDENCIES_LIST=$(cat "$PACKAGE_SOURCE_DIR/dependent_pkg")

DEPENDENCIES=""

# Проходим в цикле по списку и добавляем в зависимость
for pkg in $DEPENDENCIES_LIST; do
	echo "Check dependent package: ${pkg}"
	LIB_VER=$(get_lib_version ${pkg})
	# Если библиотека используется в данной ОС, то добавляем в зависимость пакета
	if ! [ "${LIB_VER}" = "" ]; then
		echo "Add dependent package: ${pkg}"
		if [ "$DEPENDENCIES" = "" ]; then
			DEPENDENCIES="${pkg} (>= ${LIB_VER})"
		else
			DEPENDENCIES="$DEPENDENCIES, ${pkg} (>= ${LIB_VER})"
		fi
	fi
done

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# Создаем директории для сборки deb пакета
mkdir -p "$PACKAGE_DEST_DIR" || exit 1
mkdir -p "$WORK_PREFIX/$EXECUTABLE_PATH" || exit 1

# Копируем в нее все необходимые файлы
# /DEBIAN/control
cp -ar "$PACKAGE_SOURCE_DIR/$CONTROL_NAME" "$PACKAGE_DEST_DIR" || exit 1
# /DEBIAN/copyright
cp "$PACKAGE_SOURCE_DIR/copyright" "$PACKAGE_DEST_DIR" || exit 1
# /DEBIAN/postinst
cp "$PACKAGE_SOURCE_DIR/postinst" "$PACKAGE_DEST_DIR" || exit 1
# /usr/bin/$PACKAGE_NAME
cp "$EXECUTABLE_FILE" "$WORK_PREFIX/$EXECUTABLE_PATH" || exit 1

# Определяем размер установаленного пакета
size=$(du -h -s -k "$WORK_PREFIX")
size=${size//[^0-9]/}

# Заполняем поля в файле control необходимыми значениями
sed -i "s/@version@/${VERSION}/g" "$PACKAGE_DEST_DIR/$CONTROL_NAME"
sed -i "s/@package@/${PACKAGE_NAME}/g" "$PACKAGE_DEST_DIR/$CONTROL_NAME"
sed -i "s/@architecture@/${SYSTEM_ARCHITECTURE}/g" "$PACKAGE_DEST_DIR/$CONTROL_NAME"
sed -i "s/@dependencies@/${DEPENDENCIES}/g" "$PACKAGE_DEST_DIR/$CONTROL_NAME"
sed -i "s/@size@/${size}/g" "$PACKAGE_DEST_DIR/$CONTROL_NAME"

# Подсчитываем контрольную сумму
pushd "$WORK_PREFIX"
hashdeep -c md5 -r -l $(ls -I "DEBIAN") > "DEBIAN/md5sums"
popd

# Создаем deb пакет
fakeroot dpkg-deb --build "$WORK_PREFIX" || exit 1

# Устанавливаем имя deb пакета foo_VVV-RRR_AAA.deb
if [ "${OS_NAME}" = "" ]; then
	deb_name="${PACKAGE_NAME}_${VERSION}-1_${SYSTEM_ARCHITECTURE}.deb"
else
	deb_name="${PACKAGE_NAME}_${VERSION}-1~${OS_NAME}_${SYSTEM_ARCHITECTURE}.deb"
fi
mv "${WORK_PREFIX}.deb" "$deb_name" || exit 1

# Очищаем сборочную директорию
rm -rf "$WORK_PREFIX"
echo "Successfully created package $deb_name"
