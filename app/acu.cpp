/**
 * @file: acu.cpp
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

/**
 * Подключаем модули парсера
 */
#include <lib.hpp>
#include <env.hpp>
#include <parser.hpp>

/**
 * Подключаем модуль файловой системы
 */
#include <sys/fs.hpp>

// Подключаем пространство имён
using namespace anyks;

/**
 * help Функция вывода справки
 */
static void help() noexcept {
	// Формируем строку справки
	const string msg = "\r\n\x1B[32m\x1B[1musage:\x1B[0m acu [-V | --version] [-H | --info] [<args>]\r\n\r\n\r\n"
	"\x1B[34m\x1B[1m[FLAGS]\x1B[0m\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Flag for generating headers when parsing CSV files: \x1B[1m[-header | --header]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Flag for generating a readable file format (XML or JSON): \x1B[1m[-pretty | --pretty]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display application version: \x1B[1m[-version | --version | -V]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display information about available application functions: \x1B[1m[-info | --info | -H]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m[ARGS]\x1B[0m\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Separator for parsing CSV files (default: \";\"): \x1B[1m[-delim <value> | --delim=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Log generation date format (if required): \x1B[1m[-formatDate <value> | --formatDate=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m CEF file parsing mode: \x1B[1m[-cef <value> | --cef=<value>]\x1B[0m\r\n"
	"\x1B[32m\x1B[1m  -\x1B[0m (LOW | MEDIUM | STRONG)\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m File format to which the writing is made: \x1B[1m[-to <value> | --to=<value>]\x1B[0m\r\n"
	"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG)\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Format of the file from which reading is performed: \x1B[1m[-from <value> | --from=<value>]\x1B[0m\r\n"
	"\x1B[32m\x1B[1m  -\x1B[0m (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | GROK)\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Directory address for saving converted files: \x1B[1m[-dest <value> | --dest=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m File address for writing trade logs (if required): \x1B[1m[-log <value> | --log=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Address of the file or directory with files to convert: \x1B[1m[-src <value> | --src=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Address of the file in JSON format with GROK templates: \x1B[1m[-patterns <value> | --patterns=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Address of the file with the regular expression in GROK format: \x1B[1m[-express <value> | --express=<value>]\x1B[0m\r\n\r\n";
	// Выводим сообщение справки
	cout << msg << endl;
}
/**
 * version Функция вывода версии приложения
 * @param fmk     объект фреймворка
 * @param log     объект для работы с логами
 * @param address адрес приложения
 */
static void version(const fmk_t * fmk, const log_t * log, const string & address) noexcept {
	// Если входящие данные переданы
	if((fmk != nullptr) && (log != nullptr) && !address.empty()){
		// Название операционной системы
		const char * os = nullptr;
		// Определяем название операционной системы
		switch(static_cast <uint8_t> (os_t().type())){
			// Если операционной системой является Unix
			case static_cast <uint8_t> (os_t::type_t::UNIX):
				// Устанавливаем название Операционной Системы
				os = "Unix";
			break;
			// Если операционной системой является Linux
			case static_cast <uint8_t> (os_t::type_t::LINUX):
				// Устанавливаем название Операционной Системы
				os = "Linux";
			break;
			// Если операционной системой является неизвестной
			case static_cast <uint8_t> (os_t::type_t::NONE):
				// Устанавливаем название Операционной Системы
				os = "Unknown";
			break;
			// Если операционной системой является Windows
			case static_cast <uint8_t> (os_t::type_t::WIND32):
			case static_cast <uint8_t> (os_t::type_t::WIND64):
				// Устанавливаем название Операционной Системы
				os = "Windows";
			break;
			// Если операционной системой является MacOS X
			case static_cast <uint8_t> (os_t::type_t::MACOSX):
				// Устанавливаем название Операционной Системы
				os = "MacOS X";
			break;
			// Если операционной системой является FreeBSD
			case static_cast <uint8_t> (os_t::type_t::FREEBSD):
				// Устанавливаем название Операционной Системы
				os = "FreeBSD";
			break;
		}
		// Если операционная система обнаружена
		if(os != nullptr){
			// Позиция в каталоге
			size_t pos = 0;
			// Объект работы с файловой системой
			fs_t fs(fmk, log);
			// Определяем адрес приложения
			string app = fs.realPath(address);
			// Ищем каталог
			if((pos = app.rfind(FS_SEPARATOR)) != string::npos)
				// Получаем название приложения
				app = app.substr(0, pos);
			// Выводим версию приложения
			printf(
				"\r\n%s %s (built: %s %s)\r\n"
				"awh: %s\r\n"
				"target: %s\r\n"
				"installed dir: %s\r\n\r\n*\r\n"
				"* site:     %s\r\n"
				"* email:    %s\r\n"
				"* telegram: %s\r\n*\r\n\r\n",
				ACU_NAME,
				ACU_VERSION,
				__DATE__,
				__TIME__,
				AWH_VERSION,
				os, app.c_str(),
				ACU_SITE,
				ACU_EMAIL,
				ACU_CONTACT
			);
		// Выводим сообщение об ошибке
		} else log->print("Operating system is not identified", log_t::flag_t::CRITICAL);
	}
}
/**
 * read Функция записи данных в файл
 * @param filename адрес файла для чтения
 * @param data     данные для записи в файл
 * @param fmk      объект фреймворка
 * @param log      объект для работы с логами
 */
static void write(const string & filename, const string & data, const fmk_t * fmk, const log_t * log) noexcept {
	// Если данные переданы
	if(!filename.empty() && !data.empty() && (fmk != nullptr) && (log != nullptr)){
		/**
		 * Выполняем перехват ошибок
		 */
		try {
			/**
			 * Выполняем работу для Windows
			 */
			#if defined(_WIN32) || defined(_WIN64)
				// Выполняем открытие файла на запись
				HANDLE file = CreateFileW(fmk->convert(filename).c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				// Если открыть файл открыт нормально
				if(file != INVALID_HANDLE_VALUE){
					// Выполняем запись данных в файл
					WriteFile(file, static_cast <LPCVOID> (data.data()), static_cast <DWORD> (data.size()), 0, nullptr);
					// Выполняем закрытие файла
					CloseHandle(file);
				}
			/**
			 * Выполняем работу для Unix
			 */
			#else
				// Файловый поток для записи
				ofstream file(filename, ios::binary);
				// Если файл открыт на запись
				if(file.is_open()){
					// Выполняем запись данных в файл
					file.write(data.data(), data.size());
					// Закрываем файл
					file.close();
				}
			#endif
		/**
		 * Если возникает ошибка
		 */
		} catch (const std::ios_base::failure & error) {
			// Выводим сообщение инициализации метода класса скрипта торговой платформы
			log->print("%s for filename %s", log_t::flag_t::CRITICAL, error.what(), filename.c_str());
		}
	}
}
/**
 * read Функция чтения данных из файла
 * @param filename адрес файла для чтения
 * @param fs       объект работы с файловой системой
 * @param fmk      объект фреймворка
 * @param log      объект для работы с логами
 * @return         данные прочитанные из файла
 */
static string read(const string & filename, const fs_t * fs, const fmk_t * fmk, const log_t * log) noexcept {
	// Результат работы функции
	string result = "";
	// Если данные переданы
	if(!filename.empty() && (fs != nullptr) && (fmk != nullptr) && (log != nullptr)){
		/**
		 * Выполняем перехват ошибок
		 */
		try {
			/**
			 * Выполняем работу для Windows
			 */
			#if defined(_WIN32) || defined(_WIN64)
				// Создаём объект работы с файлом
				HANDLE file = CreateFileW(fmk->convert(filename).c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				// Если открыть файл открыт нормально
				if(file != INVALID_HANDLE_VALUE){
					// Устанавливаем размер буфера
					result.resize(static_cast <uintmax_t> (GetFileSize(file, nullptr)), 0);
					// Выполняем чтение из файла в буфер данные
					ReadFile(file, static_cast <LPVOID> (result.data()), static_cast <DWORD> (result.size()), 0, nullptr);
					// Выполняем закрытие файла
					CloseHandle(file);
				}
			/**
			 * Выполняем работу для Unix
			 */
			#else
				// Получаем размер файла
				const size_t size = fs->size(filename);
				// Если файл не пйстой
				if(size > 0){
					// Открываем файл на чтение
					ifstream file(filename, ios::in);
					// Если файл открыт на запись
					if(file.is_open()){
						// Устанавливаем размер буфера
						result.resize(size, 0);
						// Выполняем чтение данных из файла
						file.read(result.data(), size);
						// Закрываем файл
						file.close();
					}
				}
			#endif
		/**
		 * Если возникает ошибка
		 */
		} catch (const std::ios_base::failure & error) {
			// Выводим сообщение инициализации метода класса скрипта торговой платформы
			log->print("%s for filename %s", log_t::flag_t::CRITICAL, error.what(), filename.c_str());
		}
	}
	// Выводим результат
	return result;
}
/**
 * Выполняем работу для Windows
 */
#if defined(_WIN32) || defined(_WIN64)
	/**
	 * main Главная функция приложения
	 * @param count  длина массива параметров
	 * @param params массив параметров
	 * @return       код выхода из приложения
	 */
	int32_t wmain(int32_t count, const wchar_t * params[]) noexcept {
/**
 * Выполняем работу для Unix
 */
#else
	/**
	 * main Главная функция приложения
	 * @param count  длина массива параметров
	 * @param params массив параметров
	 * @return       код выхода из приложения
	 */
	int32_t main(int32_t count, const char * params[]) noexcept {
#endif
		// Создаём объект фреймворка
		fmk_t fmk;
		// Создаём объект для работы с логами
		log_t log(&fmk);
		// Объект работы с файловой системой
		fs_t fs(&fmk, &log);
		// Создаём объект переменных окружения
		env_t env(ACU_SHORT_NAME, "text", &fmk, &log);
		// Устанавливаем название сервиса
		log.name(ACU_SHORT_NAME);
		// Текстовое значение полученное из потока
		string text = "";
		// Создаём формат даты
		string formatDate = DATE_FORMAT;
		// Если операционной системой является Windows
		#if defined(_WIN32) || defined(_WIN64)
			// Выполняем чтение из коммандной строки
			std::getline(cin, text);
		// Для всех остальных операционных систем
		#else
			// Считываем строку из буфера stdin
			if(!::isatty(STDIN_FILENO))
				// Выполняем чтение из коммандной строки
				std::getline(cin, text);
		#endif
		/**
		 * Выполняем работу для Windows
		 */
		#if defined(_WIN32) || defined(_WIN64)
			// Выполняем инициализацию переданных параметров
			env.init(reinterpret_cast <const wchar_t **> (params), static_cast <uint8_t> (count));
		/**
		 * Выполняем работу для Unix
		 */
		#else
			// Выполняем инициализацию переданных параметров
			env.init(reinterpret_cast <const char **> (params), static_cast <uint8_t> (count));
		#endif
		// Если формат вывода лога передан
		if(env.is("formatDate", true))
			// Получаем формат вывода даты
			formatDate = env.get("formatDate", true);
		// Устанавливаем формат вывода даты
		log.format(formatDate);
		// Если нужно вывести справочную помощь
		if(!env.size() || (env.is("info") || env.is("H"))){
			// Выводим справочную информацию
			help();
			// Выходим из приложения
			::exit(EXIT_SUCCESS);
		// Если версия получена
		} else if(env.is("version") || env.is("V")) {
			/**
			 * Выполняем работу для Windows
			 */
			#if defined(_WIN32) || defined(_WIN64)
				// Выводим версию приложения
				version(&fmk, &log, fmk.convert(wstring(params[0])));
			/**
			 * Выполняем работу для Unix
			 */
			#else
				// Выводим версию приложения
				version(&fmk, &log, params[0]);
			#endif
			// Выходим из приложения
			::exit(EXIT_SUCCESS);
		}

		// Выполняем инициализацию объекта парсера
		parser_t parser(&fmk, &log);

		// Выводим удачное завершение работы
		return EXIT_SUCCESS;
}
