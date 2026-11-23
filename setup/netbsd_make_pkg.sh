#!/usr/bin/env bash

##
 # Сборка двоичного пакета NetBSD (формат pkgsrc, устанавливается командой pkg_add)
 #
 # Пакет раскладывается в /usr/pkg - там же, где живут все пакеты pkgsrc. Зависимости
 # (third_party) должны быть собраны заранее: bash build_third_party.sh
 #
 # Установка: pkg_add ./acu-<версия>_NetBSD_amd64.tgz
 ##

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Префикс установки пакетов pkgsrc
readonly PREFIX="/usr/pkg"

# Утилиты пакетов лежат в /usr/sbin, которого нет в PATH сеанса ssh
readonly PKG_CREATE="/usr/sbin/pkg_create"

# Адрес электронной почты
readonly PACKAGE_EMAIL="info@anyks.com"
# URL-адрес сайта
readonly PACKAGE_URL="https://acu.anyks.com"
# Краткое описание приложения
readonly PACKAGE_SUMMARY="ANYKS - conversion utility"
# Описание приложения
readonly PACKAGE_DESCRIPTION="ACU - console data conversion utility: base64, JSON, XML, CSV, YAML, TOML, timestamps, currencies and GROK patterns. Distributed under the GNU GPL v3."

# Определяем является ли операционная система NetBSD
if ! [ "$OS" = "NetBSD" ]; then
	echo "Error: Only for NetBSD"
	exit 1
fi

# Архитектура машины: для amd64 NetBSD называет её сам, а на ARM `uname -m` отдаёт имя
# порта платы (evbarm -- и 32-, и 64-разрядный), набор команд же несёт `uname -p`
SYSTEM_ARCHITECTURE=$(uname -m)
case "$SYSTEM_ARCHITECTURE" in
	evb*) SYSTEM_ARCHITECTURE=$(uname -p);;
esac

# Адрес каталога с рабочим приложением
readonly APP_DIR="$ROOT/../${PACKAGE_NAME}_app"
# Сборочная дирректория
readonly BUILD_DIR="$ROOT/../build"
# Каталог служебных файлов пакета
readonly META_DIR="$ROOT/../${PACKAGE_NAME}_meta"
# Каталог раскладки файлов пакета
readonly STAGE="$APP_DIR$PREFIX"

# Очистка каталогов прошлой сборки
rm -rf "$APP_DIR" "$META_DIR" || exit 1
if [ -d "$BUILD_DIR" ]; then
	rm -rf "$BUILD_DIR" || exit 1
fi

# Выполняем создание каталогов
mkdir -p "$STAGE/bin" || exit 1
mkdir -p "$STAGE/share/doc/$PACKAGE_NAME" || exit 1
mkdir -p "$META_DIR" || exit 1

# Собираем приложение
mkdir "$BUILD_DIR" || exit 1
cd "$BUILD_DIR" || exit 1

# Выполняем сборку приложения
cmake \
 -DCMAKE_BUILD_TYPE=Release \
 .. || exit 1

cmake --build . -j "$(sysctl -n hw.ncpu)" || exit 1

# Копируем само приложение и лицензию
cp "$BUILD_DIR/$PACKAGE_NAME" "$STAGE/bin"/ || exit 1
cp "$ROOT/../LICENSE" "$STAGE/share/doc/$PACKAGE_NAME"/ || exit 1

# Переходим в корневой каталог обратно
cd "$ROOT/../" || exit 1

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$("$STAGE/bin/$PACKAGE_NAME" -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=`echo $VERSION | awk '{print $1}'`

# Выставляем права доступа
find "$STAGE" -type d -exec chmod 755 {} \;
find "$STAGE" -type f -exec chmod 644 {} \;
chmod 755 "$STAGE/bin/$PACKAGE_NAME"

# Краткое описание пакета
echo "$PACKAGE_SUMMARY" > "$META_DIR/comment"
# Полное описание пакета
printf "%s\n\nLicense: gnu-gpl-v3, see %s/share/doc/%s/LICENSE\nHomepage: %s\n" "$PACKAGE_DESCRIPTION" "$PREFIX" "$PACKAGE_NAME" "$PACKAGE_URL" > "$META_DIR/desc"

##
 # Сведения о сборке
 #
 # pkg_add сверяет OPSYS, OS_VERSION и MACHINE_ARCH с системой и отказывает в установке
 # пакета чужой системы. LICENSE - имя лицензии в понимании pkgsrc: при CHECK_LICENSE=yes
 # в pkg_install.conf пакет встанет, лишь если имя это внесено в ACCEPTABLE_LICENSES
 ##
cat > "$META_DIR/build-info" <<EOF
OPSYS=NetBSD
OS_VERSION=$(uname -r)
MACHINE_ARCH=$(uname -p)
PKGTOOLS_VERSION=$(/usr/sbin/pkg_admin -V)
LICENSE=gnu-gpl-v3
HOMEPAGE=$PACKAGE_URL
MAINTAINER=$PACKAGE_EMAIL
EOF

# Сообщение, выводимое при установке пакета
cat > "$META_DIR/display" <<EOF
===========================================================================
$PACKAGE_NAME $VERSION is distributed under the GNU General Public
License v3. The full text of the license:

    $PREFIX/share/doc/$PACKAGE_NAME/LICENSE
===========================================================================
EOF

# Перечень файлов пакета (опустевшие каталоги pkg_delete удаляет сам)
{
	echo "@name $PACKAGE_NAME-$VERSION"
	echo "@cwd $PREFIX"
	(cd "$STAGE" && find . -type f | sed 's!^\./!!' | sort)
} > "$META_DIR/plist"

# Создаем имя архива
readonly PKG_FILE="$ROOT/../$PACKAGE_NAME-${VERSION}_NetBSD_${SYSTEM_ARCHITECTURE}.tgz"
# Удаляем прежний пакет
rm -f "$PKG_FILE"

# Выполняем сборку пакета
"$PKG_CREATE" \
 -B "$META_DIR/build-info" \
 -c "$META_DIR/comment" \
 -d "$META_DIR/desc" \
 -D "$META_DIR/display" \
 -f "$META_DIR/plist" \
 -I "$PREFIX" \
 -p "$STAGE" \
 -u root -g wheel \
 "$PKG_FILE" || exit 1

# Удаляем временные каталоги
rm -rf "$APP_DIR" "$META_DIR"

# Выводим сообщение об удачной сборке
echo "Successfully created package $(basename "$PKG_FILE")"
echo "To install the application, please perform a:"
echo ""
echo "$ sudo pkg_add ./$(basename "$PKG_FILE")"
echo ""
