/**
 * @file: env.hpp
 * @date: 2024-09-22
 *
 * @telegram: @forman
 * @author: Yuriy Lobarev
 * @phone: +7 (910) 983-95-90
 * @email: forman@anyks.com
 * @site: https://anyks.com
 *
 * @copyright: Copyright © 2024
 */

#ifndef __ACU_ENV__
#define __ACU_ENV__

#ifndef ACU_SHARED_LIBRARY_IMPORT
	#define ACU_SHARED_LIBRARY_IMPORT
#endif

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Стандартная библиотека
 */
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <nlohmann/json.hpp>

/**
 * Наши модули
 */
#include <sys/fmk.hpp>
#include <sys/log.hpp>

/**
 * Наши модули
 */
#include <lib.hpp>

// Устанавливаем область видимости
using namespace std;
// Подписываемся на пространство имён awh
using namespace awh;

// Активируем пространство имён json
using json = nlohmann::json;

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Env Класс модуля работы с переменными окружения
	 */
	typedef class Env {
		private:
			// Флаг автоматического чтения текстовой переменной
			bool _automatic;
		private:
			// Список полученных переменных окружения
			mutable json _data;
		private:
			/**
			 * Название переменной содержащей текст
			 * и префикс переменной окружения
			 */
			string _text, _prefix;
		private:
			// Создаём объект фреймворка
			const fmk_t * _fmk;
			// Создаём объект работы с логами
			const log_t * _log;
		private:
			/**
			 * env Метод извлечения переменной окружения
			 * @param key переменная окружения
			 * @return    значение переменной
			 */
			string env(const string & key) const noexcept {
				// Результат работы функции
				string result = "";
				// Если значение переменной передано
				if(!key.empty() && !this->_prefix.empty() && (this->_fmk != nullptr)){
					// Получаем суффикс переменной окружения
					string suffix = key;
					// Переводим суффикс в верхний регистр
					this->_fmk->transform(suffix, fmk_t::transform_t::UPPER);
					// Получаем значение переменной
					const char * val = ::getenv(this->_fmk->format("%s_%s", this->_prefix.c_str(), suffix.c_str()).c_str());
					// Запоминаем результат
					result = (val == nullptr ? "" : val);
				}
				// Выводим результат
				return result;
			}
		private:
			/**
			 * add Метод выполнения добавления значения в базу данных
			 * @param key   ключ записи для добавления
			 * @param value текстовое значение для добавления
			 */
			void add(const string & key, const string & value) noexcept {
				// Если строка передана
				if(!value.empty() && (value.find("|") != string::npos)){
					// Список параметров
					vector <string> params;
					// Результат работы функции
					json result = json::array();
					// Выполняем сплит параметров
					if(!this->_fmk->split(value, "|", params).empty()){
						// Заполняем массив данными
						for(auto & param : params){
							// Если ключ - это число или отрицательное число
							if(this->_fmk->is(param, fmk_t::check_t::NUMBER))
								// Выполняем добавление числа
								result.push_back(::stoull(param));
							// Если ключ - это дробное число
							else if(this->_fmk->is(param, fmk_t::check_t::DECIMAL))
								// Выполняем добавление числа
								result.push_back(::stod(param));
							// Если - это флаг, устанавливаем истинное булевое значение
							else if(this->_fmk->compare(param, "true") || this->_fmk->compare(param, "on") || this->_fmk->compare(param, "yes"))
								// Выполняем добавление истинное значение
								result.push_back(true);
							// Если - это флаг, устанавливаем ложное булевое значение
							else if(this->_fmk->compare(param, "false") || this->_fmk->compare(param, "off") || this->_fmk->compare(param, "no"))
								// Выполняем добавление ложное значение
								result.push_back(false);
							// Добавляем строку как она есть
							else result.push_back(param);
						}
					}
					// Добавляем полученный массив в базу данных
					this->_data["work"][key] = std::move(result);
				// Если передаваемое значение не является массивом
				} else {
					// Если ключ - это число или отрицательное число
					if(this->_fmk->is(value, fmk_t::check_t::NUMBER))
						// Выполняем добавление вещественного числа в базу данных
						this->_data["work"][key] = ::stoull(value);
					// Если ключ - это дробное число
					else if(this->_fmk->is(value, fmk_t::check_t::DECIMAL))
						// Выполняем добавление числа с двойной точностью в базу данных
						this->_data["work"][key] = ::stod(value);
					// Если - это флаг, устанавливаем истинное булевое значение
					else if(this->_fmk->compare(value, "true") || this->_fmk->compare(value, "on") || this->_fmk->compare(value, "yes"))
						// Выполняем добавление истинное значение
						this->_data["work"][key] = true;
					// Если - это флаг, устанавливаем ложное булевое значение
					else if(this->_fmk->compare(value, "false") || this->_fmk->compare(value, "off") || this->_fmk->compare(value, "no"))
						// Выполняем добавление ложное значение
						this->_data["work"][key] = false;
					// Добавляем строку как она есть
					else this->_data["work"][key] = value;
				}
			}
		public:
			/**
			 * size Метод извлечения количества аргументов
			 * @param root флаг чтения из корневого раздела
			 * @return     количество полученных аргументов
			 */
			size_t size(const bool root = false) const noexcept {
				// Выводим количество собранных аргументов
				return (root ? this->_data.size() : (this->_data.contains("work") ? this->_data.at("work").size() : 0));
			}
		public:
			/**
			 * exist Метод проверки на существование ключа
			 * @param key  ключ переменной для проверки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool exist(const string & key, const bool root = false) const noexcept {
				// Выполняем поиск наличия ключа
				bool result = (root ? this->_data.contains(key) : (this->_data.contains("work") && this->_data.at("work").contains(key)));
				// Если ключ в базе данных не существует
				if(!root && !result){
					// Проверяем ключ в переменных окружения
					const string & value = this->env(key);
					// Если данные получены в переменных окружения
					if((result = !value.empty()))
						// Добавляем значение новой переменной
						const_cast <Env *> (this)->add(key, value);
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * is Метод проверки наличия и значения переменной
			 * @param key  ключ переменной для проверки
			 * @param root флаг чтения из корневого раздела
			 * @param val  значение переменной для проверки (если требуется)
			 */
			bool is(const string & key, const bool root = false, const string & val = "") const noexcept {
				// Результат работы функции
				bool result = false;
				// Если ключ передан
				if(!key.empty()){
					// Выполняем чтение значения ключа
					string data = this->get(key, root);
					// Если значение переменной не получено
					if(data.empty() && !root) data = this->env(key);
					// Если искомый ключ существует и нужно выполнить сравнение значений
					if((result = !data.empty()) && !val.empty())
						// Выполняем сравнение полученного значения ключа с переданным значением для проверки
						result = this->_fmk->compare(data, val);
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * isNull Функция проверки ключа записи на тип Null
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isNull(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_null() : true) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_null() : true)
				);
			}
			/**
			 * isArray Функция проверки ключа записи на тип Array
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isArray(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_array() : false) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_array() : false)
				);
			}
			/**
			 * isObject Функция проверки ключа записи на тип Object
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isObject(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_object() : false) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_object() : false)
				);
			}
			/**
			 * isNumber Функция проверки ключа записи на тип Number
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isNumber(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_number() : false) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_number() : false)
				);
			}
			/**
			 * isString Функция проверки ключа записи на тип String
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isString(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_string() : false) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_string() : false)
				);
			}
			/**
			 * isBoolean Функция проверки ключа записи на тип Boolean
			 * @param key  ключ записи для првоерки
			 * @param root флаг чтения из корневого раздела
			 * @return     результат проверки
			 */
			bool isBoolean(const string & key, const bool root = false) const noexcept {
				// Выполняем проверку ключа в базе
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).is_boolean() : false) :
					(this->exist(key, root) ? this->_data.at("work").at(key).is_boolean() : false)
				);
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T>
			/**
			 * get Метод извлечения значения ключа из базы данных
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных
			 */
			T get(const string & key, const bool root = false) const noexcept {
				// Выводим булевые данные
				return (
					root ? (this->exist(key, root) ? this->_data.at(key).get <T> () : (T) 0) :
					(this->exist(key, root) ? this->_data.at("work").at(key).get <T> () : (T) 0)
				);
			}
			/**
			 * get Метод извлечения значения ключа из базы данных в виде строки
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных в виде строки
			 */
			string get(const string & key, const bool root = false) const noexcept {
				// Результат работы функции
				string result = "";
				// Если ключ найден в объекте JSON
				if(this->exist(key, root)){
					// Если запись является строкой
					if(root ? this->_data.at(key).is_string() : this->_data.at("work").at(key).is_string())
						// Выводим результат в виде строки
						result = (root ? this->_data.at(key).get <string> () : this->_data.at("work").at(key).get <string> ());
					// Если запись является массивом или объектом
					else if(root ? (this->_data.at(key).is_array() || this->_data.at(key).is_object()) : (this->_data.at("work").at(key).is_array() || this->_data.at("work").at(key).is_object()))
						// Выводим результат в виде строки
						result = (root ? this->_data.at(key).dump() : this->_data.at("work").at(key).dump());
					// Если запись является числом
					else if(root ? this->_data.at(key).is_number() : this->_data.at("work").at(key).is_number())
						// Выводим результат в виде строки
						result = this->_fmk->noexp((root ? this->_data.at(key).get <double> () : this->_data.at("work").at(key).get <double> ()), true);
					// Если запись является булевым значением
					else if(root ? this->_data.at(key).is_boolean() : this->_data.at("work").at(key).is_boolean())
						// Выводим результат в виде строки
						result = std::to_string(root ? this->_data.at(key).get <bool> () : this->_data.at("work").at(key).get <bool> ());
				// Если ключа в базе данных не найдено
				} else if(!root) {
					// Проверяем ключ в переменных окружения
					result = this->env(key);
					// Если данные получены в переменных окружения
					if(!result.empty())
						// Добавляем значение новой переменной
						const_cast <Env *> (this)->add(key, result);
				}
				// Выводим пустоту
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T>
			/**
			 * arr Метод извлечения значения ключа из базы данных в виде массива
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных в виде массива
			 */
			const vector <T> arr(const string & key, const bool root = false) const noexcept {
				// Результат работы функции
				vector <T> result;
				// Если ключ передан
				if(!key.empty()){
					// Если ключ в базе данных существует
					if(this->exist(key, root) && (root ? this->_data.at(key).is_array() : this->_data.at("work").at(key).is_array())){
						// Переходим по всему массиву
						for(auto & item : (root ? this->_data.at(key) : this->_data.at("work").at(key)))
							// Формируем список полученных значений
							result.push_back(item.get <T> ());
					}
				}
				// Выводим результат
				return result;
			}
			/**
			 * arr Метод извлечения значения ключа из базы данных в виде массива
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных в виде массива
			 */
			const vector <string> arr(const string & key, const bool root = false) const noexcept {
				// Результат работы функции
				vector <string> result;
				// Если ключ передан
				if(!key.empty()){
					// Если ключ в базе данных существует
					if(this->exist(key, root) && (root ? this->_data.at(key).is_array() : this->_data.at("work").at(key).is_array())){
						// Переходим по всему массиву
						for(auto & item : (root ? this->_data.at(key) : this->_data.at("work").at(key))){
							// Если значение является строкой
							if(item.is_string())
								// Добавляем значение как она есть
								result.push_back(item.get <string> ());
							// Если значение является массивом или объектом
							else if(item.is_array() || item.is_object())
								// Добавляем дамп объекта или массива
								result.push_back(item.dump());
							// Если значение является числом
							else if(item.is_number())
								// Добавляем сконвертированную строку
								result.push_back(this->_fmk->noexp(item.get <double> ()));
							// Если значение является булевым типом
							else if(item.is_boolean())
								// Добавляем сконвертированную строку
								result.push_back(item.get <bool> () ? "true" : "false");
							// Иначе добавляем NULL
							else result.push_back("null");
						}
					}
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T>
			/**
			 * obj Метод извлечения значения ключа из базы данных в виде объекта
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных в виде объекта
			 */
			const unordered_map <string, T> obj(const string & key, const bool root = false) const noexcept {
				// Результат работы функции
				unordered_map <string, T> result;
				// Если ключ передан
				if(!key.empty()){
					// Если ключ в базе данных существует
					if(this->exist(key, root) && (root ? this->_data.at(key).is_object() : this->_data.at("work").at(key).is_object())){
						// Переходим по всему объекту
						for(auto & el : (root ? this->_data.at(key).items() : this->_data.at("work").at(key).items()))
							// Формируем список полученных значений
							result.emplace(el.key(), el.value().get <T> ());
					}
				}
				// Выводим результат
				return result;
			}
			/**
			 * obj Метод извлечения значения ключа из базы данных в виде объекта
			 * @param key  ключ записи в базе данных
			 * @param root флаг чтения из корневого раздела
			 * @return     значение ключа в базе данных в виде объекта
			 */
			const unordered_map <string, string> obj(const string & key, const bool root = false) const noexcept {
				// Результат работы функции
				unordered_map <string, string> result;
				// Если ключ передан
				if(!key.empty()){
					// Если ключ в базе данных существует
					if(this->exist(key, root) && (root ? this->_data.at(key).is_object() : this->_data.at("work").at(key).is_object())){
						// Переходим по всему объекту
						for(auto & el : (root ? this->_data.at(key).items() : this->_data.at("work").at(key).items())){
							// Если значение является строкой
							if(el.value().is_string())
								// Добавляем значение как она есть
								result.emplace(el.key(), el.value().get <string> ());
							// Если значение является массивом или объектом
							else if(el.value().is_array() || el.value().is_object())
								// Добавляем дамп объекта или массива
								result.emplace(el.key(), el.value().dump());
							// Если значение является числом
							else if(el.value().is_number())
								// Добавляем сконвертированную строку
								result.emplace(el.key(), this->_fmk->noexp(el.value().get <double> ()));
							// Если значение является булевым типом
							else if(el.value().is_boolean())
								// Добавляем сконвертированную строку
								result.emplace(el.key(), el.value().get <bool> () ? "true" : "false");
							// Иначе добавляем NULL
							else result.emplace(el.key(), "null");
						}
					}
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * config Метод получения конфигурационных данных в формате JSON
			 * @return конфигурационные данные
			 */
			const json & config() const noexcept {
				// Выводим данные конфига
				return this->_data;
			}
			/**
			 * config Метод добавления данных конфига в формате JSON
			 * @param config данные в формате JSON
			 */
			void config(const json & config) noexcept {
				// Если данные переданы
				if(!config.empty()){
					// Переходим по всем ключам и добавляем всё в базу данных
					for(auto & el : config.items()){
						// Если мы нашли рабочий раздел, пропускаем его
						if(this->_fmk->compare(el.key(), "work"))
							// Пропускаем итерацию перебора
							continue;
						// Если такой ключ не существует
						else if(this->_data.count(el.key()) < 1)
							// Устанавливаем данные конфига
							this->_data[el.key()] = el.value();
					}
					// Если рабочий раздел существует
					if(config.contains("work") && config.at("work").is_object()){
						// Переходим по всем ключам и добавляем всё в базу данных
						for(auto & el : config.at("work").items()){
							// Если такой ключ не существует
							if(this->_data.at("work").count(el.key()) < 1)
								// Устанавливаем данные конфига
								this->_data.at("work")[el.key()] = el.value();
						}
					}
				}
			}
			/**
			 * config Метод добавления данных конфига в формате строки JSON
			 * @param config данные в формате строки JSON
			 */
			void config(const string & config) noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Устанавливаем конфигурационный файл
					this->config(static_cast <json> (json::parse(config)));
				// Если возникает ошибка
				} catch(const exception & error) {
					// Выводим сообщение об ошибке
					this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
				}
			}
		public:
			/**
			 * text Метод установки списка имён которые нужно извлечь
			 * @param text название переменной для извлечения текстовой информации из потока (если параметром не передана)
			 */
			void text(const string & text) noexcept {
				// Устанавливаем название текстовой переменной
				this->_text = text;
			}
			/**
			 * prefix Метод установки префикса переменной окружения
			 * @param prefix префикс переменной окружения
			 */
			void prefix(const string & prefix) noexcept {
				// Если префикс передан, устанавливаем его
				if(!prefix.empty()){
					// Устанавливаем префикс переменных окружения
					this->_prefix = prefix;
					// Переводим префикс в верхний регистр
					this->_fmk->transform(this->_prefix, fmk_t::transform_t::UPPER);
				}
			}
		public:
			/**
			 * allowAutoRead Метод разрешения или запрещения автоматического чтения текстовой переменной
			 * @param flag значение флага
			 */
			void allowAutoRead(const bool flag = true) noexcept {
				// Устанавливаем флаг автоматического чтения текстовой переменной
				this->_automatic = flag;
			}
		public:
			/**
			 * init Метод инициализации параметров запуска приложения
			 * @param args  список переменных окружения
			 * @param count количество переменных окружения
			 */
			void init(const char * args[], const uint8_t count) noexcept {
				// Позиция найденного значения
				size_t pos = 0;
				// Требуется прочитать значение
				bool isValue = false;
				// Строка аргумента, ключ и занчение
				string arg = "", key = "", val = "";
				// Переходим по массиву аргументов
				for(uint8_t i = 0; i < count; i++){
					// Получаем значение аргумента
					arg = args[i];
					// Если это прямое значение переменной
					if(arg.substr(0, 2).compare("--") == 0){
						// Убираем ожидание значения
						isValue = false;
						// Ищем знак равенства
						pos = arg.find("=", 2);
						// Если знак равенства найден
						if(pos != string::npos){
							// Получаем ключ
							key = arg.substr(2, pos - 2);
							// Получаем значение
							val = arg.substr(pos + 1, arg.length() - (pos + 1));
						// Если знак равенства не найден
						} else {
							// Очищаем значение
							val = "true";
							// Получаем ключ
							key = arg.substr(2, arg.length() - 2);
						}
						// Добавляем полученные данные в список переменных
						if(!key.empty())
							// Добавляем полученное значение
							this->add(key, val);
					// Если это относительное значение переменной
					} else if(arg.front() == '-') {
						// Получаем ключ
						key = arg.substr(1, arg.length() - 1);
						// Устанавливаем полученное значение
						this->add(key, "true");
						// Устанавливаем ожидание значения
						isValue = true;
					// Если это ожидание значения
					} else if(isValue) {
						// Убираем ожидание значения
						isValue = false;
						// Добавляем полученные данные в список переменных
						if(!key.empty() && this->exist(key))
							// Выполняем добавление значения в базу данных
							this->add(key, arg);
					}
				}
				// Если переменная текста установлена и мы её из не получили
				if(!this->_text.empty() && !this->exist(this->_text)){
					// Очищаем значение
					val.clear();
					// Если операционной системой является Windows
					#if defined(_WIN32) || defined(_WIN64)
						// Считываем строку из буфера stdin
						if(this->_automatic && this->env(this->_text).empty())
							// Выполняем чтение из коммандной строки
							std::getline(cin, val);
					// Для всех остальных операционных систем
					#else
						// Считываем строку из буфера stdin
						if(!::isatty(STDIN_FILENO) || (this->_automatic && this->env(this->_text).empty()))
							// Выполняем чтение из коммандной строки
							std::getline(cin, val);
					#endif
					// Добавляем полученные данные в список переменных
					if(!val.empty())
						// Добавляем полученное значение
						this->add(this->_text, val);
				}
			}
			/**
			 * init Метод инициализации параметров запуска приложения
			 * @param args  список переменных окружения
			 * @param count количество переменных окружения
			 */
			void init(const wchar_t * args[], const uint8_t count) noexcept {
				// Позиция найденного значения
				size_t pos = 0;
				// Требуется прочитать значение
				bool isValue = false;
				// Строка аргумента, ключ и занчение
				wstring arg = L"", key = L"", val = L"";
				// Переходим по массиву аргументов
				for(uint8_t i = 0; i < count; i++){
					// Получаем значение аргумента
					arg = args[i];
					// Если это прямое значение переменной
					if(arg.substr(0, 2).compare(L"--") == 0){
						// Убираем ожидание значения
						isValue = false;
						// Ищем знак равенства
						pos = arg.find(L"=", 2);
						// Если знак равенства найден
						if(pos != wstring::npos){
							// Получаем ключ
							key = arg.substr(2, pos - 2);
							// Получаем значение
							val = arg.substr(pos + 1, arg.length() - (pos + 1));
						// Если знак равенства не найден
						} else {
							// Очищаем значение
							val = L"true";
							// Получаем ключ
							key = arg.substr(2, arg.length() - 2);
						}
						// Добавляем полученные данные в список переменных
						if(!key.empty())
							// Добавляем полученное значение
							this->add(this->_fmk->convert(key), this->_fmk->convert(val));
					// Если это относительное значение переменной
					} else if(arg.front() == L'-') {
						// Получаем ключ
						key = arg.substr(1, arg.length() - 1);
						// Устанавливаем полученное значение
						this->add(this->_fmk->convert(key), "true");
						// Устанавливаем ожидание значения
						isValue = true;
					// Если это ожидание значения
					} else if(isValue) {
						// Убираем ожидание значения
						isValue = false;
						// Выполняем конвертацию ключа
						const string & item = this->_fmk->convert(key);
						// Добавляем полученные данные в список переменных
						if(!key.empty() && this->exist(item))
							// Выполняем добавление значения в базу данных
							this->add(item, this->_fmk->convert(arg));
					}
				}
				// Если переменная текста установлена и мы её из не получили
				if(!this->_text.empty() && !this->exist(this->_text)){
					// Значение считываемое из потока
					string value = "";
					// Если операционной системой является Windows
					#if defined(_WIN32) || defined(_WIN64)
						// Считываем строку из буфера stdin
						if(this->_automatic && this->env(this->_text).empty())
							// Выполняем чтение из коммандной строки
							std::getline(cin, value);
					// Для всех остальных операционных систем
					#else
						// Считываем строку из буфера stdin
						if(!::isatty(STDIN_FILENO) || (this->_automatic && this->env(this->_text).empty()))
							// Выполняем чтение из коммандной строки
							std::getline(cin, value);
					#endif
					// Добавляем полученные данные в список переменных
					if(!value.empty())
						// Добавляем полученное значение
						this->add(this->_text, value);
				}
			}
		public:
			/**
			 * operator Создаём оператор извлечения JSON объекта
			 * @return конфигурационные данные
			 */
			operator json & () const noexcept {
				// Выводим данные конфига
				return this->_data;
			}
		public:
			/**
			 * Env Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Env(const fmk_t * fmk, const log_t * log) noexcept :
			 _automatic(false), _text{""}, _prefix{ACU_SHORT_NAME}, _fmk(fmk), _log(log) {}
			/**
			 * Env Конструктор
			 * @param prefix префикс переменной окружения
			 * @param fmk    объект фреймворка
			 * @param log    объект для работы с логами
			 */
			Env(const string & prefix, const fmk_t * fmk, const log_t * log) noexcept :
			 _automatic(false), _text{""}, _prefix{ACU_SHORT_NAME}, _fmk(fmk), _log(log) {
				// Устанавливаем префикс переменных окружения
				this->prefix(prefix);
			}
			/**
			 * Env Конструктор
			 * @param prefix префикс переменной окружения
			 * @param text   название переменной для извлечения текстовой информации из потока (если параметром не передана)
			 * @param fmk    объект фреймворка
			 * @param log    объект для работы с логами
			 */
			Env(const string & prefix, const string & text, const fmk_t * fmk, const log_t * log) noexcept :
			 _automatic(false), _text{text}, _prefix{ACU_SHORT_NAME}, _fmk(fmk), _log(log) {
				// Устанавливаем префикс переменных окружения
				this->prefix(prefix);
			}
	} env_t;
};

#endif // __ACU_ENV__
