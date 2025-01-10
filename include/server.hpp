/**
 * @file: server.hpp
 * @date: 2024-10-06
 *
 * @telegram: @forman
 * @author: Yuriy Lobarev
 * @phone: +7 (910) 983-95-90
 * @email: forman@anyks.com
 * @site: https://anyks.com
 *
 * @copyright: Copyright © 2024
 */

#ifndef __ACU_SERVER__
#define __ACU_SERVER__

/**
 * Стандартные модули
 */
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

/**
 * Модули AWH
 */
#include <client/awh.hpp>
#include <server/awh.hpp>

/**
 * Наши модули
 */
#include <lib.hpp>
#include <parser.hpp>

/**
 * anyks пространство имён
 */
namespace anyks {
	/**
	 * Активируем пространство имён json
	 */
	using json = rapidjson::Document;
	/**
	 * Server Класс сервера
	 */
	typedef class ACUSHARED_EXPORT Server {
		private:
			// Тип рабочего формата данных
			enum class type_t : std::uint8_t {
				TEXT        = 0x00, // Тип формата - текстовый
				XML         = 0x01, // Тип формата - XML
				INI         = 0x02, // Тип формата - INI
				CEF         = 0x03, // Тип формата - CEF
				CSV         = 0x04, // Тип формата - CSV
				JSON        = 0x05, // Тип формата - JSON
				YAML        = 0x06, // Тип формата - YAML
				GROK        = 0x07, // Тип формата - GROK
				SYSLOG      = 0x08, // Тип формата - SYSLOG
				BASE64      = 0x09, // Тип формата - Base64
				MD5         = 0x0A, // Тип формата - MD5
				SHA1        = 0x0B, // Тип формата - SHA1
				SHA224      = 0x0C, // Тип формата - SHA224
				SHA256      = 0x0D, // Тип формата - SHA256
				SHA384      = 0x0E, // Тип формата - SHA384
				SHA512      = 0x0F, // Тип формата - SHA512
				HMAC_MD5    = 0x10, // Тип формата - HMAC MD5
				HMAC_SHA1   = 0x11, // Тип формата - HMAC SHA1
				HMAC_SHA224 = 0x12, // Тип формата - HMAC SHA224
				HMAC_SHA256 = 0x13, // Тип формата - HMAC SHA256
				HMAC_SHA384 = 0x14, // Тип формата - HMAC SHA384
				HMAC_SHA512 = 0x15  // Тип формата - HMAC SHA512
			};
		private:
			/**
			 * Bandwidth Структура параметров пропускной способности сети
			 */
			typedef struct Bandwidth {
				std::string read;  // Ширина канала на чтение
				std::string write; // Ширина канала на запись
				/**
				 * Bandwidth Конструктор
				 */
				Bandwidth() noexcept : read{"100Mbps"}, write{"100Mbps"} {}
			} bandwidth_t;
		private:
			// Объект работы с файловой системой
			awh::fs_t _fs;
			// Объект работы с URI параметрами
			awh::uri_t _uri;
			// Объект работы с хэшированием
			awh::hash_t _hash;
		private:
			// Адрес корневого каталога с сайтом
			std::string _root;
			// Адрес файла по умолчанию
			std::string _index;
			// Адрес сайта которому разрешён доступ к ресурсу
			std::string _origin;
			// Адрес хранения favicon.ico
			std::string _favicon;
		private:
			// Объект работы с HTTP протоколом
			awh::client::http_t _http;
		private:
			// Максимальное количество запросов на одного пользователя в сутки
			std::uint16_t _maxRequests;
		private:
			// Объект сетевого ядра сервера
			awh::server::core_t _core;
			// Объект WEB-сервера
			awh::server::awh_t _awh;
		private:
			// Объект ограничения скорости сети доступа к серверу
			bandwidth_t _bandwidth;
		private:
			// Чёрный список для IP-адресов 
			std::unordered_set <std::string> _ipBlack;
			// Белый список для IP-адресов
			std::unordered_set <std::string> _ipWhite;
		private:
			// Чёрный список для MAC адресов
			std::unordered_set <std::string> _macBlack;
			// Белый список для MAC адресов
			std::unordered_set <std::string> _macWhite;
		private:
			// Список пользователей для авторизации клиента
			std::unordered_map <std::string, std::string> _users;
		private:
			// Каунтеры запросов клиентов
			std::unordered_map <std::string, std::pair <std::uint16_t, std::time_t>> _counts;
		private:
			// Кэш контента содержимого сайта
			std::unordered_map <std::string, std::pair <std::uint64_t, std::vector <char>>> _cache;
		private:
			// Объект фреймворка
			const awh::fmk_t * _fmk;
			// Объект работы с логами
			const awh::log_t * _log;
		private:
			/**
			 * crash Метод обработки вызова крашей в приложении
			 * @param sig номер сигнала операционной системы
			 */
			void crash(const std::int32_t sig) noexcept;
			/**
			 * error Метод генерации ошибки
			 * @param sid  идентификатор потока
			 * @param bid  идентификатор брокера
			 * @param code код ответа сервера
			 * @param mess сообщение ответа клиенту
			 */
			void error(const std::int32_t sid, const std::uint64_t bid, const std::uint16_t code, const std::string & mess) noexcept;
		private:
			/**
			 * password Метод извлечения пароля (для авторизации методом Digest)
			 * @param bid   идентификатор брокера (клиента)
			 * @param login логин пользователя
			 * @return      пароль пользователя хранящийся в базе данных
			 */
			std::string password(const std::uint64_t bid, const std::string & login) noexcept;
			/**
			 * auth Метод проверки авторизации пользователя (для авторизации методом Basic)
			 * @param bid      идентификатор брокера (клиента)
			 * @param login    логин пользователя (от клиента)
			 * @param password пароль пользователя (от клиента)
			 * @return         результат авторизации
			 */
			bool auth(const std::uint64_t bid, const std::string & login, const std::string & password) noexcept;
		private:
			/**
			 * accept Метод активации клиента на сервере
			 * @param ip   адрес интернет подключения
			 * @param mac  аппаратный адрес подключения
			 * @param port порт подключения
			 * @return     результат проверки
			 */
			bool accept(const std::string & ip, const std::string & mac, const std::uint32_t port) noexcept;
		private:
			/**
			 * active Метод вывода статуса работы сетевого ядра
			 * @param status флаг запуска сетевого ядра
			 */
			void active(const awh::core_t::status_t status) noexcept;
			/**
			 * active Метод идентификации активности на Web сервере
			 * @param bid  идентификатор брокера (клиента)
			 * @param mode режим события подключения
			 */
			void active(const std::uint64_t bid, const awh::server::web_t::mode_t mode) noexcept;
		private:
			/**
			 * handshake Метод получения удачного запроса
			 * @param sid   идентификатор потока
			 * @param bid   идентификатор брокера
			 * @param agent идентификатор агента клиента
			 */
			void handshake(const std::int32_t sid, const std::uint64_t bid, const awh::server::web_t::agent_t agent) noexcept;
			/**
			 * request Метод вывода входящего запроса
			 * @param sid     идентификатор входящего потока
			 * @param bid     идентификатор брокера (клиента)
			 * @param method  метод входящего запроса
			 * @param url     адрес входящего запроса
			 * @param headers заголовки запроса
			 */
			void headers(const std::int32_t sid, const std::uint64_t bid, const awh::web_t::method_t method, const awh::uri_t::url_t & url, const std::unordered_multimap <std::string, std::string> & headers) noexcept;
			/**
			 * complete Метод завершения получения запроса клиента
			 * @param sid     идентификатор потока
			 * @param bid     идентификатор брокера
			 * @param method  метод запроса
			 * @param url     url-адрес запроса
			 * @param entity  тело запроса
			 * @param headers заголовки запроса
			 */
			void complete(const std::int32_t sid, const std::uint64_t bid, const awh::web_t::method_t method, const awh::uri_t::url_t & url, const std::vector <char> & entity, const std::unordered_multimap <std::string, std::string> & headers) noexcept;
		public:
			/**
			 * config Метод установки конфигурационных параметров в формате JSON
			 * @param config объект конфигурационных параметров в формате JSON
			 */
			void config(const json & config) noexcept;
		public:
			/**
			 * stop Метод остановки работы сервера
			 */
			void stop() noexcept;
			/**
			 * start Метод запуска работы сервера
			 */
			void start() noexcept;
		public:
			/**
			 * Server конструктор
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Server(const awh::fmk_t * fmk, const awh::log_t * log) noexcept;
			/**
			 * ~Server деструктор
			 */
			~Server() noexcept;
	} server_t;
};

#endif // __ACU_SERVER__
