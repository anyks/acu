#!/usr/bin/env bash

##
 # Документация по сборке установочного пакета
 # https://docs.oracle.com/cd/E37838_01/html/E61051/pkgcreate.html
 #

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"
# Название приложения
readonly PACKAGE_SMMARY="ANYKS - conversion utility (ACU)"
# Описание приложения
readonly PACKAGE_DESCRIPTION="Cross-platform utility for converting text formats - convert utility (ACU)."

# Автор публикуемого проекта
readonly PACKAGE_PUBLISHER="anyks"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/../build"

# Определяем является ли операционная система Sularis
if ! [ $OS = "SunOS" ]; then
	echo "Error: Only for Sularis"
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

# Задаем необходимые параметры
FINAL_PREFIX_DIRECTORY="opt/$PACKAGE_NAME"
EXECUTABLE_PATH="$FINAL_PREFIX_DIRECTORY/bin"
WORK_PREFIX="$ROOT/../pkg-${PACKAGE_NAME}"
MANIFEST_PREFIX="$ROOT/../manifest-${PACKAGE_NAME}"

# Получаем архитектуру машины
SYSTEM_ARCHITECTURE=$(uname -m)

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$($EXECUTABLE_FILE -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`
# Получаем мажёрное значение версии
VERSION_P=$(echo $VERSION | awk -F '\\.' '{print $1}')
# Получаем минорное значение версии
VERSION_u=$(echo $VERSION | awk -F '\\.' '{print $2}')
# Получаем релизное значение версии
VERSION_r=$(echo $VERSION | awk -F '\\.' '{print $3}')

# Создаём название пакета установки
readonly PKG_NAME="${PACKAGE_NAME}_${VERSION}-1_${SYSTEM_ARCHITECTURE}.p5p"

# Выполняем удаление директории для сборки пакета
if [ -d $WORK_PREFIX ]; then
	rm -rf $WORK_PREFIX || exit 1
fi

# Выполняем удаление директории для хранения манифестов
if [ -d $MANIFEST_PREFIX ]; then
	rm -rf $MANIFEST_PREFIX || exit 1
fi

# Создаем директории для хранения манифестов
mkdir -p "$MANIFEST_PREFIX" || exit 1
# Создаем директории для сборки пакета
mkdir -p "$WORK_PREFIX/$EXECUTABLE_PATH" || exit 1

# Копируем в нее все необходимые файлы
# /opt/acu/bin/$PACKAGE_NAME
cp "$EXECUTABLE_FILE" "$WORK_PREFIX/$EXECUTABLE_PATH"/ || exit 1

# Генерируем Manifest файл
pkgsend generate $WORK_PREFIX | pkgfmt > $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.1

# Создаём файл информационных данных
touch $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем версию приложения
echo "set name=pkg.fmri value=$PACKAGE_NAME@$VERSION" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем название приложения
echo "set name=pkg.summary value=\"$PACKAGE_SMMARY\"" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем описание приложения
echo "set name=pkg.description value=\"$PACKAGE_DESCRIPTION\"" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Добавляем человекочитаемую значение версии приложения
echo "set name=pkg.human-version value=\"P$VERSION_P-u$VERSION_u-r$VERSION_r\"" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем название архитектуры процессора
echo "set name=variant.arch value=\$(ARCH)" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем категорию размещения приложения согласно файлу ($ cat /usr/share/lib/pkg/opensolaris.org.sections)
echo "set name=info.classification value=\"org.opensolaris.category.2008:Applications/System Utilities\"" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем генерируемый симлинк
echo "link path=usr/bin/$PACKAGE_NAME target=/$EXECUTABLE_PATH/$PACKAGE_NAME" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog
# Формируем правила сборки
echo "<transform dir path=opt\$->drop>" >> $MANIFEST_PREFIX/$PACKAGE_NAME.mog

# Комбинируем наш манифест с информационным файлом
pkgmogrify -DARCH=`uname -p` $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.1 $MANIFEST_PREFIX/$PACKAGE_NAME.mog | pkgfmt > $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.2
# Выполняем генерацию итогового значения манифеста
pkgdepend generate -md $WORK_PREFIX $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.2 | pkgfmt > $MANIFEST_PREFIX/$PACKAGE_NAME.p5m

# Выполняем резолвинг сгенерированного манифеста
pkgdepend resolve -m $MANIFEST_PREFIX/$PACKAGE_NAME.p5m

# Проверяем соответствие зависимостям
pkglint $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.res
# Проверяем соответствие зависимостей в репозитории
pkglint -c $MANIFEST_PREFIX/solaris-reference -r http://pkg.oracle.com/solaris/release $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.res
# Перепроверяем результат с учётом кэша
pkglint -c $MANIFEST_PREFIX/solaris-reference $MANIFEST_PREFIX/$PACKAGE_NAME.p5m.res

# Выполняем создание репозитория под проект
pkgrepo create $MANIFEST_PREFIX/$PACKAGE_NAME-repository
# Проверяем наличие созданного репозитория
ls $MANIFEST_PREFIX/$PACKAGE_NAME-repository

# Переходим в каталог с репозиторием
cd $MANIFEST_PREFIX

# Выполняем установку автора репозитория
pkgrepo -s $PACKAGE_NAME-repository set publisher/prefix="$PACKAGE_PUBLISHER"
# Выполняем побликацию проекта в репозитории
pkgsend -s $PACKAGE_NAME-repository publish -d $WORK_PREFIX $PACKAGE_NAME.p5m.res

# Выполняем проверку созданного репозитория
pkgrepo verify -s $PACKAGE_NAME-repository
# Выводим информацию о репозитории
pkgrepo info -s $PACKAGE_NAME-repository
# Выводим список пакетов в репозитории
pkgrepo list -s $PACKAGE_NAME-repository
# Выводим проверку всего репозитория
pkg list -afv -g $PACKAGE_NAME-repository

# Выполняем подпись репозитория
pkgsign -s $PACKAGE_NAME-repository -a sha256 '*'

# Выполняем генерацию пакета установки
pkgrecv -s $PACKAGE_NAME-repository -a -d $ROOT/../$PKG_NAME $PACKAGE_NAME

# Переходим в корневой каталог обратно
cd $ROOT/../

# Выполняем удаление директории для сборки пакета
rm -rf $WORK_PREFIX
# Выполняем удаление директории для хранения манифестов
rm -rf $MANIFEST_PREFIX

# Выводим сообщение об удачной сборке
echo "Successfully created package $PKG_NAME"
echo "To install the application, please perform a:"
echo ""
echo "$ sudo pkg install -g $PKG_NAME $PACKAGE_NAME"
echo ""
