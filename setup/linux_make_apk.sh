#!/usr/bin/env bash

##
 # Сборка пакета apk для Alpine Linux и OpenWRT (устанавливается командой apk add)
 #
 # Пакет раскладывается в /usr/bin и /usr/share/licenses - как deb и rpm этого приложения.
 # Собирается командой apk mkpkg в формате v3, который ставят обе системы, поэтому
 # зависимости (third_party) должны быть собраны заранее: bash build_third_party.sh
 #
 # У OpenWRT apk урезан и команды mkpkg не несёт. Тогда сценарий берёт уже собранный
 # прежде apk-tools (каталог acu_apk-tools рядом с клоном) либо просит указать его явно:
 # APK=/путь/к/apk. Собрать apk-tools на месте нельзя: шифрование нужно самому apk-tools,
 # а пакета с заголовками libcrypto у OpenWRT нет (у Alpine он есть - openssl-dev)
 #
 # Проверка наличия mkpkg делается делом - сборкой пустого пакета, - а не справкой:
 # урезанный apk OpenWRT на «mkpkg --help» печатает общую справку вместо отказа
 #
 # Пакет не подписан: apk ставит его с ключом --allow-untrusted
 #
 # Установка: apk add --allow-untrusted ./acu-<версия>-r<выпуск>_<ОС>_<архитектура>.apk
 ##

# Имя пакета (не итоговое)
readonly PACKAGE_NAME="acu"

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
readonly OS=$(uname -a | awk '{print $1}')

# Адрес каталога с собранными бинарями
readonly BUILD_DIR="$ROOT/../build"

# Адрес каталога с рабочим приложением
readonly APP_DIR="$ROOT/../${PACKAGE_NAME}_app"
# Каталог раскладки файлов пакета
readonly STAGE="$APP_DIR/usr"
# Каталог сборки apk-tools для систем, где apk не умеет mkpkg
readonly APK_TOOLS_DIR="$ROOT/../${PACKAGE_NAME}_apk-tools"

# Адрес электронной почты
readonly PACKAGE_EMAIL="info@anyks.com"
# URL-адрес сайта
readonly PACKAGE_URL="https://acu.anyks.com"
# Краткое описание приложения
readonly PACKAGE_SUMMARY="ANYKS - conversion utility"
# Описание приложения
readonly PACKAGE_DESCRIPTION="ACU - console data conversion utility: base64, JSON, XML, CSV, YAML, TOML, timestamps, currencies and GROK patterns. Distributed under the GNU GPL v3."

# Выполняем получение номера релиза
RELEASE_NUMBER="$1"
# Если номер релиза не установлен
if [ ! -n "$RELEASE_NUMBER" ]; then
	# Устанавливаем номер релиза по умолчанию
	RELEASE_NUMBER="1"
fi

# Определяем является ли операционная система Linux
if ! [ "$OS" = "Linux" ]; then
	echo "Error: Only for Linux (Alpine, OpenWRT)"
	exit 1
fi

# Определяем является ли система пакетов apk
if ! command -v apk > /dev/null 2>&1; then
	echo "Error: apk package manager is not found"
	exit 1
fi

##
 # Функция проверки наличия команды mkpkg у apk
 #
 # @param apk  вызываемый apk
 # @param libs каталог его библиотек (пустой у системного)
 # @return     успех сборки пробного пакета
 ##
has_mkpkg(){
	local probe=$(mktemp -d)
	mkdir -p "$probe/root" || return 1
	env LD_LIBRARY_PATH="$2" "$1" mkpkg -I "name:probe" -I "version:0-r0" -F "$probe/root" -o "$probe/probe.apk" > /dev/null 2>&1
	local rc=$?
	rm -rf "$probe"
	[ $rc -eq 0 ]
}

##
 # Функция поиска apk с командой mkpkg
 #
 # Выводит две строки: путь к apk и каталог его библиотек (пустой у системного)
 ##
find_apk(){
	# Если apk указан явно
	if [ -n "$APK" ]; then
		has_mkpkg "$APK" "" && printf "%s\n\n" "$APK"
		return
	fi
	# Если системный apk умеет mkpkg (Alpine)
	if has_mkpkg "$(command -v apk)" ""; then
		printf "%s\n\n" "$(command -v apk)"
		return
	fi
	# Если apk-tools уже собран прежним запуском
	if [ -x "$APK_TOOLS_DIR/src/apk" ] && has_mkpkg "$APK_TOOLS_DIR/src/apk" "$APK_TOOLS_DIR/src"; then
		printf "%s\n%s\n" "$APK_TOOLS_DIR/src/apk" "$APK_TOOLS_DIR/src"
	fi
}

# Получаем apk с командой mkpkg
APK_FOUND=$(find_apk)
# Если такого нет, спрашиваем, где взять
if [ -z "$APK_FOUND" ]; then
	echo "Error: this apk cannot build packages (mkpkg is missing) and no ready apk-tools was found."
	echo "Error: keep a built apk-tools in $APK_TOOLS_DIR, or point at a working one: APK=/path/to/apk"
	echo "Error: building apk-tools here is not possible - it needs libcrypto headers,"
	echo "Error: which OpenWRT does not package (Alpine has them as openssl-dev)."
	exit 1
fi
# Путь к apk и каталог его библиотек
readonly APK_BIN=$(echo "$APK_FOUND" | sed -n 1p)
readonly APK_LIBS=$(echo "$APK_FOUND" | sed -n 2p)

# Очистка сборочной директории
if [ -d "$BUILD_DIR" ]; then
	rm -rf "$BUILD_DIR" || exit 1
fi

# Выполняем создание каталогов раскладки
rm -rf "$APP_DIR" || exit 1
mkdir -p "$STAGE/bin" || exit 1
mkdir -p "$STAGE/share/licenses/$PACKAGE_NAME" || exit 1

# Собираем приложение
mkdir "$BUILD_DIR" || exit 1
cd "$BUILD_DIR" || exit 1

# Выполняем сборку приложения
cmake \
 -DCMAKE_BUILD_TYPE=Release \
 .. || exit 1

cmake --build . || exit 1

# Переходим в корневой каталог обратно
cd "$ROOT/../" || exit 1

# Имя исполнительного файла
EXECUTABLE_FILE=$(find . -name "$PACKAGE_NAME" -type f | head -1)

# Если исполнительный файл не найден, печатаем сообщение и прекращаем работу
if [ -z "${EXECUTABLE_FILE}" ]; then
	echo "Error: Executable file \"$PACKAGE_NAME\" is not found! Please build the project and run again!"
	exit 1
fi

# Кладём приложение и лицензию
cp "$EXECUTABLE_FILE" "$STAGE/bin"/ || exit 1
cp "$ROOT/../LICENSE" "$STAGE/share/licenses/$PACKAGE_NAME"/ || exit 1

# Получаем версию приложения
GREP_VERSION_PHRASE="ANYKS - conversion utility"
VERSION=$("$EXECUTABLE_FILE" -V | grep "$GREP_VERSION_PHRASE")
VERSION=${VERSION#$GREP_VERSION_PHRASE}
VERSION=$(echo $VERSION | awk '{print $1}')

# Выставляем права доступа
find "$APP_DIR" -type d -exec chmod 755 {} \;
find "$APP_DIR" -type f -exec chmod 644 {} \;
chmod 755 "$STAGE/bin/$PACKAGE_NAME"

##
 # Зависимости пакета - пакеты, которым принадлежат библиотеки нужные приложению
 #
 # Имена их у систем разные: libstdc++ и libgcc у Alpine, libstdcpp6 и libgcc1 у OpenWRT,
 # потому они спрашиваются у самой системы, а не пишутся списком
 ##
DEPENDENCIES=""
for lib in $(ldd "$STAGE/bin/$PACKAGE_NAME" 2> /dev/null | awk '/=>/ {print $3} !/=>/ && /^\t\// {print $1}' | sort -u); do
	# Получаем пакет которому принадлежит библиотека (путь ld-musl вида /lib/ld-musl-x86_64.so.1 тоже учитывается)
	owner=$(apk info --who-owns "$(readlink -f "$lib")" 2> /dev/null | sed -n 's/.* is owned by \(.*\)-[^-]*-r[0-9]*$/\1/p')
	if [ -z "$owner" ]; then
		owner=$(apk info --who-owns "$lib" 2> /dev/null | sed -n 's/.* is owned by \(.*\)-[^-]*-r[0-9]*$/\1/p')
	fi
	if [ -n "$owner" ] && ! echo " $DEPENDENCIES " | grep -q " $owner "; then
		echo "Add dependent package: $owner ($lib)"
		DEPENDENCIES="$DEPENDENCIES $owner"
	fi
done
DEPENDENCIES=$(echo $DEPENDENCIES)

# Получаем имя ОС по образцу tar-архива: название и версия одним словом (OpenWrt25.12.5)
OS_NAME=$(cat /etc/os-release | grep "^NAME")
OS_NAME=${OS_NAME##*=}
OS_NAME=$(echo $OS_NAME | awk '{print $1}')
VERSION_ID=$(cat /etc/os-release | grep VERSION_ID)
VERSION_ID=${VERSION_ID##*=}
OS_NAME="${OS_NAME}${VERSION_ID}"
OS_NAME=$(echo "${OS_NAME//\"}")

# Получаем архитектуру пакетов системы
readonly SYSTEM_ARCHITECTURE=$(apk --print-arch)
# В имя файла кладём принятую в матрице метку (amd64/arm64), а поле arch пакета остаётся
# выводом apk: под другой архитектурой система отказывает устанавливать пакет
NAME_ARCH="$SYSTEM_ARCHITECTURE"
case "$NAME_ARCH" in
	x86_64)	NAME_ARCH="amd64";;
	aarch64) NAME_ARCH="arm64";;
esac

# Создаем имя пакета
readonly PKG_FILE="$ROOT/../${PACKAGE_NAME}-${VERSION}-r${RELEASE_NUMBER}_${OS_NAME}_${NAME_ARCH}.apk"
# Удаляем прежний пакет
rm -f "$PKG_FILE"

# Выполняем сборку пакета
env LD_LIBRARY_PATH="$APK_LIBS" "$APK_BIN" mkpkg \
 --info "name:$PACKAGE_NAME" \
 --info "version:$VERSION-r$RELEASE_NUMBER" \
 --info "description:$PACKAGE_SUMMARY" \
 --info "arch:$SYSTEM_ARCHITECTURE" \
 --info "license:GPL-3.0-only" \
 --info "origin:$PACKAGE_NAME" \
 --info "maintainer:ANYKS <$PACKAGE_EMAIL>" \
 --info "url:$PACKAGE_URL" \
 --info "depends:$DEPENDENCIES" \
 --files "$APP_DIR" \
 --output "$PKG_FILE" || exit 1

# Удаляем временные каталоги
rm -rf "$APP_DIR"

# Выводим сообщение об удачной сборке
echo "Successfully created package $(basename "$PKG_FILE")"
echo "To install the application, please perform a:"
echo ""
echo "# apk add --allow-untrusted ./$(basename "$PKG_FILE")"
echo ""
