/**
 * @file: env.hpp
 * @date: 2024-12-22
 *
 * @telegram: @forman
 * @author: Yuriy Lobarev
 * @phone: +7 (910) 983-95-90
 * @email: forman@anyks.com
 * @site: https://anyks.com
 *
 * @copyright: Copyright © 2025
 */

#ifndef __ANYKS_ACU_ENV__
#define __ANYKS_ACU_ENV__

/**
 * Стандартная библиотека
 */
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <unistd.h>

/**
 * Модули AWH
 */
#include <sys/fs.hpp>
#include <sys/fmk.hpp>
#include <sys/log.hpp>

/**
 * Наши модули
 */
#include <lib.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Подписываемся на пространство имён awh
	 */
	using namespace awh;
	/**
	 * Подписываемся на пространство имён rapidjson
	 */
	using namespace rapidjson;
	/**
	 * Активируем пространство имён json
	 */
	using json = Document;
	/**
	 * Env Класс модуля работы с переменными окружения
	 */
	typedef class Env {
		private:
			/**
			 * Флаги типов данных
			 */
			enum class type_t : uint8_t {
				ENV_NONE       = 0x00, // Тип данных не определён
				ENV_NUMBER     = 0x01, // Тип данных Number
				ENV_FLOAT      = 0x02, // Тип данных Float
				ENV_DOUBLE     = 0x03, // Тип данных Double
				ENV_ARRAY      = 0x04, // Тип данных Array
				ENV_OBJECT     = 0x05, // Тип данных Object
				ENV_STRING     = 0x06, // Тип данных String
				ENV_BOOLEAN    = 0x07, // Тип данных Boolean
				ENV_INTEGER    = 0x08, // Тип данных Integer
				ENV_UINTEGER   = 0x09, // Тип данных Unsigned Integer
				ENV_INTEGER64  = 0x0A, // Тип данных Integer 64
				ENV_UINTEGER64 = 0x0B  // Тип данных Unsigned Integer 64
			};
		private:
			// Объект работы с файловой системой
			fs_t _fs;
		private:
			// Список полученных переменных окружения
			json _data;
		private:
			// Флаг автоматического чтения текстовой переменной
			bool _automatic;
		private:
			// Название переменной содержащей текст
			string _text;
			// Префикс переменной окружения
			string _prefix;
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
					/**
					 * Выполняем отлов ошибок
					 */
					try {
						// Получаем суффикс переменной окружения
						string suffix = key;
						// Переводим суффикс в верхний регистр
						this->_fmk->transform(suffix, fmk_t::transform_t::UPPER);
						// Получаем значение переменной
						const char * val = getenv(this->_fmk->format("%s_%s", this->_prefix.c_str(), suffix.c_str()).c_str());
						// Запоминаем результат
						result = (val == nullptr ? "" : val);
					/**
					 * Если возникает ошибка
					 */
					} catch(const exception & error) {
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key), log_t::flag_t::CRITICAL, error.what());
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
						#endif
					}
				}
				// Выводим результат
				return result;
			}
		private:
			/**
			 * set Метод выполнения установку значения в базу данных
			 * @param key   ключ записи для установки
			 * @param value текстовое значение для установки
			 */
			void set(const string & key, const string & value) noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Если строка передана
					if(!value.empty() && (value.find("|") != string::npos)){
						// Список параметров
						vector <string> params;
						// Результат работы функции
						json result(kArrayType);
						// Выполняем сплит параметров
						if(!this->_fmk->split(value, "|", params).empty()){
							// Заполняем массив данными
							for(auto & param : params){
								// Если ключ - это число или отрицательное число
								if(this->_fmk->is(param, fmk_t::check_t::NUMBER)){
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Если число является отрицательным
										if(param.front() == '-')
											// Выполняем добавление отрицательного числа
											result.PushBack(Value(static_cast <int64_t> (stoll(param))).Move(), result.GetAllocator());
										// Выполняем добавление положительного числа
										else result.PushBack(Value(static_cast <uint64_t> (stoull(param))).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем строку как она есть
										result.PushBack(Value(param.c_str(), param.length(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								// Если ключ - это дробное число
								} else if(this->_fmk->is(param, fmk_t::check_t::DECIMAL)) {
									/**
									 * Выполняем отлов ошибок
									 */
									try {
										// Выполняем добавление числа
										result.PushBack(Value(stod(param)).Move(), result.GetAllocator());
									/**
									 * Если возникает ошибка
									 */
									} catch(const exception &) {
										// Добавляем строку как она есть
										result.PushBack(Value(param.c_str(), param.length(), result.GetAllocator()).Move(), result.GetAllocator());
									}
								// Если - это флаг, устанавливаем истинное булевое значение
								} else if(this->_fmk->compare(param, "true"))
									// Выполняем добавление истинное значение
									result.PushBack(Value(kTrueType).Move(), result.GetAllocator());
								// Если - это флаг, устанавливаем ложное булевое значение
								else if(this->_fmk->compare(param, "false"))
									// Выполняем добавление ложное значение
									result.PushBack(Value(kFalseType).Move(), result.GetAllocator());
								// Добавляем строку как она есть
								else result.PushBack(Value(param.c_str(), param.length(), result.GetAllocator()).Move(), result.GetAllocator());
							}
						}
						// Если ключ work не обнаружен
						if(!this->_data.HasMember("work"))
							// Выполняем добавление ключа work
							this->_data.AddMember(Value("work", this->_data.GetAllocator()).Move(), Value(kObjectType).Move(), this->_data.GetAllocator());
						// Если такого ключа ещё не существует в объекте
						if(!this->_data["work"].HasMember(key.c_str())){
							// Создаём значение сообщения
							Value value(kArrayType);
							// Выполняем копирование полученного JSON
							value.CopyFrom(result, result.GetAllocator());
							// Добавляем полученный массив в базу данных
							this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), value.Move(), this->_data.GetAllocator());
						// Если такой ключ уже существует в объекте
						} else this->_data["work"][key.c_str()].CopyFrom(result, result.GetAllocator());
					// Если передаваемое значение не является массивом
					} else if(!value.empty()) {
						// Если ключ work не обнаружен
						if(!this->_data.HasMember("work"))
							// Выполняем добавление ключа work
							this->_data.AddMember(Value("work", this->_data.GetAllocator()).Move(), Value(kObjectType).Move(), this->_data.GetAllocator());
						// Если ключ - это число или отрицательное число
						if(this->_fmk->is(value, fmk_t::check_t::NUMBER)){
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Если такого ключа ещё не существует в объекте
								if(!this->_data["work"].HasMember(key.c_str())){
									// Если число является отрицательным
									if(value.front() == '-')
										// Выполняем добавление вещественного отрицательного числа в базу данных
										this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(static_cast <int64_t> (stoll(value))).Move(), this->_data.GetAllocator());
									// Выполняем добавление вещественного положительного числа в базу данных
									else this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(static_cast <uint64_t> (stoull(value))).Move(), this->_data.GetAllocator());
								// Если такой ключ уже существует в объекте
								} else {
									// Если число является отрицательным
									if(value.front() == '-')
										// Выполняем установку вещественного отрицательного числа в базу данных
										this->_data["work"][key.c_str()].SetInt64(static_cast <int64_t> (stoll(value)));
									// Выполняем добавление вещественного положительного числа в базу данных
									else this->_data["work"][key.c_str()].SetUint64(static_cast <uint64_t> (stoull(value)));
								}
							/**
							 * Если возникает ошибка
							 */
							} catch(const exception &) {
								// Если такого ключа ещё не существует в объекте
								if(!this->_data["work"].HasMember(key.c_str()))
									// Добавляем строку как она есть
									this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(value.c_str(), value.length(), this->_data.GetAllocator()).Move(), this->_data.GetAllocator());
								// Если такой ключ уже существует в объекте
								else this->_data["work"][key.c_str()].SetString(value.c_str(), value.length(), this->_data.GetAllocator());
							}
						// Если ключ - это дробное число
						} else if(this->_fmk->is(value, fmk_t::check_t::DECIMAL)) {
							/**
							 * Выполняем отлов ошибок
							 */
							try {
								// Если такого ключа ещё не существует в объекте
								if(!this->_data["work"].HasMember(key.c_str()))
									// Выполняем добавление числа с двойной точностью в базу данных
									this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(stod(value)).Move(), this->_data.GetAllocator());
								// Если такой ключ уже существует в объекте
								else this->_data["work"][key.c_str()].SetDouble(stod(value));
							/**
							 * Если возникает ошибка
							 */
							} catch(const exception &) {
								// Если такого ключа ещё не существует в объекте
								if(!this->_data["work"].HasMember(key.c_str()))
									// Добавляем строку как она есть
									this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(value.c_str(), value.length(), this->_data.GetAllocator()).Move(), this->_data.GetAllocator());
								// Если такой ключ уже существует в объекте
								else this->_data["work"][key.c_str()].SetString(value.c_str(), value.length(), this->_data.GetAllocator());
							}
						// Если - это флаг, устанавливаем истинное булевое значение
						} else if(this->_fmk->compare(value, "true")) {
							// Если такого ключа ещё не существует в объекте
							if(!this->_data["work"].HasMember(key.c_str()))
								// Выполняем добавление истинное значение
								this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(kTrueType).Move(), this->_data.GetAllocator());
							// Если такой ключ уже существует в объекте
							else this->_data["work"][key.c_str()].SetBool(kTrueType);
						// Если - это флаг, устанавливаем ложное булевое значение
						} else if(this->_fmk->compare(value, "false")) {
							// Если такого ключа ещё не существует в объекте
							if(!this->_data["work"].HasMember(key.c_str()))
								// Выполняем добавление ложное значение
								this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(kFalseType).Move(), this->_data.GetAllocator());
							// Если такой ключ уже существует в объекте
							else this->_data["work"][key.c_str()].SetBool(kFalseType);
						// Добавляем строку как она есть
						} else {
							// Если такого ключа ещё не существует в объекте
							if(!this->_data["work"].HasMember(key.c_str()))
								// Добавляем строку как она есть
								this->_data["work"].AddMember(Value(key.c_str(), key.length(), this->_data.GetAllocator()).Move(), Value(value.c_str(), value.length(), this->_data.GetAllocator()).Move(), this->_data.GetAllocator());
							// Если такой ключ уже существует в объекте
							else this->_data["work"][key.c_str()].SetString(value.c_str(), value.length(), this->_data.GetAllocator());
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(key, value), log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * count Метод извлечения количества аргументов
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     количество полученных аргументов
			 */
			size_t count(const bool root, Args&&... args) const noexcept {
				// Результат работы функции
				size_t result = 0;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если ключ в базе данных существует
								if(this->exist(root, key))
									// Получаем значение родительского объекта
									item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
								// Если ключа в базе данных не найдено
								else if(!root) {
									// Проверяем ключ в переменных окружения
									const string & value = this->env(key);
									// Если данные получены в переменных окружения
									if(!value.empty())
										// Выполняем добавление полученных значений переменной окружения
										const_cast <Env *> (this)->set(key, value);
									// Если ключ в базе данных существует
									if(this->exist(root, key))
										// Получаем значение родительского объекта
										item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								}
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							if(i == (keys.size() - 1)){
								// Если элемент является массивом
								if(item.IsArray() && !item.Empty())
									// Выполняем получение количества элементов в массиве
									result = item.Size();
								// Если это объект
								else if(item.IsObject() && !item.ObjectEmpty()) {
									// Переходим по всем ключам
									for([[maybe_unused]] auto & i : item.GetObj())
										// Подсчитываем весь список ключей
										result++;
								// Запоминаем, что элемент всего один
								} else result = 1;
							}
						}
					// Получаем количество собранных элементов
					} else {
						// Если нужно проверить корневой элемент
						if(root){
							// Если элемент является массивом
							if(this->_data.IsArray() && !this->_data.Empty())
								// Выполняем получение количества элементов в массиве
								result = this->_data.Size();
							// Если это объект
							else if(this->_data.IsObject() && !this->_data.ObjectEmpty()) {
								// Переходим по всем ключам
								for([[maybe_unused]] auto & i : this->_data.GetObj())
									// Подсчитываем весь список ключей
									result++;
							}
						// Если нужно проверить рабочий элемент
						} else if(this->_data.HasMember("work")) {
							// Если элемент является массивом
							if(this->_data["work"].IsArray() && !this->_data["work"].Empty())
								// Выполняем получение количества элементов в массиве
								result = this->_data["work"].Size();
							// Если это объект
							else if(this->_data["work"].IsObject() && !this->_data["work"].ObjectEmpty()) {
								// Переходим по всем ключам
								for([[maybe_unused]] auto & i : this->_data["work"].GetObj())
									// Подсчитываем весь список ключей
									result++;
							}
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * exist Метод проверки на существование ключа
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool exist(const bool root, Args&&... args) const noexcept {
				// Результат работы функции
				bool result = false;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если это корневой элемент
								if(root){
									// Если ключ в базе данных существует
									if(this->_data.HasMember(key.c_str()))
										// Получаем значение родительского объекта
										item.CopyFrom(const_cast <Env *> (this)->_data[key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								// Если элемент дочерний
								} else if(this->_data.HasMember("work")) {
									// Если ключ в базе данных существует
									if(this->_data["work"].HasMember(key.c_str()))
										// Получаем значение родительского объекта
										item.CopyFrom(const_cast <Env *> (this)->_data["work"][key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
									// Если ключа в базе данных не найдено
									else {
										// Проверяем ключ в переменных окружения
										const string & value = this->env(key);
										// Если данные получены в переменных окружения
										if(!value.empty())
											// Выполняем добавление полученных значений переменной окружения
											const_cast <Env *> (this)->set(key, value);
										// Если ключ в базе данных существует
										if(this->_data["work"].HasMember(key.c_str()))
											// Получаем значение родительского объекта
											item.CopyFrom(const_cast <Env *> (this)->_data["work"][key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
										// Выходим из цикла
										else break;
									}
								// Выходим из цикла
								} else break;
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							result = (i == (keys.size() - 1));
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * is Метод проверки наличия и значения переменной
			 * @param root флаг чтения из корневого раздела
			 * @param key  ключ переменной для проверки
			 * @param val  значение переменной для проверки (если требуется)
			 */
			bool is(const bool root, const string & key, const string & val = "") const noexcept {
				// Результат работы функции
				bool result = false;
				// Если ключ передан
				if(!key.empty()){
					/**
					 * Выполняем отлов ошибок
					 */
					try {
						// Выполняем чтение значения ключа
						string data = this->get <string> (root, key);
						// Если значение переменной не получено
						if(data.empty() && !root)
							// Проверяем ключ в переменных окружения
							data = this->env(key);
						// Если искомый ключ существует и нужно выполнить сравнение значений
						if((result = !data.empty()) && !val.empty())
							// Выполняем сравнение полученного значения ключа с переданным значением для проверки
							result = this->_fmk->compare(data, val);
					/**
					 * Если возникает ошибка
					 */
					} catch(const exception & error) {
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(root, key, val), log_t::flag_t::CRITICAL, error.what());
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
						#endif
					}
				}
				// Выводим результат
				return result;
			}
		private:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isType Метод проверки ключа записи на тип данных
			 * @param root флаг чтения из корневого раздела
			 * @param type тип элемента который необходимо проверить
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isType(const bool root, const type_t type, Args&&... args) const noexcept {
				// Результат работы функции
				bool result = false;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если это корневой элемент
								if(root){
									// Если ключ в базе данных существует
									if(this->_data.HasMember(key.c_str()))
										// Получаем значение родительского объекта
										item.CopyFrom(const_cast <Env *> (this)->_data[key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								// Если элемент дочерний
								} else if(this->_data.HasMember("work")) {
									// Если ключ в базе данных существует
									if(this->_data["work"].HasMember(key.c_str()))
										// Получаем значение родительского объекта
										item.CopyFrom(const_cast <Env *> (this)->_data["work"][key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
									// Если ключа в базе данных не найдено
									else {
										// Проверяем ключ в переменных окружения
										const string & value = this->env(key);
										// Если данные получены в переменных окружения
										if(!value.empty())
											// Выполняем добавление полученных значений переменной окружения
											const_cast <Env *> (this)->set(key, value);
										// Если ключ в базе данных существует
										if(this->_data["work"].HasMember(key.c_str()))
											// Получаем значение родительского объекта
											item.CopyFrom(const_cast <Env *> (this)->_data["work"][key.c_str()], const_cast <Env *> (this)->_data.GetAllocator());
										// Выходим из цикла
										else break;
									}
								}
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							if((i == (keys.size() - 1))){
								// Определяем тип запрашиваемого элемента
								switch(static_cast <uint8_t> (type)){
									// Если тип элемента является объектом
									case static_cast <uint8_t> (type_t::ENV_OBJECT):
										// Выполняем проверку типа элемента
										result = item.IsObject();
									break;
									// Если тип элемента является массивом
									case static_cast <uint8_t> (type_t::ENV_ARRAY):
										// Выполняем проверку типа элемента
										result = item.IsArray();
									break;
									// Если тип элемента является числом
									case static_cast <uint8_t> (type_t::ENV_NUMBER):
										// Выполняем проверку типа элемента
										result = item.IsNumber();
									break;
									// Если тип элемента является строкой
									case static_cast <uint8_t> (type_t::ENV_STRING):
										// Выполняем проверку типа элемента
										result = item.IsString();
									break;
									// Если тип элемента является булевым элементом
									case static_cast <uint8_t> (type_t::ENV_BOOLEAN):
										// Выполняем проверку типа элемента
										result = item.IsBool();
									break;
									// Если тип элемента является Double элементом
									case static_cast <uint8_t> (type_t::ENV_DOUBLE):
										// Выполняем проверку типа элемента
										result = item.IsDouble();
									break;
									// Если тип элемента является Float элементом
									case static_cast <uint8_t> (type_t::ENV_FLOAT):
										// Выполняем проверку типа элемента
										result = item.IsFloat();
									break;
									// Если тип элемента является Integer элементом
									case static_cast <uint8_t> (type_t::ENV_INTEGER):
										// Выполняем проверку типа элемента
										result = item.IsInt();
									break;
									// Если тип элемента является Unsigned Integer элементом
									case static_cast <uint8_t> (type_t::ENV_UINTEGER):
										// Выполняем проверку типа элемента
										result = item.IsUint();
									break;
									// Если тип элемента является Integer 64 элементом
									case static_cast <uint8_t> (type_t::ENV_INTEGER64):
										// Выполняем проверку типа элемента
										result = item.IsInt64();
									break;
									// Если тип элемента является Unsigned Integer 64 элементом
									case static_cast <uint8_t> (type_t::ENV_UINTEGER64):
										// Выполняем проверку типа элемента
										result = item.IsUint64();
									break;
								}
							}
						}
					// Выполняем проверку корневого элемента
					} else {
						// Если нужно выполнить поиск в корневом элементе
						if(root){
							// Определяем тип запрашиваемого элемента
							switch(static_cast <uint8_t> (type)){
								// Если тип элемента является объектом
								case static_cast <uint8_t> (type_t::ENV_OBJECT):
									// Выполняем проверку типа элемента
									result = this->_data.IsObject();
								break;
								// Если тип элемента является массивом
								case static_cast <uint8_t> (type_t::ENV_ARRAY):
									// Выполняем проверку типа элемента
									result = this->_data.IsArray();
								break;
								// Если тип элемента является числом
								case static_cast <uint8_t> (type_t::ENV_NUMBER):
									// Выполняем проверку типа элемента
									result = this->_data.IsNumber();
								break;
								// Если тип элемента является строкой
								case static_cast <uint8_t> (type_t::ENV_STRING):
									// Выполняем проверку типа элемента
									result = this->_data.IsString();
								break;
								// Если тип элемента является булевым элементом
								case static_cast <uint8_t> (type_t::ENV_BOOLEAN):
									// Выполняем проверку типа элемента
									result = this->_data.IsBool();
								break;
								// Если тип элемента является Double элементом
								case static_cast <uint8_t> (type_t::ENV_DOUBLE):
									// Выполняем проверку типа элемента
									result = this->_data.IsDouble();
								break;
								// Если тип элемента является Float элементом
								case static_cast <uint8_t> (type_t::ENV_FLOAT):
									// Выполняем проверку типа элемента
									result = this->_data.IsFloat();
								break;
								// Если тип элемента является Integer элементом
								case static_cast <uint8_t> (type_t::ENV_INTEGER):
									// Выполняем проверку типа элемента
									result = this->_data.IsInt();
								break;
								// Если тип элемента является Unsigned Integer элементом
								case static_cast <uint8_t> (type_t::ENV_UINTEGER):
									// Выполняем проверку типа элемента
									result = this->_data.IsUint();
								break;
								// Если тип элемента является Integer 64 элементом
								case static_cast <uint8_t> (type_t::ENV_INTEGER64):
									// Выполняем проверку типа элемента
									result = this->_data.IsInt64();
								break;
								// Если тип элемента является Unsigned Integer 64 элементом
								case static_cast <uint8_t> (type_t::ENV_UINTEGER64):
									// Выполняем проверку типа элемента
									result = this->_data.IsUint64();
								break;
							}
						// Если нужно выполнить поиск в рабочем объекте
						} else if(this->_data.HasMember("work")) {
							// Определяем тип запрашиваемого элемента
							switch(static_cast <uint8_t> (type)){
								// Если тип элемента является объектом
								case static_cast <uint8_t> (type_t::ENV_OBJECT):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsObject();
								break;
								// Если тип элемента является массивом
								case static_cast <uint8_t> (type_t::ENV_ARRAY):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsArray();
								break;
								// Если тип элемента является числом
								case static_cast <uint8_t> (type_t::ENV_NUMBER):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsNumber();
								break;
								// Если тип элемента является строкой
								case static_cast <uint8_t> (type_t::ENV_STRING):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsString();
								break;
								// Если тип элемента является булевым элементом
								case static_cast <uint8_t> (type_t::ENV_BOOLEAN):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsBool();
								break;
								// Если тип элемента является Double элементом
								case static_cast <uint8_t> (type_t::ENV_DOUBLE):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsDouble();
								break;
								// Если тип элемента является Float элементом
								case static_cast <uint8_t> (type_t::ENV_FLOAT):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsFloat();
								break;
								// Если тип элемента является Integer элементом
								case static_cast <uint8_t> (type_t::ENV_INTEGER):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsInt();
								break;
								// Если тип элемента является Unsigned Integer элементом
								case static_cast <uint8_t> (type_t::ENV_UINTEGER):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsUint();
								break;
								// Если тип элемента является Integer 64 элементом
								case static_cast <uint8_t> (type_t::ENV_INTEGER64):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsInt64();
								break;
								// Если тип элемента является Unsigned Integer 64 элементом
								case static_cast <uint8_t> (type_t::ENV_UINTEGER64):
									// Выполняем проверку типа элемента
									result = this->_data["work"].IsUint64();
								break;
							}
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isArray Метод проверки ключа записи на тип Array
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isArray(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_ARRAY, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isObject Метод проверки ключа записи на тип Object
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isObject(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_OBJECT, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isNumber Метод проверки ключа записи на тип Number
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isNumber(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_NUMBER, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isInt Метод проверки ключа записи на тип Integer
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isInt(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_INTEGER, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isUint Метод проверки ключа записи на тип Unsigned Integer
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isUint(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_UINTEGER, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isInt64 Метод проверки ключа записи на тип Integer 64
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isInt64(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_INTEGER64, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isUint64 Метод проверки ключа записи на тип Unsigned Integer 64
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isUint64(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_UINTEGER64, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isDouble Метод проверки ключа записи на тип Double
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isDouble(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_DOUBLE, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isFloat Метод проверки ключа записи на тип Float
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isFloat(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_FLOAT, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isString Метод проверки ключа записи на тип String
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isString(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_STRING, args...);
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename... Args>
			/**
			 * isBoolean Метод проверки ключа записи на тип Boolean
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     результат проверки
			 */
			bool isBoolean(const bool root, Args&&... args) const noexcept {
				// Выполняем проверку ключа записи
				return this->isType(root, type_t::ENV_BOOLEAN, args...);
			}
		private:
			/**
			 * get Метод извлечения записи в виде булевого значения
			 * @param item   объект для извлечения данных
			 * @param result полученный результат
			 */
			void get(const Value & item, bool & result) const noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Устанавливаем отрицательное значение результата
					result = false;
					// Если запись является булевым значением
					if(item.IsBool())
						// Выводим результат в виде булевого значения
						result = item.GetBool();
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T>
			/**
			 * get Метод извлечения записи в виде числа
			 * @param item   объект для извлечения данных
			 * @param result полученный результат
			 */
			void get(const Value & item, T & result) const noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Устанавливаем отрицательное значение результата
					result = 0;
					// Если запись является числом
					if(item.IsInt())
						// Выводим результат в виде числа
						result = item.GetInt();
					// Если запись является числом
					else if(item.IsUint())
						// Выводим результат в виде числа
						result = item.GetUint();
					// Если запись является числом
					else if(item.IsInt64())
						// Выводим результат в виде числа
						result = item.GetInt64();
					// Если запись является числом
					else if(item.IsUint64())
						// Выводим результат в виде числа
						result = item.GetUint64();
					// Если запись является числом с плавающей точкой
					else if(item.IsDouble())
						// Выводим результат в виде числа
						result = item.GetDouble();
					// Если запись является числом с плавающей точкой
					else if(item.IsFloat())
						// Выводим результат в виде числа
						result = item.GetFloat();
					// Если запись является булевым значением
					else if(item.IsBool())
						// Выводим результат в виде булевого значения
						result = static_cast <int8_t> (item.GetBool());
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
			/**
			 * get Метод извлечения записи в виде строки
			 * @param item   объект для извлечения данных
			 * @param result полученный результат
			 */
			void get(const Value & item, string & result) const noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Выполняем очистку результата
					result.clear();
					// Если запись является строкой
					if(item.IsString())
						// Выводим результат в виде строки
						result = item.GetString();
					// Если запись является массивом или объектом
					else if(item.IsArray() || item.IsObject()) {
						// Создаём результьрующий буфер
						StringBuffer data;
						// Выполняем очистку результирующего буфера
						data.Clear();
						// Выполняем создание объекта писателя
						Writer <StringBuffer> writer(data);
						// Передаем данные объекта JSON писателю
						item.Accept(writer);
						// Извлекаем созданную запись сктроки в формате JSON
						result = data.GetString();
					// Если запись является числом
					} else if(item.IsInt())
						// Выводим результат в виде числа
						result = std::to_string(item.GetInt());
					// Если запись является числом
					else if(item.IsUint())
						// Выводим результат в виде числа
						result = std::to_string(item.GetUint());
					// Если запись является числом
					else if(item.IsInt64())
						// Выводим результат в виде числа
						result = std::to_string(item.GetInt64());
					// Если запись является числом
					else if(item.IsUint64())
						// Выводим результат в виде числа
						result = std::to_string(item.GetUint64());
					// Если запись является числом с плавающей точкой
					else if(item.IsDouble())
						// Выводим результат в виде числа
						result = this->_fmk->noexp(item.GetDouble(), true);
					// Если запись является числом с плавающей точкой
					else if(item.IsFloat())
						// Выводим результат в виде числа
						result = this->_fmk->noexp(item.GetFloat(), true);
					// Если запись является булевым значением
					else if(item.IsBool())
						// Выводим результат в виде булевого значения
						result = (item.GetBool() ? "true" : "false");
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(result), log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
			/**
			 * get Метод извлечения записи в виде JSON
			 * @param item   объект для извлечения данных
			 * @param result полученный результат
			 */
			void get(const Value & item, json & result) const noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Очищаем объект JSON
					result = json();
					// Выполняем копирование полученного JSON
					result.CopyFrom(item, result.GetAllocator());
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T, typename... Args>
			/**
			 * get Метод извлечения значения ключа из базы данных
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     значение ключа в базе данных
			 */
			T get(const bool root, Args&&... args) const noexcept {
				// Результат работы функции
				T result;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если ключ в базе данных существует
								if(this->exist(root, key))
									// Получаем значение родительского объекта
									item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
								// Если ключа в базе данных не найдено
								else if(!root) {
									// Проверяем ключ в переменных окружения
									const string & value = this->env(key);
									// Если данные получены в переменных окружения
									if(!value.empty())
										// Выполняем добавление полученных значений переменной окружения
										const_cast <Env *> (this)->set(key, value);
									// Если ключ в базе данных существует
									if(this->exist(root, key))
										// Получаем значение родительского объекта
										item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								}
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							if(i == (keys.size() - 1))
								// Выполняем извлечение данных записи
								this->get(item, result);
						}
					// Если список ключей не получен
					} else {
						// Значение ключа в объекте
						Value item;
						// Получаем значение родительского объекта
						item.CopyFrom((root ? const_cast <Env *> (this)->_data : const_cast <Env *> (this)->_data["work"]), const_cast <Env *> (this)->_data.GetAllocator());
						// Выполняем извлечение данных записи
						this->get(item, result);
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
				// Выводим полученный результат
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T, typename... Args>
			/**
			 * arr Метод извлечения значения ключа из базы данных в виде массива
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     значение ключа в базе данных в виде массива
			 */
			const vector <T> arr(const bool root, Args&&... args) const noexcept {
				// Результат работы функции
				vector <T> result;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если ключ в базе данных существует
								if(this->exist(root, key))
									// Получаем значение родительского объекта
									item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
								// Если ключа в базе данных не найдено
								else if(!root) {
									// Проверяем ключ в переменных окружения
									const string & value = this->env(key);
									// Если данные получены в переменных окружения
									if(!value.empty())
										// Выполняем добавление полученных значений переменной окружения
										const_cast <Env *> (this)->set(key, value);
									// Если ключ в базе данных существует
									if(this->exist(root, key))
										// Получаем значение родительского объекта
										item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								}
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							if(i == (keys.size() - 1)){
								// Если элемент является простым элементом
								if(item.IsNumber() || item.IsBool() || item.IsString()){
									// Создаём объект данных
									T data;
									// Выполняем извлечение данных записи
									this->get(item, data);
									// Формируем список полученных значений
									result.push_back(std::move(data));
								// Если элемент является массивом
								} else if(item.IsArray() && !item.Empty()) {
									// Создаём объект данных
									T data;
									// Переходим по всему массиву
									for(auto & v : item.GetArray()){
										// Выполняем извлечение данных записи
										this->get(v, data);
										// Формируем список полученных значений
										result.push_back(std::move(data));
									}
								// Если элемент является объектом
								} else if(item.IsObject() && !item.ObjectEmpty()) {
									// Создаём объект данных
									T data;
									// Переходим по всему объекту
									for(auto & m : item.GetObj()){
										// Выполняем извлечение данных записи
										this->get(m.value, data);
										// Формируем список полученных значений
										result.push_back(std::move(data));
									}
								}
							}
						}
					// Если список ключей не указан
					} else if(this->_data.IsObject() && !this->_data.ObjectEmpty()) {
						// Если нужно выполнить поиск в корневом элементе
						if(root){
							// Создаём объект данных
							T data;
							// Переходим по всему объекту
							for(auto & m : this->_data.GetObj()){
								// Выполняем извлечение данных записи
								this->get(m.value, data);
								// Формируем список полученных значений
								result.push_back(std::move(data));
							}
						// Если нужно выполнить поиск в рабочем объекте
						} else if(this->_data.HasMember("work")) {
							// Создаём объект данных
							T data;
							// Переходим по всему объекту
							for(auto & m : this->_data["work"].GetObj()){
								// Выполняем извлечение данных записи
								this->get(m.value, data);
								// Формируем список полученных значений
								result.push_back(std::move(data));
							}
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * Шаблон извлечения нужного типа данных
			 */
			template <typename T, typename... Args>
			/**
			 * obj Метод извлечения значения ключа из базы данных в виде объекта
			 * @param root флаг чтения из корневого раздела
			 * @param args ключи для извлечения
			 * @return     значение ключа в базе данных в виде объекта
			 */
			const unordered_map <string, T> obj(const bool root, Args&&... args) const noexcept {
				// Результат работы функции
				unordered_map <string, T> result;
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Получаем список ключей
					const vector <string> & keys = {args...};
					// Если количество ключей получено
					if(!keys.empty()){
						// Значение ключа в объекте
						Value item;
						// Выполняем перебор всех аргументов функции
						for(size_t i = 0; i < keys.size(); i++){
							// Поулчаем название ключа
							const string & key = keys.at(i);
							// Если это первый элемент в списке
							if(i == 0){
								// Если ключ в базе данных существует
								if(this->exist(root, key))
									// Получаем значение родительского объекта
									item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
								// Если ключа в базе данных не найдено
								else if(!root) {
									// Проверяем ключ в переменных окружения
									const string & value = this->env(key);
									// Если данные получены в переменных окружения
									if(!value.empty())
										// Выполняем добавление полученных значений переменной окружения
										const_cast <Env *> (this)->set(key, value);
									// Если ключ в базе данных существует
									if(this->exist(root, key))
										// Получаем значение родительского объекта
										item.CopyFrom((root ? const_cast <Env *> (this)->_data[key.c_str()] : const_cast <Env *> (this)->_data["work"][key.c_str()]), const_cast <Env *> (this)->_data.GetAllocator());
									// Выходим из цикла
									else break;
								}
							// Если ключ существует в объекте
							} else if(item.IsObject() && !item.ObjectEmpty() && item.HasMember(key.c_str()))
								// Получаем значение текущего ключа
								item = item[key.c_str()];
							// Выходим из цикла
							else break;
							// Если мы перебрали все ключи
							if(i == (keys.size() - 1)){
								// Если элемент является простым элементом
								if(item.IsNumber() || item.IsBool() || item.IsString()){
									// Создаём объект данных
									T data;
									// Выполняем извлечение данных записи
									this->get(item, data);
									// Формируем список полученных значений
									result.emplace("item", std::move(data));
								// Если элемент является массивом
								} else if(item.IsArray() && !item.Empty()) {
									// Создаём объект данных
									T data;
									// Индекс текущего элемента
									size_t index = 0;
									// Переходим по всему массиву
									for(auto & v : item.GetArray()){
										// Выполняем извлечение данных записи
										this->get(v, data);
										// Формируем список полученных значений
										result.emplace(std::to_string(index++), std::move(data));
									}
								// Если элемент является объектом
								} else if(item.IsObject() && !item.ObjectEmpty()) {
									// Создаём объект данных
									T data;
									// Переходим по всему объекту
									for(auto & m : item.GetObj()){
										// Выполняем извлечение данных записи
										this->get(m.value, data);
										// Формируем список полученных значений
										result.emplace(m.name.GetString(), std::move(data));
									}
								}
							}
						}
					// Если список ключей не указан
					} else if(this->_data.IsObject() && !this->_data.ObjectEmpty()) {
						// Если нужно выполнить поиск в корневом элементе
						if(root){
							// Создаём объект данных
							T data;
							// Переходим по всему объекту
							for(auto & m : this->_data.GetObj()){
								// Выполняем извлечение данных записи
								this->get(m.value, data);
								// Формируем список полученных значений
								result.emplace(m.name.GetString(), std::move(data));
							}
						// Если нужно выполнить поиск в рабочем объекте
						} else if(this->_data.HasMember("work")) {
							// Создаём объект данных
							T data;
							// Переходим по всему объекту
							for(auto & m : this->_data["work"].GetObj()){
								// Выполняем извлечение данных записи
								this->get(m.value, data);
								// Формируем список полученных значений
								result.emplace(m.name.GetString(), std::move(data));
							}
						}
					}
				/**
				 * Если возникает ошибка
				 */
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
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
				if(config.IsObject() && !config.ObjectEmpty()){
					/**
					 * Выполняем отлов ошибок
					 */
					try {
						// Переходим по всем ключам и добавляем всё в базу данных
						for(auto & m : config.GetObj()){
							// Если мы нашли рабочий раздел, пропускаем его
							if(this->_fmk->compare(m.name.GetString(), "work"))
								// Пропускаем итерацию перебора
								continue;
							// Если такой ключ не существует
							else if(!this->_data.HasMember(m.name.GetString()))
								// Устанавливаем данные конфига в виде строки
								this->_data.AddMember(Value(m.name, this->_data.GetAllocator()).Move(), Value(m.value, this->_data.GetAllocator()).Move(), this->_data.GetAllocator());
						}
						// Если рабочий раздел существует
						if(config.HasMember("work")){
							// Если ключ work не обнаружен
							if(!this->_data.HasMember("work"))
								// Выполняем добавление ключа work
								this->_data.AddMember(Value("work", this->_data.GetAllocator()).Move(), Value(kObjectType).Move(), this->_data.GetAllocator());
							// Переходим по всем ключам и добавляем всё в базу данных
							for(auto & m : config["work"].GetObj()){
								// Если такой ключ не существует
								if(!this->_data["work"].HasMember(m.name.GetString()))
									// Устанавливаем данные конфига
									this->_data["work"].AddMember(Value(m.name, this->_data.GetAllocator()).Move(), Value(m.value, this->_data.GetAllocator()).Move(), this->_data.GetAllocator());
							}
						}
					/**
					 * Если возникает ошибка
					 */
					} catch(const exception & error) {
						/**
						 * Если включён режим отладки
						 */
						#if defined(DEBUG_MODE)
							// Выводим сообщение об ошибке
							this->_log->debug("%s", __PRETTY_FUNCTION__, {}, log_t::flag_t::CRITICAL, error.what());
						/**
						* Если режим отладки не включён
						*/
						#else
							// Выводим сообщение об ошибке
							this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
						#endif
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
					// Создаём объект конфигурационного файла
					json data(kObjectType);
					// Выполняем парсинг объекта JSON
					if(data.Parse(config.c_str(), config.length()).HasParseError())
						// Выводим сообщение об ошибке
						this->_log->print("\"Env:%s\": (offset %d): %s", log_t::flag_t::CRITICAL, __FUNCTION__, data.GetErrorOffset(), GetParseError_En(data.GetParseError()));
					// Выполняем установку полученных данных конфигурационного файла
					else this->config(data);
				// Если возникает ошибка
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(config), log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
				}
			}
		public:
			/**
			 * filename Метод загрузки данных JSON из файла
			 * @param filename адрес файла для загрузки
			 */
			void filename(const string & filename) noexcept {
				/**
				 * Выполняем отлов ошибок
				 */
				try {
					// Если файл найден
					if(this->_fs.isFile(filename)){
						// Считываем конфигурационный файл из файловой системы
						const auto & config = this->_fs.read(filename);
						// Если данные получены, устанавливаем их
						if(!config.empty()){
							// Создаём объект конфигурационного файла
							json data(kObjectType);
							// Выполняем парсинг объекта JSON
							if(data.Parse(config.data(), config.size()).HasParseError())
								// Выводим сообщение об ошибке
								this->_log->print("\"Env:%s\": (offset %d): %s", log_t::flag_t::CRITICAL, __FUNCTION__, data.GetErrorOffset(), GetParseError_En(data.GetParseError()));
							// Выполняем установку полученных данных конфигурационного файла
							else this->config(data);
						}
					}
				// Если возникает ошибка
				} catch(const exception & error) {
					/**
					 * Если включён режим отладки
					 */
					#if defined(DEBUG_MODE)
						// Выводим сообщение об ошибке
						this->_log->debug("%s", __PRETTY_FUNCTION__, make_tuple(filename), log_t::flag_t::CRITICAL, error.what());
					/**
					* Если режим отладки не включён
					*/
					#else
						// Выводим сообщение об ошибке
						this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
					#endif
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
							this->set(key, val);
					// Если это относительное значение переменной
					} else if(arg.front() == '-') {
						// Получаем ключ
						key = arg.substr(1, arg.length() - 1);
						// Устанавливаем полученное значение
						this->set(key, "true");
						// Устанавливаем ожидание значения
						isValue = true;
					// Если это ожидание значения
					} else if(isValue) {
						// Убираем ожидание значения
						isValue = false;
						// Добавляем полученные данные в список переменных
						if(!key.empty() && this->exist(false, key))
							// Выполняем добавление значения в базу данных
							this->set(key, arg);
					}
				}
				// Если переменная текста установлена и мы её из не получили
				if(!this->_text.empty() && !this->exist(false, this->_text)){
					// Очищаем значение
					val.clear();
					// Считываем строку из буфера stdin
					if(!::isatty(STDIN_FILENO) || (this->_automatic && this->env(this->_text).empty()))
						// Выполняем чтение из коммандной строки
						getline(cin, val);
					// Добавляем полученные данные в список переменных
					if(!val.empty())
						// Добавляем полученное значение
						this->set(this->_text, val);
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
							this->set(this->_fmk->convert(key), this->_fmk->convert(val));
					// Если это относительное значение переменной
					} else if(arg.front() == L'-') {
						// Получаем ключ
						key = arg.substr(1, arg.length() - 1);
						// Устанавливаем полученное значение
						this->set(this->_fmk->convert(key), "true");
						// Устанавливаем ожидание значения
						isValue = true;
					// Если это ожидание значения
					} else if(isValue) {
						// Убираем ожидание значения
						isValue = false;
						// Выполняем конвертацию ключа
						const string & item = this->_fmk->convert(key);
						// Добавляем полученные данные в список переменных
						if(!key.empty() && this->exist(false, item))
							// Выполняем добавление значения в базу данных
							this->set(item, this->_fmk->convert(arg));
					}
				}
				// Если переменная текста установлена и мы её из не получили
				if(!this->_text.empty() && !this->exist(false, this->_text)){
					// Значение считываемое из потока
					string value = "";
					// Если операционной системой является Windows
					#if defined(_WIN32) || defined(_WIN64)
						// Считываем строку из буфера stdin
						if(this->_automatic && this->env(this->_text).empty())
							// Выполняем чтение из коммандной строки
							getline(cin, value);
					// Для всех остальных операционных систем
					#else
						// Считываем строку из буфера stdin
						if(!::isatty(STDIN_FILENO) || (this->_automatic && this->env(this->_text).empty()))
							// Выполняем чтение из коммандной строки
							getline(cin, value);
					#endif
					// Добавляем полученные данные в список переменных
					if(!value.empty())
						// Добавляем полученное значение
						this->set(this->_text, value);
				}
			}
		public:
			/**
			 * operator Создаём оператор извлечения JSON объекта
			 * @return конфигурационные данные
			 */
			operator const json & () const noexcept {
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
			 _fs(fmk, log), _data(kObjectType), _automatic(false),
			 _text{""}, _prefix{ACU_SHORT_NAME}, _fmk(fmk), _log(log) {}
			/**
			 * Env Конструктор
			 * @param prefix префикс переменной окружения
			 * @param fmk    объект фреймворка
			 * @param log    объект для работы с логами
			 */
			Env(const string & prefix, const fmk_t * fmk, const log_t * log) noexcept :
			 _fs(fmk, log), _data(kObjectType), _automatic(false), _text{""}, _prefix{prefix}, _fmk(fmk), _log(log) {
				// Переводим префикс в верхний регистр
				this->_fmk->transform(this->_prefix, fmk_t::transform_t::UPPER);
			}
			/**
			 * Env Конструктор
			 * @param prefix префикс переменной окружения
			 * @param text   название переменной для извлечения текстовой информации из потока (если параметром не передана)
			 * @param fmk    объект фреймворка
			 * @param log    объект для работы с логами
			 */
			Env(const string & prefix, const string & text, const fmk_t * fmk, const log_t * log) noexcept :
			 _fs(fmk, log), _data(kObjectType), _automatic(false), _text{text}, _prefix{prefix}, _fmk(fmk), _log(log) {
				// Переводим префикс в верхний регистр
				this->_fmk->transform(this->_prefix, fmk_t::transform_t::UPPER);
			}
	} env_t;
};

#endif // __ANYKS_ACU_ENV__
