/**
 * @file: cef.hpp
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

#ifndef __ANYKS_ACU_CEF__
#define __ANYKS_ACU_CEF__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <ctime>
#include <stack>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <unordered_map>

/**
 * Модули AWH
 */
#include <sys/fmk.hpp>
#include <sys/log.hpp>
#include <net/net.hpp>
#include <sys/reg.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Активируем пространство имён json
	 */
	using json = rapidjson::Document;
	/**
	 * Cef Класс модуля CEF
	 */
	typedef class ACUSHARED_EXPORT Cef {
		private:
			// Устанавливаем формат даты и времени по умолчанию
			static constexpr char FORMAT[] = "%b %d %Y %H:%M:%S %Z";
		public:
			/**
			 * Флаги режимов парсинга
			 */
			enum class mode_t : std::uint8_t {
				NONE   = 0x00, // Все соответствия отключены
				LOW    = 0x01, // Соответствие ключей простых типов данных
				MEDIUM = 0x02, // Соответствие ключей и простых типов данных
				STRONG = 0x03  // Соответствие ключей и всех типов данных
			};
			/**
			 * Типы расширения
			 */
			enum class type_t : std::uint8_t {
				NONE      = 0x00, // Тип не установлен
				MAC       = 0x01, // Мак адрес
				IP        = 0x02, // Адрес IP (IPv4 или IPv6)
				IPV4      = 0x03, // Адрес IPv4
				IPV6      = 0x04, // Адрес IPv6
				LONG      = 0x05, // Тип данных Long
				INT32     = 0x06, // Тип данных Int32
				INT64     = 0x07, // Тип данных Int64
				FLOAT     = 0x08, // Тип данных Float
				DOUBLE    = 0x09, // Тип данных Double
				STRING    = 0x0A, // Тип данных String
				TIMESTAMP = 0x0B  // Штамп врмени
			};
		private:
			/**
			 * Extension Структура разсширения
			 */
			typedef struct Extension {
				type_t type; // Тип расширения
				std::size_t size; // Размер расширения
				std::string name; // Название переменной
				std::string desc; // Описание расширения
				/**
				 * Extension Конструктор
				 */
				Extension() noexcept : type(type_t::NONE), size(0), name{""}, desc{""} {}
				/**
				 * Extension Конструктор
				 * @param name название переменной
				 * @param desc описание переменной
				 * @param type тип расширения
				 * @param size размер расширения
				 */
				Extension(const std::string & name, const std::string & desc, const type_t type, const std::size_t size = 0) noexcept :
				 type(type), size(size), name{name}, desc{desc}  {}
			} ext_t;
		public:
			/**
			 * Severity Структура важности события
			 */
			typedef struct Severity {
				// Уровень важности
				std::uint8_t level;
				// Название важности
				char name[10];
				/**
				 * Severity Конструктор
				 */
				Severity() noexcept : level(0) {
					// Заполняем нулями буфер названия важности
					::memset(this->name, 0, sizeof(this->name));
					// Устанавливаем уровень важности
					::memcpy(this->name, "Low", 3);
				}
			} __attribute__((packed)) sev_t;
			/**
			 * Event Основные параметры события
			 */
			typedef struct Event {
				sev_t severity;         // Важность события
				char name[512];         // Название события
				char devVendor[63];     // Поставщик данных
				char devVersion[31];    // Версия устройства
				char devProduct[63];    // Тип устройства
				char signatureId[1023]; // Подпись события
				/**
				 * Event Конструктор
				 */
				Event() noexcept {
					// Заполняем нулями буфер названия события
					::memset(this->name, 0, sizeof(this->name));
					// Заполняем нулями буфер поставщик данных
					::memset(this->devVendor, 0, sizeof(this->devVendor));
					// Заполняем нулями буфер версии устройства
					::memset(this->devVersion, 0, sizeof(this->devVersion));
					// Заполняем нулями буфер типа устройства
					::memset(this->devProduct, 0, sizeof(this->devProduct));
					// Заполняем нулями буфер подпись события
					::memset(this->signatureId, 0, sizeof(this->signatureId));
				}
			} __attribute__((packed)) event_t;
		private:
			// Флаг соответствия
			mode_t _mode;
		private:
			// Версия контейнера
			double _version;
		private:
			// Заголовок записи
			std::string _header;
		private:
			// Формат даты
			std::string _format;
		private:
			// Параметры события
			event_t _event;
		private:
			// Объект работы с регулярными выражениями
			awh::regexp_t _reg;
			// Регулярное выражение для парсинга расширений
			awh::regexp_t::exp_t _exp;
		private:
			// Схема соответствий ключей расширения
			std::unordered_map <std::string, std::string> _mapping;
		private:
			// Схема расширений для SEFv0
			std::unordered_map <std::string, ext_t> _extensionSEFv0;
			// Схема расширений для SEFv1
			std::unordered_map <std::string, ext_t> _extensionSEFv1;
		private:
			// Расширения контейнера в бинарном виде
			std::unordered_map <std::string, std::vector <char>> _extensions;
		private:
			// Объект фреймворка
			const awh::fmk_t * _fmk;
			// Объект работы с логами
			const awh::log_t * _log;
		private:
			/**
			 * @tparam Шаблон метода записи числовых данных в контейнер
			 */
			template <typename T>
			/**
			 * _set Метод записи числовых данных в контейнер
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void _set(const std::string & key, T value) noexcept {
				// Выполняем поиск указанного ключа в списке расширений
				auto i = this->_extensions.find(key);
				// Если ключ расширения найдено, удаляем его
				if(i != this->_extensions.end())
					// Удаляем переданное расширение
					this->_extensions.erase(i);
				// Выполняем добавление расширение
				this->extension(key, std::to_string(value));
			}
			/**
			 * _set Метод записи строковых данных в контейнер
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void _set(const std::string & key, const std::string & value) noexcept {
				// Выполняем поиск указанного ключа в списке расширений
				auto i = this->_extensions.find(key);
				// Если ключ расширения найдено, удаляем его
				if(i != this->_extensions.end())
					// Удаляем переданное расширение
					this->_extensions.erase(i);
				// Выполняем добавление расширение
				this->extension(key, value);
			}
		public:
			/**
			 * @tparam Шаблон метода записи данных в контейнер
			 */
			template <typename T>
			/**
			 * set Метод записи данных в контейнер
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void set(const std::string & key, T value) noexcept {
				// Если данные являются основными
				if(std::is_class <T>::value || std::is_integral <T>::value || std::is_floating_point <T>::value){
					// Выполняем поиск соответствие нашему ключу
					auto i = this->_mapping.find(key);
					// Если соответствие ключу найдено
					if(i != this->_mapping.end())
						// Добавляем полученные данные в контейнер
						this->_set(i->second, value);
					// Если мы получили непонятный ключ и включён не строгий режим
					else if(this->_mode == mode_t::NONE)
						// Добавляем полученные данные в контейнер
						this->_set(key, value);
				}
			}
		public:
			/**
			 * @tparam Шаблон метода чтения данных из контейнера
			 */
			template <typename T>
			/**
			 * get Метод чтения данных из контейнера
			 * @param key ключ записи
			 * @return    результат работы функции
			 */
			T get(const std::string & key) noexcept {
				// Результат работы функции
				T result;
				// Если ключ передан
				if(!key.empty() && !this->_extensions.empty() && std::is_class <T>::value){
					// Если режим парсинга установлен
					if(this->_mode != mode_t::NONE){
						// Выполняем поиск соответствие нашему ключу
						auto i = this->_mapping.find(key);
						// Если соответствие ключу найдено
						if(i != this->_mapping.end()){
							// Выполняем поиск данных нашего ключа
							auto j = this->_extensions.find(i->second);
							// Если данные запрашиваемого ключа получены
							if(j != this->_extensions.end()){
								// Создаём объект параметров расширения
								const ext_t * params = nullptr;
								// Выполняем поиск параметров ключа
								auto i = this->_extensionSEFv0.find(j->first);
								// Если параметры ключа найдены
								if(i != this->_extensionSEFv0.end())
									// Получаем параметры ключа
									params = &i->second;
								// Если параметры ключа не найдены и версия протокола №1
								else if(this->_version > .0) {
									// Выполняем поиск параметров ключа
									auto i = this->_extensionSEFv1.find(j->first);
									// Если параметры ключа найдены
									if(i != this->_extensionSEFv0.end())
										// Получаем параметры ключа
										params = &i->second;
								}
								// Если параметры ключа получены
								if(params != nullptr){
									// Определяем тип ключа
									switch(static_cast <std::uint8_t> (params->type)){
										// Если тип ключа является IP-адресом
										case static_cast <std::uint8_t> (type_t::IP): {
											// Если включён строгий режим парсинга
											if(this->_mode == mode_t::STRONG){
												// Создаём объект сети
												awh::net_t net(this->_log);
												// Если количество байт в буфере 4
												if(j->second.size() == 4){
													// Формируем число из бинарного буфера
													std::uint32_t value = 0;
													// Копируем в бинарный буфер данные IP адреса
													::memcpy(&value, j->second.data(), j->second.size());
													// Устанавливаем данные адреса в объект сети
													net.v4(value);
												// Если количество байт в буфере 16
												} else if(j->second.size() == 16){
													// Формируем бинарный буфер данных
													std::array <std::uint64_t, 2> buffer;
													// Копируем в бинарный буфер данные IP адреса
													::memcpy(buffer.data(), j->second.data(), j->second.size());
													// Устанавливаем данные адреса в объект сети
													net.v6(buffer);
												}
												// Извлекаем данные IP адреса
												const std::string & ip = net.get();
												// Устанавливаем значение ключа
												result.assign(ip.begin(), ip.end());
											// Если строгий режим парсинга не активирован, устанавливаем значение ключа
											} else result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является MAC-адресом
										case static_cast <std::uint8_t> (type_t::MAC): {
											// Если включён строгий режим парсинга
											if(this->_mode == mode_t::STRONG){
												// Создаём объект сети
												awh::net_t net(this->_log);
												// Формируем число из бинарного буфера
												std::uint64_t value = 0;
												// Копируем в бинарный буфер данные IP адреса
												::memcpy(&value, j->second.data(), j->second.size());
												// Устанавливаем данные адреса в объект сети
												net.mac(value);
												// Извлекаем данные MAC адреса
												const std::string & mac = net.get();
												// Устанавливаем значение ключа
												result.assign(mac.begin(), mac.end());
											// Если строгий режим парсинга не активирован, устанавливаем значение ключа
											} else result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является IPV4-адресом
										case static_cast <std::uint8_t> (type_t::IPV4): {
											// Если включён строгий режим парсинга
											if(this->_mode == mode_t::STRONG){
												// Создаём объект сети
												awh::net_t net(this->_log);
												// Формируем число из бинарного буфера
												std::uint32_t value = 0;
												// Копируем в бинарный буфер данные IP адреса
												::memcpy(&value, j->second.data(), j->second.size());
												// Устанавливаем данные адреса в объект сети
												net.v4(value);
												// Извлекаем данные IP адреса
												const std::string & ip = net.get();
												// Устанавливаем значение ключа
												result.assign(ip.begin(), ip.end());
											// Если строгий режим парсинга не активирован, устанавливаем значение ключа
											} else result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является IPV6-адресом
										case static_cast <std::uint8_t> (type_t::IPV6): {
											// Если включён строгий режим парсинга
											if(this->_mode == mode_t::STRONG){
												// Создаём объект сети
												awh::net_t net(this->_log);
												// Формируем бинарный буфер данных
												std::array <std::uint64_t, 2> buffer;
												// Копируем в бинарный буфер данные IP адреса
												::memcpy(buffer.data(), j->second.data(), j->second.size());
												// Устанавливаем данные адреса в объект сети
												net.v6(buffer);
												// Извлекаем данные IP адреса
												const std::string & ip = net.get();
												// Устанавливаем значение ключа
												result.assign(ip.begin(), ip.end());
											// Если строгий режим парсинга не активирован, устанавливаем значение ключа
											} else result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является LONG
										case static_cast <std::uint8_t> (type_t::LONG): {
											// Если включён простой режим парсинга
											if(this->_mode == mode_t::LOW)
												// Устанавливаем значение ключа
												result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является INT32
										case static_cast <std::uint8_t> (type_t::INT32): {
											// Если включён простой режим парсинга
											if(this->_mode == mode_t::LOW)
												// Устанавливаем значение ключа
												result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является INT64
										case static_cast <std::uint8_t> (type_t::INT64): {
											// Если включён простой режим парсинга
											if(this->_mode == mode_t::LOW)
												// Устанавливаем значение ключа
												result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является FLOAT
										case static_cast <std::uint8_t> (type_t::FLOAT): {
											// Если включён простой режим парсинга
											if(this->_mode == mode_t::LOW)
												// Устанавливаем значение ключа
												result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является DOUBLE
										case static_cast <std::uint8_t> (type_t::DOUBLE): {
											// Если включён простой режим парсинга
											if(this->_mode == mode_t::LOW)
												// Устанавливаем значение ключа
												result.assign(j->second.begin(), j->second.end());
										} break;
										// Если тип ключа является STRING
										case static_cast <std::uint8_t> (type_t::STRING):
											// Устанавливаем значение ключа
											result.assign(j->second.begin(), j->second.end());
										break;
										// Если тип ключа является TIMESTAMP
										case static_cast <std::uint8_t> (type_t::TIMESTAMP): {
											// Если включён строгий режим парсинга
											if(this->_mode == mode_t::STRONG){
												// Если формат даты установлен
												if(!this->_format.empty()){
													// Формируем число из бинарного буфера
													std::time_t date = 0;
													// Извлекаем из буфера данные числа
													::memcpy(&date, j->second.data(), j->second.size());
													// Создаём объект потока
													std::stringstream transTime;
													// Создаем структуру времени
													std::tm * tm = ::localtime(&date);
													// Выполняем извлечение даты
													transTime << std::put_time(tm, this->_format.c_str());
													// Устанавливаем значение ключа
													result = transTime.str();
												}
											// Если строгий режим парсинга не активирован, устанавливаем значение ключа
											} else result.assign(j->second.begin(), j->second.end());
										} break;
									}
								}
							}
						}
					// Если режим парсинга не установлен
					} else {
						// Выполняем поиск данных нашего ключа
						auto i = this->_extensions.find(key);
						// Если данные запрашиваемого ключа получены
						if(i != this->_extensions.end())
							// Устанавливаем значение ключа
							result.assign(i->second.begin(), i->second.end());
					}
				}
				// Выводим результат
				return result;
			}
			/**
			 * @tparam Шаблон метода чтения данных из контейнера
			 */
			template <typename T>
			/**
			 * get Метод чтения данных из контейнера
			 * @param key      ключ записи
			 * @param response значение по умолчанию
			 * @return         результат работы функции
			 */
			T get(const std::string & key, T response) noexcept {
				// Результат работы функции
				T result = response;
				// Если ключ передан
				if(!key.empty() && (std::is_integral <T>::value || std::is_floating_point <T>::value)){
					// Если режим парсинга установлен
					if((this->_mode == mode_t::STRONG) || (this->_mode == mode_t::MEDIUM)){
						// Выполняем поиск соответствие нашему ключу
						auto i = this->_mapping.find(key);
						// Если соответствие ключу найдено
						if(i != this->_mapping.end()){
							// Выполняем поиск данных нашего ключа
							auto j = this->_extensions.find(i->second);
							// Если данные запрашиваемого ключа получены
							if(j != this->_extensions.end()){
								// Создаём объект параметров расширения
								const ext_t * params = nullptr;
								// Выполняем поиск параметров ключа
								auto i = this->_extensionSEFv0.find(j->first);
								// Если параметры ключа найдены
								if(i != this->_extensionSEFv0.end())
									// Получаем параметры ключа
									params = &i->second;
								// Если параметры ключа не найдены и версия протокола №1
								else if(this->_version > .0) {
									// Выполняем поиск параметров ключа
									auto i = this->_extensionSEFv1.find(j->first);
									// Если параметры ключа найдены
									if(i != this->_extensionSEFv0.end())
										// Получаем параметры ключа
										params = &i->second;
								}
								// Если параметры ключа получены
								if(params != nullptr){
									// Определяем тип ключа
									switch(static_cast <std::uint8_t> (params->type)){
										// Если тип ключа является LONG
										case static_cast <std::uint8_t> (type_t::LONG): {
											// Формируем число из бинарного буфера
											long value = 0;
											// Извлекаем из буфера данные числа
											::memcpy(&value, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = value;
										} break;
										// Если тип ключа является INT32
										case static_cast <std::uint8_t> (type_t::INT32): {
											// Формируем число из бинарного буфера
											std::int32_t value = 0;
											// Извлекаем из буфера данные числа
											::memcpy(&value, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = value;
										} break;
										// Если тип ключа является INT64
										case static_cast <std::uint8_t> (type_t::INT64): {
											// Формируем число из бинарного буфера
											std::int64_t value = 0;
											// Извлекаем из буфера данные числа
											::memcpy(&value, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = value;
										} break;
										// Если тип ключа является FLOAT
										case static_cast <std::uint8_t> (type_t::FLOAT): {
											// Формируем число из бинарного буфера
											float value = .0f;
											// Извлекаем из буфера данные числа
											::memcpy(&value, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = value;
										} break;
										// Если тип ключа является DOUBLE
										case static_cast <std::uint8_t> (type_t::DOUBLE): {
											// Формируем число из бинарного буфера
											double value = .0;
											// Извлекаем из буфера данные числа
											::memcpy(&value, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = value;
										} break;
										// Если тип ключа является TIMESTAMP
										case static_cast <std::uint8_t> (type_t::TIMESTAMP): {
											// Формируем число из бинарного буфера
											std::time_t date = 0;
											// Извлекаем из буфера данные числа
											::memcpy(&date, j->second.data(), j->second.size());
											// Устанавливаем значение ключа
											result = date;
										} break;
									}
								}
							}
						}
					}
				}
				// Выводим результат
				return result;
			}
		public:
			/**
			 * clear Метод очистки данных
			 */
			void clear() noexcept;
		public:
			/**
			 * parse Метод парсинга строки в формате CEF
			 * @param cef строка в формате CEF
			 */
			void parse(const std::string & cef) noexcept;
			/**
			 * prepare Метод препарирования расширений
			 * @param extensions строка с расширениями
			 */
			void prepare(const std::string & extensions) noexcept;
		public:
			/**
			 * mode Метод установки режима парсинга
			 * @param mode режим парсинга для установки
			 */
			void mode(const mode_t mode) noexcept;
		public:
			/**
			 * version Метод извлечения версии контейнера
			 * @return версия контейнера
			 */
			double version() const noexcept;
			/**
			 * version Метод установки версии контейнера
			 * @param version версия контейнера для установки
			 */
			void version(const double version) noexcept;
		public:
			/**
			 * cef Метод получения данных в формате CEF
			 * @return данные в формате CEF
			 */
			std::string cef() const noexcept;
		public:
			/**
			 * dump Метод извлечения данных в виде JSON
			 * @return json объект дампа данных
			 */
			json dump() const noexcept;
			/**
			 * dump Метод установки данных в формате JSON
			 * @param dump данные в формате JSON
			 */
			void dump(const json & dump) noexcept;
		public:
			/**
			 * header Метод извлечения заголовка
			 * @return заголовок контейнера
			 */
			const std::string & header() const noexcept;
			/**
			 * header Метод установки заголовка контейнера
			 * @param header заголовок контейнера
			 */
			void header(const std::string & header) noexcept;
		public:
			/**
			 * event Метод извлечения события
			 * @return событие контейнера
			 */
			const event_t & event() const noexcept;
			/**
			 * event Метод установки события
			 * @param event данные события
			 */
			void event(const event_t & event) noexcept;
		public:
			/**
			 * format Метод установки формата даты
			 * @param format формат даты
			 */
			void format(const std::string & format = FORMAT) noexcept;
		public:
			/**
			 * type Метод извлечения типа ключа
			 * @param key ключ для извлечения типа расширения
			 * @return    тип данных которому соответствует ключ
			 */
			type_t type(const std::string & key) const noexcept;
		public:
			/**
			 * events Метод получения списка событий
			 * @return список полученных событий
			 */
			std::unordered_map <std::string, std::string> events() const noexcept;
			/**
			 * extensions Метод извлечения списка расширений
			 * @return список установленных расширений
			 */
			std::unordered_map <std::string, std::string> extensions() const noexcept;
		public:
			/**
			 * extension Метод извлечения расширения в бинарном виде
			 * @param key ключ для извлечения расширения
			 * @return    данные расширения в бинарном виде
			 */
			const std::vector <char> & extension(const std::string & key) const noexcept;
			/**
			 * extension Метод установки расширения в бинарном виде
			 * @param key   ключ расширения
			 * @param value значение расширения
			 */
			void extension(const std::string & key, const std::string & value) noexcept;
			/**
			 * extension Метод установки расширения в бинарном виде
			 * @param key   ключ расширения
			 * @param value значение расширения
			 */
			void extension(const std::string & key, const std::vector <char> & value) noexcept;
		public:
			/**
			 * Оператор вывода данные контейнера в качестве строки
			 * @return данные контейнера в качестве строки
			 */
			operator std::string() const noexcept;
		public:
			/**
			 * Оператор [!=] сравнения контейнеров
			 * @param cef контенер для сравнения
			 * @return    результат сравнения
			 */
			bool operator != (const Cef & cef) const noexcept;
			/**
			 * Оператор [==] сравнения контейнеров
			 * @param cef контенер для сравнения
			 * @return    результат сравнения
			 */
			bool operator == (const Cef & cef) const noexcept;
		public:
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param cef контенер для присвоения
			 * @return    текущий объект
			 */
			Cef & operator = (const Cef & cef) noexcept;
			/**
			 * Оператор [=] присвоения режима парсинга
			 * @param mode режим парсинга для установки
			 * @return     текущий объект
			 */
			Cef & operator = (const mode_t mode) noexcept;
			/**
			 * Оператор [=] присвоения контейнеров
			 * @param cef контенер для присвоения
			 * @return    текущий объект
			 */
			Cef & operator = (const std::string & cef) noexcept;
		public:
			/**
			 * Cef Конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Cef(const awh::fmk_t * fmk, const awh::log_t * log) noexcept;
	} cef_t;
	/**
	 * Оператор [>>] чтения из потока CEF контейнера
	 * @param is  поток для чтения
	 * @param cef контенер для присвоения
	 */
	ACUSHARED_EXPORT std::istream & operator >> (std::istream & is, cef_t & cef) noexcept;
	/**
	 * Оператор [<<] вывода в поток CEF контейнера
	 * @param os  поток куда нужно вывести данные
	 * @param cef контенер для присвоения
	 */
	ACUSHARED_EXPORT std::ostream & operator << (std::ostream & os, const cef_t & cef) noexcept;
};

#endif // __ANYKS_ACU_CEF__
