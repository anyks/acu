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

	# Если операционная система используется Solaris
	if [ $OS = "SunOS" ]; then
		# Устанавливаем жёстко компилятор
		export CC="gcc"
	fi

	# Определяем количество логических ядер
	if [ $OS = "Darwin" ]; then
		# Устанавливаем количество ядер системы
		numproc=$(sysctl -n hw.logicalcpu)
		# Если версия MacOS X не установлена
		if [ ! -n "$MACOSX_DEPLOYMENT_TARGET" ]; then
			# Устанавливаем версию операционной системы
			export MACOSX_DEPLOYMENT_TARGET=$(sw_vers -productVersion)
		fi
	# Если сборка производится в операционной системе Windows, Linux или Solaris
	elif [ $OS = "Windows" ] || [ $OS = "Linux" ] || [ $OS = "SunOS" ]; then
		# Устанавливаем количество ядер системы
		numproc=$(nproc)
	# Если сборка производится в операционной системе FreeBSD, NetBSD или OpenBSD
	elif [ $OS = "FreeBSD" ] || [ $OS = "NetBSD" ] || [ $OS = "OpenBSD" ]; then
		# Устанавливаем количество ядер системы
		numproc=$(sysctl -n hw.ncpu)
	# Если операционная система не определена
	else
		echo "Operating system not defined"
		exit 1
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

	# Фукция компенсации неверных каталогов
	restorelibs(){
		# Если сборка производится в операционной системе Linux
		if [ $OS = "Windows" ] || [ $OS = "Linux" ]; then
			# Если на вход получен каталог
			if [[ -d "$1/lib64" ]]; then
				# Переносим всё что есть в каталоге, в нужный нам каталог
				for i in $(ls "$1/lib64");
				do
					# Если файла нет в каталоге
					if [[ ! -f "$1/lib/$i" ]] && [[ -f "$1/lib64/$i" ]]; then
						echo "Move \"$1/lib64/$i\" to \"$1/lib/$i\""
						mv "$1/lib64/$i" "$1/lib/$i" || exit 1
					fi
				done
				# Удаляем ненужный нам каталог
				rm -rf "$1/lib64" || exit 1
			fi
		# Если сборка производится в операционной системе Solaris
		elif [ $OS = "SunOS" ]; then
			# Если на вход получен каталог
			if [[ -d "$1/lib/64" ]]; then
				# Переносим всё что есть в каталоге, в нужный нам каталог
				for i in $(ls "$1/lib/64");
				do
					# Если файла нет в каталоге
					if [[ ! -f "$1/lib/$i" ]] && [[ -f "$1/lib/64/$i" ]]; then
						echo "Move \"$1/lib/64/$i\" to \"$1/lib/$i\""
						mv "$1/lib/64/$i" "$1/lib/$i" || exit 1
					fi
				done
				# Удаляем ненужный нам каталог
				rm -rf "$1/lib/64" || exit 1
			fi
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
