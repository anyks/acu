#!/usr/bin/env bash

##
 # Сборка двоичного пакета FreeBSD и DragonFly BSD (формат pkg, устанавливается командой pkg add)
 #
 # Пакет раскладывается в /usr/local. Зависимости (third_party) должны быть собраны
 # заранее: bash build_third_party.sh
 #
 # У DragonFly основным собирателем системы является gcc 8.3, проекта им не собрать: если CC и
 # CXX не заданы явно, берётся самый свежий GCC из портов, а пакет объявляет его своей
 # зависимостью. Библиотеки времени исполнения этого GCC пакет не тянет: тот кладёт их в свой
 # каталог и в перечне предоставляемых не объявляет
 #
 # Установка: pkg add ./acu-<версия>_FreeBSD_amd64.pkg
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

# Определяем является ли операционная система FreeBSD или DragonFly
if ! [ "$OS" = "FreeBSD" ] && ! [ "$OS" = "DragonFly" ]; then
	echo "Error: Only for FreeBSD and DragonFly BSD"
	exit 1
fi

# Получаем архитектуру машины: у FreeBSD ядро отзывается aarch64, а пакет пишется arm64
SYSTEM_ARCHITECTURE=$(uname -m)
if [ "$SYSTEM_ARCHITECTURE" = "x86_64" ]; then
	SYSTEM_ARCHITECTURE="amd64"
elif [ "$OS" = "FreeBSD" ] && [ "$SYSTEM_ARCHITECTURE" = "aarch64" ]; then
	SYSTEM_ARCHITECTURE="arm64"
fi

# Если у DragonFly собиратель не задан, берём самый свежий GCC из портов
if [ "$OS" = "DragonFly" ] && [ -z "$CXX" ]; then
	GCC_VERSION=$(ls /usr/local/bin | sed -n 's/^g++\([0-9][0-9]*\)$/\1/p' | sort -n | tail -1)
	if [ -z "$GCC_VERSION" ]; then
		echo "Error: GCC from ports is not installed (pkg install gcc14)"
		exit 1
	fi
	export CC="/usr/local/bin/gcc$GCC_VERSION"
	export CXX="/usr/local/bin/g++$GCC_VERSION"
fi

# Получаем пакет, которому принадлежит библиотека C++ собирателя: он станет зависимостью пакета
COMPILER_PKG=""
if [ "$OS" = "DragonFly" ]; then
	COMPILER_PKG=$(pkg which -q "$("$CXX" -print-file-name=libstdc++.so)" 2> /dev/null)
fi

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
mkdir -p "$STAGE/share/licenses/$PACKAGE_NAME" || exit 1
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
cp "$ROOT/../LICENSE" "$STAGE/share/licenses/$PACKAGE_NAME"/ || exit 1

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

# Перечень файлов пакета (каталоги свои помечаются @dir и удаляются вместе с пакетом)
{
	(cd "$APP_DIR" && find ".$PREFIX" -type f | sed 's!^\.!!' | sort)
	(cd "$APP_DIR" && find ".$PREFIX/share/licenses/$PACKAGE_NAME" -type d | sed 's!^\.!@dir !' | sort -r)
} > "$META_DIR/plist" || exit 1

# Зависимость пакета от GCC, чьей библиотекой C++ пользуется приложение
DEPS=""
if [ -n "$COMPILER_PKG" ]; then
	DEPS=$(pkg query "  \"%n\": { origin: \"%o\", version: \"%v\" }" "$COMPILER_PKG")
fi

##
 # Манифест пакета
 #
 # Сообщение выводится при установке и напоминает, что приложение распространяется под GNU GPL v3
 ##
cat > "$META_DIR/manifest" <<EOF
name: "$PACKAGE_NAME"
version: "$VERSION"
origin: "converters/$PACKAGE_NAME"
comment: "$PACKAGE_SUMMARY"
desc: "$PACKAGE_DESCRIPTION"
www: "$PACKAGE_URL"
maintainer: "$PACKAGE_EMAIL"
prefix: "$PREFIX"
licenselogic: "single"
licenses: [ "GPLv3" ]
categories: [ "converters" ]
deps: {
$DEPS
}
messages: [
  { message: <<EOM
$PACKAGE_NAME $VERSION is distributed under the GNU General Public License v3.
The full text of the license:

    $PREFIX/share/licenses/$PACKAGE_NAME/LICENSE
EOM
  }
]
EOF

##
 # Выполняем сборку пакета
 #
 # У DragonFly сжатие xz: zstd, какой pkg берёт по умолчанию, у pkg DragonFly не встроен.
 # Библиотеки GCC исключаются из требуемых по той же причине, по какой они не объявлены у
 # пакета GCC из портов
 ##
if [ "$OS" = "DragonFly" ]; then
	pkg -o SHLIB_REQUIRE_IGNORE_GLOB="libgcc_s.so.*,libstdc++.so.*" create -f txz -M "$META_DIR/manifest" -p "$META_DIR/plist" -r "$APP_DIR" -o "$ROOT/.." || exit 1
else
	pkg create -M "$META_DIR/manifest" -p "$META_DIR/plist" -r "$APP_DIR" -o "$ROOT/.." || exit 1
fi

# Удаляем временные каталоги
rm -rf "$APP_DIR" "$META_DIR"

# Получаем имя созданного пакета и переименовываем его по принятому в поставке образцу
readonly PKG_FILE=$(ls "$ROOT/../$PACKAGE_NAME-$VERSION".* 2> /dev/null | head -1)
readonly TARGET_NAME="$PACKAGE_NAME-${VERSION}_${OS}_${SYSTEM_ARCHITECTURE}.pkg"
mv "$PKG_FILE" "$ROOT/../$TARGET_NAME" || exit 1

# Выводим сообщение об удачной сборке
echo "Successfully created package $TARGET_NAME"
echo "To install the application, please perform a:"
echo ""
echo "$ sudo pkg add ./$TARGET_NAME"
echo ""
