#!/usr/bin/env bash

# Получаем корневую дирректорию
readonly ROOT=$(cd "$(dirname "$0")" && pwd)

# Выполняем пересборку сабмодуля awh
$ROOT/submodule.sh remove awh
$ROOT/submodule.sh add awh https://gitflic.ru/project/anyks/awh.git

# Выполняем пересборку сабмодуля yaml
$ROOT/submodule.sh remove yaml
$ROOT/submodule.sh add yaml https://gitflic.ru/project/third_party/yaml-cpp.git

# Выполняем пересборку сабмодуля libxml2
$ROOT/submodule.sh remove libxml2
$ROOT/submodule.sh add libxml2 https://gitflic.ru/project/third_party/libxml2.git

# Выводим список добавленных модулей
cat $ROOT/.gitmodules
