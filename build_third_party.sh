#!/usr/bin/env bash

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Получаем версию OS
OS=$(uname -a | awk '{print $1}')

# Флаг активации модуля IDN
IDN="no"

# Флаг активации режима отладки
DEBUG="no"

if [[ $OS =~ "MINGW64" ]]; then
	OS="Windows"
fi

if [ -n "$1" ]; then
	if [ $1 = "--clean" ] || [ $1 = "--reset" ]; then

		# Очистка подпроекта
		clean_submodule(){
			cd "$ROOT/submodules/$1" || exit 1
			git clean -dfx
			git stash
			cd "$ROOT" || exit 1
		}

		# Очистка директории
		clean_directory(){
			git clean -dfx "$1"
		}

		clean_submodule "yaml"
		clean_submodule "libxml2"

		if [ $1 = "--reset" ]; then
			clean_submodule "awh"
			"$ROOT/submodules/awh/build_third_party.sh" --clean
		else
			rm -rf "$ROOT/submodules/awh/build"
			rm -f "$ROOT/submodules/awh/.stamp_done"
		fi

		# Удаляем сборочную директорию
		rm -rf "$ROOT/third_party"

		printf "\n****************************************"
		printf "\n************   Success!!!   ************"
		printf "\n****************************************"
		printf "\n\n\n"

		exit 0
	elif [ $1 != "--update" ]; then
		printf "Usage: config [options]\n"
		printf " --clean - Cleaning all submodules and build directory\n"

		exit 1
	fi
fi

# Каталог для установки собранных библиотек
PREFIX="$ROOT/third_party"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"

# Устанавливаем флаги глобального использования
# export CPPFLAGS=""
export CXXFLAGS="$CXXFLAGS -fPIC"
export CFLAGS="-I$PREFIX/include -fPIC"

export LDFLAGS="-L$PREFIX/lib"
export LD_LIBRARY_PATH="$PREFIX/lib"

# Создаём каталоги
mkdir -p "$PREFIX/bin"
mkdir -p "$PREFIX/lib"
mkdir -p "$PREFIX/include"

# Определяем количество логических ядер
if [ $OS = "Darwin" ]; then
	# Устанавливаем количество ядер системы
	numproc=$(sysctl -n hw.logicalcpu)
	# Если версия MacOS X не установлена
	if [ ! -n "$MACOSX_DEPLOYMENT_TARGET" ]; then
		# Устанавливаем версию операционной системы
		export MACOSX_DEPLOYMENT_TARGET=$(sw_vers -productVersion)
	fi
elif [ $OS = "FreeBSD" ]; then
	# Устанавливаем количество ядер системы
	numproc=$(sysctl -n hw.ncpu)
else
	# Устанавливаем количество ядер системы
	numproc=$(nproc)
fi

if [ $OS = "Darwin" ]; then # MacOS
	INSTALL_CMD="ditto -v"
elif [ $OS = "FreeBSD" ]; then # FreeBSD
	INSTALL_CMD="install -m 0644"
elif [ $OS = "Windows" ]; then # Windows
	INSTALL_CMD="install -D -m 0644"
else # Linux
	INSTALL_CMD="install -D -m 0644"
fi

# Применяем патчи
apply_patch(){
	patch="$ROOT/patches/$1/$2"
	if ! git apply --reverse --check "$patch" 2> /dev/null; then
		echo "applaying patch $patch"
		git apply "$patch" || exit 1
	else
		echo "patch $patch already applied"
	fi
}

if [[ $CMD != "--update" ]]; then
	# Выполняем синхронизацию сабмодулей
	git submodule sync
	# Инициализируем подпроекты
	git submodule update --init --recursive
fi

# Сборка AWH
src="$ROOT/submodules/awh"
if [ ! -f "$src/.stamp_done" ]; then
	printf "\n****** ANYKS AWH ******\n"
	cd "$src" || exit 1

	# Версия AWH
	VER="4.2.9"

	# Переключаемся на main
	git checkout main
	# Выполняем удаление предыдущей закаченной версии
	git tag -d v${VER}
	# Закачиваем все теги
	git fetch --all --tags
	# Удаляем старую ветку
	git branch -D v${VER}-branch
	# Выполняем переключение на указанную версию
	git checkout -b v${VER}-branch v${VER}

	if [[ $CMD = "--update" ]]; then
		# Выполняем обновление репозитория
		git pull origin main
	else
		# Выполняем конфигурацию проекта
		if [[ $OS = "Windows" ]]; then
			# Выполняем сборку зависимостей
			./build_third_party.sh
		else
			# Выполняем сборку зависимостей
			./build_third_party.sh --idn
		fi
	fi

	# Создаём каталог сборки
	mkdir -p "build" || exit 1
	# Переходим в каталог
	cd "build" || exit 1

	# Удаляем старый файл кэша
	rm -rf "$src/build/CMakeCache.txt"

	# Выполняем конфигурацию проекта
	if [[ $OS = "Windows" ]]; then
		cmake \
		 -DCMAKE_BUILD_IDN="ON" \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DCMAKE_SYSTEM_NAME=Windows \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 -G "MSYS Makefiles" \
		 .. || exit 1
	else
		cmake \
		 -DCMAKE_BUILD_IDN="ON" \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 .. || exit 1
	fi

	# Выполняем сборку на всех логических ядрах
	make -j"$numproc" || exit 1
	# Выполняем установку проекта
	make install || exit 1

	# Помечаем флагом, что сборка и установка произведена
	touch "$src/.stamp_done"
	cd "$ROOT" || exit 1
fi

# Сборка YAML
src="$ROOT/submodules/yaml"
if [ ! -f "$src/.stamp_done" ]; then
	printf "\n****** YAML ******\n"
	cd "$src" || exit 1

	# Версия YAML
	VER="0.8.0"

	# Переключаемся на master
	git checkout master
	# Выполняем удаление предыдущей закаченной версии
	git tag -d v${VER}
	# Закачиваем все теги
	git fetch --all --tags
	# Удаляем старую ветку
	git branch -D v${VER}-branch
	# Выполняем переключение на указанную версию
	git checkout -b v${VER}-branch ${VER}

	if [[ $CMD = "--update" ]]; then
		# Выполняем обновление репозитория
		git pull origin master
	fi

	# Создаём каталог сборки
	mkdir -p "build" || exit 1
	# Переходим в каталог
	cd "build" || exit 1

	# Удаляем старый файл кэша
	rm -rf "$src/build/CMakeCache.txt"

	# Выполняем конфигурацию проекта
	if [[ $OS = "Windows" ]]; then
		cmake \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DCMAKE_SYSTEM_NAME=Windows \
		 -DYAML_CPP_BUILD_TOOLS=OFF \
		 -DYAML_BUILD_SHARED_LIBS=OFF \
		 -DYAML_CPP_BUILD_CONTRIB=OFF \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 -G "MSYS Makefiles" \
		 .. || exit 1
	else
		cmake \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DYAML_CPP_BUILD_TOOLS=OFF \
		 -DYAML_BUILD_SHARED_LIBS=OFF \
		 -DYAML_CPP_BUILD_CONTRIB=OFF \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 .. || exit 1
	fi

	# Выполняем сборку на всех логических ядрах
	make -j"$numproc" || exit 1
	# Выполняем установку проекта
	make install || exit 1

	# Помечаем флагом, что сборка и установка произведена
	touch "$src/.stamp_done"
	cd "$ROOT" || exit 1
fi

# Сборка LibXML2
src="$ROOT/submodules/libxml2"
if [ ! -f "$src/.stamp_done" ]; then
 	printf "\n****** GNU LibXML2 ******\n"
 	cd "$src" || exit 1

	# Версия LibXML2
	VER="2.13.5"

	# Выполняем удаление предыдущей закаченной версии
	git tag -d v${VER}
	# Закачиваем все изменения
	git fetch --all
	# Закачиваем все теги
	git fetch --all --tags
	# Выполняем жесткое переключение на master
	git reset --hard origin/master
	# Переключаемся на master
	git checkout master
	# Выполняем обновление данных
	git pull origin master
	# Удаляем старую ветку
	git branch -D v${VER}-branch
	# Выполняем переключение на указанную версию
	git checkout -b v${VER}-branch v${VER}

	# Создаём каталог сборки
	mkdir -p "build" || exit 1
	# Переходим в каталог
	cd "build" || exit 1

	# Удаляем старый файл кэша
	rm -rf "$src/build/CMakeCache.txt"

	# Выполняем конфигурацию проекта
	if [[ $OS = "Windows" ]]; then
		cmake \
		 -DBUILD_SHARED_LIBS="OFF" \
		 -DLIBXML2_WITH_LZMA="OFF" \
		 -DLIBXML2_WITH_ZLIB="OFF" \
		 -DLIBXML2_WITH_ICONV="OFF" \
		 -DLIBXML2_WITH_PYTHON="OFF" \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DCMAKE_SYSTEM_NAME=Windows \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 -G "MSYS Makefiles" \
		 .. || exit 1
	else
		cmake \
		 -DBUILD_SHARED_LIBS="OFF" \
		 -DLIBXML2_WITH_LZMA="OFF" \
		 -DLIBXML2_WITH_ZLIB="OFF" \
		 -DLIBXML2_WITH_ICONV="OFF" \
		 -DLIBXML2_WITH_PYTHON="OFF" \
		 -DCMAKE_BUILD_TYPE=Release \
		 -DCMAKE_INSTALL_PREFIX="$PREFIX" \
		 .. || exit 1
	fi

	# Выполняем сборку на всех логических ядрах
	make -j"$numproc" || exit 1
	# Выполняем установку проекта
	make install || exit 1

	# Помечаем флагом, что сборка и установка произведена
	touch "$src/.stamp_done"
	cd "$ROOT" || exit 1
fi

# Если CMAKE-файл уже существует
if [ -f "$ROOT/cmake/FindAWH.cmake" ]; then
	# Выполняем удаление ранее созданный CMAKE-файл
	rm "$ROOT/cmake/FindAWH.cmake" || exit 1
fi

# Если операционной системой является Windows
if [ $OS = "Windows" ]; then
	# Выполняем создание символьной ссылки CMAKE-файла
	ln -s "$PREFIX/cmake/FindAWH.cmake" "$ROOT/cmake/FindAWH.cmake" || exit 1
# Если операционная система является nix-подобной
else
	# Выполняем создание символьной ссылки CMAKE-файла
	ln -f "$PREFIX/cmake/FindAWH.cmake" "$ROOT/cmake/FindAWH.cmake" || exit 1
fi

# Переименовываем расширение библиотек для Windows
if [ $OS = "Windows" ]; then # Windows
	for i in $(ls "$PREFIX/lib" | grep .a$);
	do
		mv "$PREFIX/lib/$i" "$PREFIX/lib/$(basename "$i" .a).lib"
	done
fi

printf "\n****************************************"
printf "\n************   Success!!!   ************"
printf "\n****************************************"
printf "\n\n\n"
