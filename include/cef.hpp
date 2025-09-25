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
 * @copyright: Copyright © 2025
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
#include <awh/sys/fmk.hpp>
#include <awh/sys/log.hpp>
#include <awh/net/net.hpp>
#include <awh/sys/chrono.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>

/**
 * @brief пространство имён
 *
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
	 * @brief Класс модуля CEF
	 *
	 */
	typedef class ACUSHARED_EXPORT Cef {
		private:
			/**
			 * Устанавливаем формат даты и времени по умолчанию
			 */
			static constexpr char FORMAT[] = "%b %d %Y %H:%M:%S %Z";
		public:
			/**
			 * Флаги режимов парсинга
			 */
			enum class mode_t : uint8_t {
				NONE   = 0x00, // Все соответствия отключены
				LOW    = 0x01, // Соответствие ключей простых типов данных
				MEDIUM = 0x02, // Соответствие ключей и простых типов данных
				STRONG = 0x03  // Соответствие ключей и всех типов данных
			};
			/**
			 * Типы расширения
			 */
			enum class type_t : uint8_t {
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
			 * @brief Структура разсширения
			 *
			 */
			typedef struct Extension {
				type_t type; // Тип расширения
				size_t size; // Размер расширения
				string name; // Название переменной
				string desc; // Описание расширения
				/**
				 * @brief Конструктор
				 *
				 */
				Extension() noexcept : type(type_t::NONE), size(0), name{""}, desc{""} {}
				/**
				 * @brief Конструктор
				 *
				 * @param name название переменной
				 * @param desc описание переменной
				 * @param type тип расширения
				 * @param size размер расширения
				 */
				Extension(const string & name, const string & desc, const type_t type, const size_t size = 0) noexcept :
				 type(type), size(size), name{name}, desc{desc}  {}
			} ext_t;
		public:
			/**
			 * @brief Структура важности события
			 *
			 */
			typedef struct Severity {
				// Уровень важности
				uint8_t level;
				// Название важности
				char name[10];
				/**
				 * @brief Конструктор
				 *
				 */
				Severity() noexcept : level(0) {
					// Заполняем нулями буфер названия важности
					::memset(this->name, 0, sizeof(this->name));
					// Устанавливаем уровень важности
					::memcpy(this->name, "Low", 3);
				}
			} __attribute__((packed)) sev_t;
			/**
			 * @brief Основные параметры события
			 *
			 */
			typedef struct Event {
				sev_t severity;         // Важность события
				char name[512];         // Название события
				char devVendor[63];     // Поставщик данных
				char devVersion[31];    // Версия устройства
				char devProduct[63];    // Тип устройства
				char signatureId[1023]; // Подпись события
				/**
				 * @brief Конструктор
				 *
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
			string _header;
		private:
			// Формат даты
			string _format;
		private:
			// Параметры события
			event_t _event;
		private:
			// Объект работы с датой и временем
			chrono_t _chrono;
		private:
			// Объект работы с IP-адресами
			mutable net_t _net;
		private:
			// Схема соответствий ключей расширения
			std::unordered_map <string, string> _mapping;
		private:
			// Схема расширений для SEFv0
			std::unordered_map <string, ext_t> _extensionSEFv0;
			// Схема расширений для SEFv1
			std::unordered_map <string, ext_t> _extensionSEFv1;
		private:
			// Расширения контейнера в бинарном виде
			std::unordered_map <string, vector <char>> _extensions;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		private:
			/**
			 * @brief Шаблон метода записи числовых данных в контейнер
			 *
			 * @tparam T тип данных для записи в контейнер
			 */
			template <typename T>
			/**
			 * @brief Метод записи числовых данных в контейнер
			 *
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void _set(const string & key, const T value) noexcept;
			/**
			 * @brief Метод записи строковых данных в контейнер
			 *
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void _set(const string & key, const string & value) noexcept;
		public:
			/**
			 * @brief Шаблон метода записи данных в контейнер
			 *
			 * @tparam T тип данных для записи в контейнер
			 */
			template <typename T>
			/**
			 * @brief Метод записи данных в контейнер
			 *
			 * @param key   ключ записи
			 * @param value значение для добавления
			 */
			void set(const string & key, const T value) noexcept;
		public:
			/**
			 * @brief Шаблон метода чтения данных из контейнера
			 *
			 * @tparam T тип данных для извлечения из контейнера
			 */
			template <typename T>
			/**
			 * @brief Метод чтения данных из контейнера
			 *
			 * @param key ключ записи
			 * @return    результат работы функции
			 */
			T get(const string & key) noexcept;
			/**
			 * @brief Шаблон метода чтения данных из контейнера
			 *
			 * @tparam T тип данных для извлечения из контейнера
			 */
			template <typename T>
			/**
			 * @brief Метод чтения данных из контейнера
			 *
			 * @param key      ключ записи
			 * @param response значение по умолчанию
			 * @return         результат работы функции
			 */
			T get(const string & key, T response) noexcept;
		public:
			/**
			 * @brief Метод очистки данных
			 *
			 */
			void clear() noexcept;
		public:
			/**
			 * @brief Метод парсинга строки в формате CEF
			 *
			 * @param cef строка в формате CEF
			 */
			void parse(const string & cef) noexcept;
			/**
			 * @brief Метод препарирования расширений
			 *
			 * @param extensions строка с расширениями
			 */
			void prepare(const string & extensions) noexcept;
		public:
			/**
			 * @brief Метод установки режима парсинга
			 *
			 * @param mode режим парсинга для установки
			 */
			void mode(const mode_t mode) noexcept;
		public:
			/**
			 * @brief Метод извлечения версии контейнера
			 *
			 * @return версия контейнера
			 */
			double version() const noexcept;
			/**
			 * @brief Метод установки версии контейнера
			 *
			 * @param version версия контейнера для установки
			 */
			void version(const double version) noexcept;
		public:
			/**
			 * @brief Метод получения данных в формате CEF
			 *
			 * @return данные в формате CEF
			 */
			string cef() const noexcept;
		public:
			/**
			 * @brief Метод извлечения данных в виде JSON
			 *
			 * @return json объект дампа данных
			 */
			json dump() const noexcept;
			/**
			 * @brief Метод установки данных в формате JSON
			 *
			 * @param dump данные в формате JSON
			 */
			void dump(const json & dump) noexcept;
		public:
			/**
			 * @brief Метод извлечения заголовка
			 *
			 * @return заголовок контейнера
			 */
			const string & header() const noexcept;
			/**
			 * @brief Метод установки заголовка контейнера
			 *
			 * @param header заголовок контейнера
			 */
			void header(const string & header) noexcept;
		public:
			/**
			 * @brief Метод извлечения события
			 *
			 * @return событие контейнера
			 */
			const event_t & event() const noexcept;
			/**
			 * @brief Метод установки события
			 *
			 * @param event данные события
			 */
			void event(const event_t & event) noexcept;
		public:
			/**
			 * @brief Метод установки формата даты
			 *
			 * @param format формат даты
			 */
			void format(const string & format = FORMAT) noexcept;
		public:
			/**
			 * @brief Метод извлечения типа ключа
			 *
			 * @param key ключ для извлечения типа расширения
			 * @return    тип данных которому соответствует ключ
			 */
			type_t type(const string & key) const noexcept;
		public:
			/**
			 * @brief Метод получения списка событий
			 *
			 * @return список полученных событий
			 */
			std::unordered_map <string, string> events() const noexcept;
			/**
			 * @brief Метод извлечения списка расширений
			 *
			 * @return список установленных расширений
			 */
			std::unordered_map <string, string> extensions() const noexcept;
		public:
			/**
			 * @brief Метод извлечения расширения в бинарном виде
			 *
			 * @param key ключ для извлечения расширения
			 * @return    данные расширения в бинарном виде
			 */
			const vector <char> & extension(const string & key) const noexcept;
			/**
			 * @brief Метод установки расширения в бинарном виде
			 *
			 * @param key   ключ расширения
			 * @param value значение расширения
			 */
			void extension(const string & key, const string & value) noexcept;
			/**
			 * @brief Метод установки расширения в бинарном виде
			 *
			 * @param key   ключ расширения
			 * @param value значение расширения
			 */
			void extension(const string & key, const vector <char> & value) noexcept;
		public:
			/**
			 * @brief Оператор вывода данные контейнера в качестве строки
			 *
			 * @return данные контейнера в качестве строки
			 */
			operator string() const noexcept;
		public:
			/**
			 * @brief Оператор [!=] сравнения контейнеров
			 *
			 * @param cef контенер для сравнения
			 * @return    результат сравнения
			 */
			bool operator != (const Cef & cef) const noexcept;
			/**
			 * @brief Оператор [==] сравнения контейнеров
			 *
			 * @param cef контенер для сравнения
			 * @return    результат сравнения
			 */
			bool operator == (const Cef & cef) const noexcept;
		public:
			/**
			 * @brief Оператор [=] присвоения контейнеров
			 *
			 * @param cef контенер для присвоения
			 * @return    текущий объект
			 */
			Cef & operator = (const Cef & cef) noexcept;
			/**
			 * @brief Оператор [=] присвоения режима парсинга
			 *
			 * @param mode режим парсинга для установки
			 * @return     текущий объект
			 */
			Cef & operator = (const mode_t mode) noexcept;
			/**
			 * @brief Оператор [=] присвоения контейнеров
			 *
			 * @param cef контенер для присвоения
			 * @return    текущий объект
			 */
			Cef & operator = (const string & cef) noexcept;
		public:
			/**
			 * @brief Конструктор
			 *
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Cef(const fmk_t * fmk, const log_t * log) noexcept;
	} cef_t;
	/**
	 * @brief Оператор [>>] чтения из потока CEF контейнера
	 *
	 * @param is  поток для чтения
	 * @param cef контенер для присвоения
	 */
	ACUSHARED_EXPORT istream & operator >> (istream & is, cef_t & cef) noexcept;
	/**
	 * @brief Оператор [<<] вывода в поток CEF контейнера
	 *
	 * @param os  поток куда нужно вывести данные
	 * @param cef контенер для присвоения
	 */
	ACUSHARED_EXPORT ostream & operator << (ostream & os, const cef_t & cef) noexcept;
};

#endif // __ANYKS_ACU_CEF__
