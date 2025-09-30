/**
 * @file: csv.hpp
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

#ifndef __ANYKS_ACU_CSV__
#define __ANYKS_ACU_CSV__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <functional>

/**
 * Модули AWH
 */
#include <awh/sys/fs.hpp>
#include <awh/sys/fmk.hpp>
#include <awh/sys/log.hpp>

/**
 * Подключаем заголовочные файлы JSON
 */
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

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
	 * @brief Класс модуля CSV
	 *
	 */
	typedef class ACU_SHARED_EXPORT CSV {
		private:
			// Флаг разрешения использования заголовков
			bool _header;
		private:
			// Объект работы с файловой системой
			fs_t _fs;
		private:
			// Схема соответствий ключей расширения
			vector <vector <string>> _mapping;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		private:
			/**
			 * Максимальный размер буфера данных на чтение из файла
			 */
			static constexpr uintmax_t CSV_BUFFER_SIZE = 0x10000;
		private:
			/**
			 * @brief Метод выполнения препарирования полученных данных строки
			 *
			 * @param buffer буфер данных для препарирования
			 * @param size   размер буфера данных для препарирования
			 * @param delim  используемый разделитель
			 */
			void prepare(const char * buffer, const size_t size, const char delim = ',') noexcept;
			/**
			 * @brief Метод выполнения препарирования полученных данных строки
			 *
			 * @param buffer   буфер данных для препарирования
			 * @param size     размер буфера данных для препарирования
			 * @param callback функция обратного вызова
			 * @param delim    используемый разделитель
			 */
			void prepare(const char * buffer, const size_t size, function <void (const vector <string> &)> callback, const char delim = ',') noexcept;
		public:
			/**
			 * @brief Метод очистки данных
			 *
			 */
			void clear() noexcept;
		public:
			/**
			 * @brief Метод установки флага использования заголовков
			 *
			 * @param mode флаг использования заголовков
			 */
			void header(const bool mode) noexcept;
		public:
			/**
			 * @brief Метод выполнения парсинга текста
			 *
			 * @param text текст для парсинга
			 */
			void parse(const string & text) noexcept;
			/**
			 * @brief Метод выполнения парсинга текста
			 *
			 * @param text  текст для парсинга
			 * @param delim используемый разделитель
			 */
			void parse(const string & text, const char delim) noexcept;
		public:
			/**
			 * @brief Метод получения количества столбцов
			 *
			 * @return количество столбцов
			 */
			size_t cols() const noexcept;
			/**
			 * @brief Метод получения количества строк
			 *
			 * @return количество строк
			 */
			size_t rows() const noexcept;
		public:
			/**
			 * @brief Метод получения строки
			 *
			 * @param index индекс строки
			 * @param delim используемый разделитель
			 * @return      сформированная строка
			 */
			string row(const size_t index, const char delim = ';') noexcept;
		public:
			/**
			 * @brief Метод записи данных в файл
			 *
			 * @param filename адрес файла контейнера CSV для записи
			 * @param delim    используемый разделитель
			 */
			void write(const string & filename, const char delim = ';') noexcept;
		public:
			/**
			 * @brief Метод чтения данных из файла
			 *
			 * @param filename адрес файла контейнера CSV для чтения
			 */
			void read(const string & filename) noexcept;
			/**
			 * @brief Метод чтения данных из файла
			 *
			 * @param filename адрес файла контейнера CSV для чтения
			 * @param delim    используемый разделитель
			 */
			void read(const string & filename, const char delim) noexcept;
			/**
			 * @brief Метод чтения данных из файла
			 *
			 * @param filename адрес файла контейнера CSV для чтения
			 * @param callback функция обратного вызова
			 * @param delim    используемый разделитель
			 */
			void read(const string & filename, function <void (const vector <string> &)> callback, const char delim = '0') noexcept;
		public:
			/**
			 * @brief Метод создания дампа данных
			 *
			 * @return дамп данных в формате JSON
			 */
			json dump() const noexcept;
			/**
			 * @brief Метод установки дампа данных
			 *
			 * @param dump дамп данных в формате JSON
			 */
			void dump(const json & dump) noexcept;
		public:
			/**
			 * @brief Метод извлечения данных контейнера
			 *
			 * @return собранные данные контейнера
			 */
			const vector <vector <string>> & get() const noexcept;
		public:
			/**
			 * @brief Оператор вывода данные контейнера в качестве строки
			 *
			 * @return данные контейнера в качестве строки
			 */
			operator string() const noexcept;
		public:
			/**
			 * @brief Оператор [=] присвоения контейнеров
			 *
			 * @param csv контенер для присвоения
			 * @return    текущий объект
			 */
			CSV & operator = (const CSV & csv) noexcept;
			/**
			 * @brief Оператор [=] присвоения контейнеров
			 *
			 * @param csv контенер для присвоения
			 * @return    текущий объект
			 */
			CSV & operator = (const string & csv) noexcept;
		public:
			/**
			 * @brief Конструктор
			 *
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			CSV(const fmk_t * fmk, const log_t * log) noexcept : _header(false), _fs(fmk, log), _fmk(fmk), _log(log) {}
			/**
			 * @brief Деструктор
			 *
			 */
			~CSV() noexcept {}
	} csv_t;
	/**
	 * @brief Оператор [>>] чтения из потока CSV контейнера
	 *
	 * @param is  поток для чтения
	 * @param csv контенер для присвоения
	 */
	ACU_SHARED_EXPORT istream & operator >> (istream & is, csv_t & csv) noexcept;
	/**
	 * @brief Оператор [<<] вывода в поток CSV контейнера
	 *
	 * @param os  поток куда нужно вывести данные
	 * @param csv контенер для присвоения
	 */
	ACU_SHARED_EXPORT ostream & operator << (ostream & os, const csv_t & csv) noexcept;
};

#endif // __ANYKS_ACU_CSV__
