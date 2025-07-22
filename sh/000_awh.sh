#!/usr/bin/env bash

# Если команда указана
if [ -n "$1" ]; then
	# Если необходимо удалить или очистить модуль
	if [ $1 = "--clean" ]; then
		# Удаляем фал контроля сборки проекта
		rm -f "$ROOT/submodules/awh/.stamp_done"
		# Удаляем каталог сборки
		rm -rf "$ROOT/submodules/awh/build"
		# Для операционной системы Windows
		if [[ $OS = "Windows" ]]; then
			# Удаляем все зависимости библиотеки
			rm -f "$ROOT/third_party/lib/libawh.lib"
		# Для всех остальных операционных систем
		else
			# Удаляем все зависимости библиотеки
			rm -f "$ROOT/third_party/lib/libawh.a"
		fi
		# Удаляем все зависимые заголовки библиотеки
		rm -rf "$ROOT/third_party/include/awh"

		# Переходим обратно в рабочий каталог
		cd "$ROOT" || exit 1
	# Если необходимо полностью сбросить проект
	elif [ $1 = "--reset" ]; then
		# Очищаем зависимость проекта
		clean_submodule "awh"
		# Очищаем все зависимости модуля
		"$ROOT/submodules/awh/build_third_party.sh" --clean
		# Для операционной системы Windows
		if [[ $OS = "Windows" ]]; then
			# Удаляем все зависимости библиотеки
			rm -f "$ROOT/third_party/lib/libawh.lib"
		# Для всех остальных операционных систем
		else
			# Удаляем все зависимости библиотеки
			rm -f "$ROOT/third_party/lib/libawh.a"
		fi
		# Удаляем все зависимые заголовки библиотеки
		rm -rf "$ROOT/third_party/include/awh"

		# Переходим обратно в рабочий каталог
		cd "$ROOT" || exit 1
	# Если необходимо выполнить переключение на указанную ветку
	elif [ $1 = "--switch" ] && [ -n "$2" ]; then
		# Переключение ветки
		src="$ROOT/submodules/awh"
		printf "\n****** ANYKS AWH ******\n"
		cd "$src" || exit 1

		# Выполняем получение данных с репозитория
		git pull origin $2 || exit 1
		git fetch origin $2 || exit 1

		# Переключаемся на указанную ветку
		git checkout $2 || exit 1

		# Переходим обратно в рабочий каталог
		cd "$ROOT" || exit 1
	# Если необходимо собрать проект
	elif [ $1 = "--build" ] || [ $1 = "--update" ]; then
		# Сборка AWH
		src="$ROOT/submodules/awh"
		if [ ! -f "$src/.stamp_done" ]; then
			printf "\n****** ANYKS AWH ******\n"
			cd "$src" || exit 1

			# Устанавливаем название флага
			FLAG="--v"
			# Устанавливаем название ветки/тега/версии по умолчанию
			NAME="4.3.0"

			# Если ветка или тег передан
			if [ -n "$2" ]; then
				# Получаем название ветки или тега
				NAME=$(echo "${2:3}")
				# Получаем флаг ветки или тега
				FLAG=$(echo "$2" | cut -c-3)
			fi

			# Если флаг передан в виде ветки
			if [ $FLAG = "--b" ]; then
				# Скачиваем зависимости
				git fetch origin $NAME
				# Переключаемся на ветку
				git checkout $NAME
				# Выполняем обновление репозитория
				git pull origin $NAME

				# Если требуется собрать зависимости с нуля
				if [[ ! $1 = "--update" ]]; then
					# Устанавливаем минимальную версию CMake
					export CMAKE_POLICY_VERSION_MINIMUM=3.5

					# Выполняем конфигурацию проекта
					if [[ $OS = "Windows" ]]; then
						# Выполняем сборку зависимостей
						./build_third_party.sh
					else
						# Выполняем сборку зависимостей
						./build_third_party.sh --idn
					fi
				fi
			# Если флаг передан в виде коммита
			elif [ $FLAG = "--c" ]; then
				# Если установлена команда обновления сборки
				if [[ $1 = "--update" ]]; then
					# Выполняем обновление репозитория
					git pull origin
					# Переходим на указанный коммит
					git reset --hard $NAME
				else
					# Переходим на указанный коммит
					git reset --hard $NAME

					# Устанавливаем минимальную версию CMake
					export CMAKE_POLICY_VERSION_MINIMUM=3.5

					# Выполняем конфигурацию проекта
					if [[ $OS = "Windows" ]]; then
						# Выполняем сборку зависимостей
						./build_third_party.sh
					else
						# Выполняем сборку зависимостей
						./build_third_party.sh --idn
					fi
				fi
			# Если флаг передан в виде тега
			elif [ $FLAG = "--t" ]; then
				# Выполняем удаление предыдущей закаченной версии
				git tag -d $NAME
				# Закачиваем все теги
				git fetch --all --tags
				# Удаляем старую ветку
				git branch -D $NAME-branch
				# Выполняем переключение на указанную версию
				git checkout -b $NAME-branch $NAME

				# Если установлена команда обновления сборки
				if [[ $1 = "--update" ]]; then
					# Выполняем обновление репозитория
					git pull origin
				else
					# Устанавливаем минимальную версию CMake
					export CMAKE_POLICY_VERSION_MINIMUM=3.5

					# Выполняем конфигурацию проекта
					if [[ $OS = "Windows" ]]; then
						# Выполняем сборку зависимостей
						./build_third_party.sh
					else
						# Выполняем сборку зависимостей
						./build_third_party.sh --idn
					fi
				fi
			# Если флаг передан в виде версии
			elif [ $FLAG = "--v" ]; then
				# Переключаемся на ветку
				git checkout main
				# Выполняем удаление предыдущей закаченной версии
				git tag -d v${NAME}
				# Закачиваем все теги
				git fetch --all --tags
				# Удаляем старую ветку
				git branch -D v${NAME}-branch
				# Выполняем переключение на указанную версию
				git checkout -b v${NAME}-branch v${NAME}

				# Если установлена команда обновления сборки
				if [[ $1 = "--update" ]]; then
					# Выполняем обновление репозитория
					git pull origin main
				else
					# Устанавливаем минимальную версию CMake
					export CMAKE_POLICY_VERSION_MINIMUM=3.5

					# Выполняем конфигурацию проекта
					if [[ $OS = "Windows" ]]; then
						# Выполняем сборку зависимостей
						./build_third_party.sh
					else
						# Выполняем сборку зависимостей
						./build_third_party.sh --idn
					fi
				fi
			# Если передан непонятный флаг
			else
				# Сообщаем, что флаги не поддерживаются
				echo "Flag \"$FLAG\" is not supported"
				# Выходим из скрипта
				exit 1
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
			$MAKE -j"$numproc" || exit 1
			# Выполняем установку проекта
			$MAKE install || exit 1

			# Выполняем компенсацию каталогов
			restorelibs $PREFIX

			# Помечаем флагом, что сборка и установка произведена
			touch "$src/.stamp_done"

			# Переходим обратно в рабочий каталог
			cd "$ROOT" || exit 1
		fi
	fi
fi
