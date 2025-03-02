/**
 * @file: lib.hpp
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

#ifndef __ACU_CONFIG__
#define __ACU_CONFIG__

// Версия приложения
#define ACU_VERSION "1.0.8"
// Короткое название библиотеки
#define ACU_SHORT_NAME "ACU"
// Название библиотеки
#define ACU_NAME "ANYKS - conversion utility"
// Основной хост системы
#define ACU_HOST "anyks.com"
// Телеграм-контакт
#define ACU_CONTACT "@forman"
// Адрес сайта автора
#define ACU_SITE "https://acu.anyks.com"
// Адрес электронной почты
#define ACU_EMAIL "info@anyks.com"

// Порт сервера по умолчанию
#define ACU_SERVER_PORT 2222
// Хост сервера по умолчанию
#define ACU_SERVER_HOST "127.0.0.1"
// Имя пользователя по умолчанию
#define ACU_SERVER_USERNAME "admin"
// Пароль пользователя по умолчанию
#define ACU_SERVER_PASSWORD "admin"

/**
 * Если операционной системой является Windows
 */
#if defined(_WIN32) || defined(_WIN64)
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH ""
	// Адрес конфигурационного файла
	#define ACU_CONFIG { \
		"config.json", \
		"%ProgramFiles%\\acu\\config.json" \
	}
/**
 * Если операционной системой является MacOS X
 */
#elif __APPLE__ || __MACH__
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH "/var/run"
	// Адрес конфигурационного файла
	#define ACU_CONFIG { \
		"./config.json", \
		"~/acu/config.json", \
		"~/.acu/config.json", \
		"/usr/local/etc/acu/config.json" \
	}
/**
 * Если операционной системой является Linux
 */
#elif __linux__
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH "/var/run"
	// Адрес конфигурационного файла
	#define ACU_CONFIG { \
		"./config.json", \
		"~/acu/config.json", \
		"~/.acu/config.json", \
		"/etc/acu/config.json", \
		"/usr/local/etc/acu/config.json" \
	}
/**
 * Если операционной системой является FreeBSD
 */
#elif __FreeBSD__
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH "/var/run"
	// Адрес конфигурационного файла
	#define ACU_CONFIG { \
		"./config.json", \
		"~/acu/config.json", \
		"~/.acu/config.json", \
		"/usr/local/etc/acu/config.json" \
	}
/**
 * Для всех остальных Unix-подобных операционных систем
 */
#elif __unix || __unix__
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH "/var/run"
	// Адрес конфигурационного файла
	#define ACU_CONFIG {}
/**
 * Для всех остальных операционных систем
 */
#else
	// Адрес каталога с PID файлами
	#define ACU_PID_PATH ""
	// Адрес конфигурационного файла
	#define ACU_CONFIG {}
#endif

#endif // __ACU_CONFIG__
