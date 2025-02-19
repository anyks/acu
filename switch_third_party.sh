#!/usr/bin/env bash

# Если команда указана
if [ -n "$1" ] && [ -n "$2" ]; then
	# Получаем корневую дирректорию
	readonly ROOT=$(cd "$(dirname "$0")" && pwd)

	# Устанавливаем каталог с скриптами
	readonly SCRIPTS="$ROOT/sh"

	# Каталог для установки собранных библиотек
	readonly PREFIX="$ROOT/third_party"

	# Получаем версию OS
	OS=$(uname -a | awk '{print $1}')

	if [[ $OS =~ "MINGW64" ]]; then
		OS="Windows"
	fi

	# Тип архитектуры
	ARCHITECTURE=""
	# Получаем тип архитектуры
	if [ $OS = "FreeBSD" ]; then # FreeBSD
		ARCHITECTURE=$(sysctl -a | egrep -i 'hw.machine|hw.model|hw.ncpu' | grep hw.machine: | awk '{print $2}')
	else # Linux
		ARCHITECTURE=$(arch)
	fi

	# Выполняем корректировку типа процессора
	if [ $ARCHITECTURE = "arm64" ] || [ $ARCHITECTURE = "aarch64" ]; then
		ARCHITECTURE="arm"
	fi

	# Устанавливаем флаги глобального использования
	# export CPPFLAGS=""
	export CXXFLAGS="$CXXFLAGS -fPIC"
	export CFLAGS="-I$PREFIX/include -fPIC"

	export LDFLAGS="-L$PREFIX/lib"
	export LD_LIBRARY_PATH="$PREFIX/lib"

	# Инициализируем каталоги установки
	export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"

	# Создаём каталоги
	mkdir -p "$PREFIX/bin"
	mkdir -p "$PREFIX/lib"
	mkdir -p "$PREFIX/include"

	# Определяем количество логических ядер
	if [ $OS = "Darwin" ]; then
		# Устанавливаем количество ядер системы
		numproc=$(sysctl -n hw.logicalcpu)
		# Устанавливаем версию операционной системы
		export MACOSX_DEPLOYMENT_TARGET=$(sw_vers -productVersion)
	elif [ $OS = "FreeBSD" ]; then
		# Устанавливаем количество ядер системы
		numproc=$(sysctl -n hw.ncpu)
	else
		# Устанавливаем количество ядер системы
		numproc=$(nproc)
	fi

	# Очистка директории
	clean_directory(){
		git clean -dfx "$1"
	}

	# Очистка подпроекта
	clean_submodule(){
		cd "$ROOT/submodules/$1" || exit 1
		git clean -dfx
		git checkout .
		cd "$ROOT" || exit 1
	}

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

	# Выполняем удаление старых данных модуля
	source $SCRIPTS/*_$1.sh --reset || exit 1
	# Выполняем переключение на указанную ветку
	source $SCRIPTS/*_$1.sh --switch $2 || exit 1
	# Выполняем сборку проекта
	source $SCRIPTS/*_$1.sh --build --b$2 || exit 1
# Если параметры не переданы
else
	# Выводим сообщение об ошибке
	echo "Arguments is not passed"
	# Выходим из скрипта
	exit 1
fi
