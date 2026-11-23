#!/usr/bin/env bash

##
 # Сборка двоичного пакета OpenBSD (упаковка pkg, устанавливается командой pkg_add)
 #
 # Пакет раскладывается в /usr/local. Зависимости (third_party) должны быть собраны
 # заранее: bash build_third_party.sh
 #
 # Пакет не подписан: pkg_add ставит его лишь с ключом -D unsigned
 #
 # Установка: pkg_add -D unsigned ./acu-<версия>_OpenBSD_amd64.tgz
 ##

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Префикс установки пакетов
readonly PREFIX="/usr/local"

# Адрес электронной почты
readonly PACKAGE_EMAIL="info@anyks.com"
# URL-адрес сайта
readonly PACKAGE_URL="https://acu.anyks.com"
# Краткое описание приложения
readonly PACKAGE_SUMMARY="ANYKS - conversion utility"
# Описание приложения
readonly PACKAGE_DESCRIPTION="ACU - console data conversion utility: base64, JSON, XML, CSV, YAML, TOML, timestamps, currencies and GROK patterns. Distributed under the GNU GPL v3."

# Определяем является ли операционная система OpenBSD
if ! [ "$OS" = "OpenBSD" ]; then
	echo "Error: Only for OpenBSD"
	exit 1
fi

# Утилиты пакетов лежат в /usr/sbin, которого нет в PATH сеанса ssh
readonly PKG_CREATE="/usr/sbin/pkg_create"

# Архитектура машины: OpenBSD сам называет её amd64 или arm64
SYSTEM_ARCHITECTURE=$(uname -m)

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

# Полное описание пакета
printf "%s\n\nLicense: GPL-3.0, see %s/share/doc/%s/LICENSE\nHomepage: %s\n" "$PACKAGE_DESCRIPTION" "$PREFIX" "$PACKAGE_NAME" "$PACKAGE_URL" > "$META_DIR/desc"

# Сообщение, выводимое при установке пакета
cat > "$META_DIR/display" <<EOF
===========================================================================
$PACKAGE_NAME $VERSION is distributed under the GNU General Public
License v3. The full text of the license:

    $PREFIX/share/doc/$PACKAGE_NAME/LICENSE
===========================================================================
EOF

##
 # Перечень файлов пакета
 #
 # Приложение помечается @bin, свои каталоги пишутся с косой чертой на конце и удаляются
 # вместе с пакетом
 ##
{
	echo "@comment \$OpenBSD\$"
	echo "@bin bin/$PACKAGE_NAME"
	(cd "$STAGE" && find "share/doc/$PACKAGE_NAME" | sort | while read -r f; do
		if [ -d "$f" ]; then echo "$f/"; else echo "$f"; fi
	done)
} > "$META_DIR/plist"

# Создаем имя архива
readonly PKG_FILE="$ROOT/../$PACKAGE_NAME-${VERSION}_OpenBSD_${SYSTEM_ARCHITECTURE}.tgz"
# Удаляем прежний пакет
rm -f "$PKG_FILE"

# Выполняем сборку пакета
"$PKG_CREATE" \
 -A "$SYSTEM_ARCHITECTURE" \
 -d "$META_DIR/desc" \
 -D COMMENT="$PACKAGE_SUMMARY" \
 -D FULLPKGPATH="converters/$PACKAGE_NAME" \
 -D PORTSDIR="/usr/ports" \
 -D HOMEPAGE="$PACKAGE_URL" \
 -D MAINTAINER="ANYKS <$PACKAGE_EMAIL>" \
 -M "$META_DIR/display" \
 -B "$APP_DIR" \
 -f "$META_DIR/plist" \
 -p "$PREFIX" \
 "$PKG_FILE" || exit 1

# Удаляем временные каталоги
rm -rf "$APP_DIR" "$META_DIR"

# Выводим сообщение об удачной сборке
echo "Successfully created package $(basename "$PKG_FILE")"
echo "To install the application, please perform a:"
echo ""
echo "$ doas pkg_add -D unsigned ./$(basename "$PKG_FILE")"
echo ""
