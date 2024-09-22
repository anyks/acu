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
	const string msg = "\r\n\x1B[32m\x1B[1musage:\x1B[0m acu [-V | --version] [-H | --info] "
	"[-script <value> | --script=<value>] [<args>]\r\n\r\n\r\n"
	"\x1B[34m\x1B[1m[FLAGS]\x1B[0m\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Flag to stop the trading script: \x1B[1m[-stop | --stop]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Flag to pause the trading script: \x1B[1m[-pause | --pause]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Flag for starting the trading script: \x1B[1m[-start | --start]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display user permissions information: \x1B[1m[-user | --user]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display a list of available broker scripts: \x1B[1m[-brokers | --brokers]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display a list of available trading scripts: \x1B[1m[-scripts | --scripts]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display a list of available extension scripts: \x1B[1m[-extensions | --extensions]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display application version: \x1B[1m[-version | --version | -V]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Display information about available application functions: \x1B[1m[-info | --info | -H]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m[ARGS]\x1B[0m\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Activating the selected trading script: \x1B[1m[-script <value> | --script=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Set trading script language: \x1B[1m[-lang <value> | --lang=<value>]\x1B[0m\r\n"
	"\x1B[32m\x1B[1m  -\x1B[0m (UA | RU | EN | DE | FR | CN)\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m File address for recording trade logs (if required): \x1B[1m[-log <value> | --log=<value>]\x1B[0m\r\n\r\n"
	"\x1B[34m\x1B[1m*\x1B[0m Configuration file address: \x1B[1m[-config <value> | --config=<value>]\x1B[0m\r\n\r\n";
	// Выводим сообщение справки
	cout << msg << endl;
}
/**
 * version Функция вывода версии приложения
 * @param fmk     объект фреймворка
 * @param log     объект для работы с логами
 * @param address адрес приложения
 */
static void version(const fmk_t * fmk, const log_t * log, const char * address) noexcept {
	// Если входящие данные переданы
	if((fmk != nullptr) && (log != nullptr) && (address != nullptr)){
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
		// Создаём формат даты
		string formatDate = DATE_FORMAT;
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
		
		// Выводим удачное завершение работы
		// return EXIT_FAILURE;
}
