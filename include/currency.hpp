/**
 * @file: currency.hpp
 * @date: 2026-09-25
 *
 * @telegram: @forman
 * @author: Yuriy Lobarev
 * @phone: +7 (910) 983-95-90
 * @email: forman@anyks.com
 * @site: https://anyks.com
 *
 * @copyright: Copyright © 2026
 */

#ifndef __ANYKS_ACU_CURRENCY__
#define __ANYKS_ACU_CURRENCY__

/**
 * Разрешаем сборку под Windows
 */
#include <global.hpp>

/**
 * Подключаем зависимые заголовки
 */
#include <ctime>
#include <string>
#include <vector>
#include <unordered_map>

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
	 * @brief Класс модуля конвертации валют по курсам CoinGecko
	 *
	 * Модуль не выполняет сетевых запросов сам: метод need() сообщает, какой запрос к API нужен,
	 * а метод load() принимает его ответ. Так один и тот же модуль работает и в утилите
	 * (синхронные запросы), и в сервере (асинхронные запросы в базе событий сервера).
	 *
	 * Все курсы хранятся как количество единиц валюты за один биткоин, как их отдаёт CoinGecko
	 * в /exchange_rates. Монеты вне этого списка находятся через /search, а их цена берётся
	 * через /simple/price в долларах США и переводится в биткоины по курсу доллара.
	 *
	 * Курсы можно держать в общем файле кэша: его читают и пополняют все рабочие процессы сервера
	 * и все запуски утилиты, поэтому CoinGecko опрашивается раз в время жизни курсов на всю машину,
	 * а не на каждый процесс. Пока один процесс обновляет курсы (файл аренды занят), остальные
	 * отвечают по имеющимся курсам, а не идут в API следом
	 */
	typedef class ACU_SHARED_EXPORT Currency {
		private:
			/**
			 * @brief Структура курса валюты
			 *
			 */
			typedef struct Rate {
				bool fiat;    // Флаг фиатной валюты или драгоценного металла
				double value; // Количество единиц валюты за один биткоин
				/**
				 * @brief Конструктор
				 *
				 */
				Rate() noexcept : fiat(false), value(0.) {}
			} rate_t;
			/**
			 * @brief Структура монеты, найденной через поиск
			 *
			 */
			typedef struct Coin {
				string id;     // Идентификатор монеты в CoinGecko
				double usd;    // Цена монеты в долларах США
				time_t date;   // Время получения цены
				time_t used;   // Время последнего обращения к монете
				/**
				 * @brief Конструктор
				 *
				 */
				Coin() noexcept : id{""}, usd(0.), date(0), used(0) {}
			} coin_t;
		private:
			// Флаг использования платного тарифа API
			bool _pro;
			// Ключ доступа к API
			string _key;
			// Адрес сервера API, заданный вместо адреса CoinGecko
			string _url;
			// Текст последней ошибки
			string _error;
		private:
			// Адрес общего файла кэша курсов
			string _cache;
			// Флаг удержания аренды обновления курсов
			bool _lease;
			// Время изменения и размер файла кэша при последнем чтении
			std::pair <time_t, size_t> _synced;
		private:
			// Время жизни полученных курсов в секундах
			time_t _ttl;
			// Время получения списка курсов
			time_t _date;
		private:
			// Список курсов валют
			std::unordered_map <string, rate_t> _rates;
			// Список найденных монет
			std::unordered_map <string, coin_t> _coins;
			// Список неизвестных валют и время, когда о них узнали
			std::unordered_map <string, time_t> _unknown;
		private:
			// Объект работы с файловой системой
			fs_t _fs;
		private:
			// Объект фреймворка
			const fmk_t * _fmk;
			// Объект работы с логами
			const log_t * _log;
		private:
			/**
			 * @brief Метод приведения названия валюты к виду ключа
			 *
			 * @param name название валюты
			 * @return     ключ валюты или пустая строка, если название недопустимо
			 */
			string symbol(const string & name) const noexcept;
		private:
			/**
			 * @brief Метод получения курса валюты
			 *
			 * @param symbol ключ валюты
			 * @param fiat   флаг фиатной валюты
			 * @return       количество единиц валюты за один биткоин или 0
			 */
			double rate(const string & symbol, bool & fiat) const noexcept;
		private:
			/**
			 * @brief Метод чтения общего файла кэша курсов
			 *
			 */
			void read() noexcept;
			/**
			 * @brief Метод принудительного перечитывания общего файла кэша курсов
			 *
			 */
			void reread() noexcept;
			/**
			 * @brief Метод записи общего файла кэша курсов
			 *
			 */
			void write() noexcept;
		private:
			/**
			 * @brief Метод захвата аренды обновления курсов
			 *
			 * @return результат захвата: false, если курсы уже обновляет другой процесс
			 */
			bool lease() noexcept;
			/**
			 * @brief Метод освобождения аренды обновления курсов
			 *
			 */
			void release() noexcept;
		private:
			/**
			 * @brief Метод применения ответа API к спискам курсов
			 *
			 * @param path   путь выполненного запроса
			 * @param entity тело ответа
			 * @return       результат применения
			 */
			bool update(const string & path, const vector <char> & entity) noexcept;
		private:
			/**
			 * @brief Метод проверки ответа API на ошибку
			 *
			 * @param answer разобранный ответ API
			 * @return       результат проверки
			 */
			bool failed(const Document & answer) noexcept;
		public:
			/**
			 * @brief Метод получения адреса сервера API
			 *
			 * @return адрес сервера API
			 */
			string host() const noexcept;
			/**
			 * @brief Метод получения заголовков запроса к API
			 *
			 * @return заголовки запроса
			 */
			std::unordered_multimap <string, string> headers() const noexcept;
		public:
			/**
			 * @brief Метод получения текста последней ошибки
			 *
			 * @return текст ошибки
			 */
			const string & error() const noexcept;
		public:
			/**
			 * @brief Метод получения пути запроса к API, необходимого для конвертации
			 *
			 * @param from  валюта, из которой выполняется конвертация
			 * @param to    валюта, в которую выполняется конвертация
			 * @param stale флаг разрешения устаревших курсов (когда API недоступен)
			 * @return      путь запроса или пустая строка, если всё для конвертации есть
			 */
			string need(const string & from, const string & to, const bool stale = false) noexcept;
			/**
			 * @brief Метод загрузки ответа API
			 *
			 * @param path   путь выполненного запроса
			 * @param entity тело ответа
			 * @return       результат загрузки
			 */
			bool load(const string & path, const vector <char> & entity) noexcept;
		public:
			/**
			 * @brief Метод конвертации суммы из одной валюты в другую
			 *
			 * @param amount сумма для конвертации
			 * @param from   валюта, из которой выполняется конвертация
			 * @param to     валюта, в которую выполняется конвертация
			 * @param result результат конвертации
			 * @return       результат работы
			 */
			bool convert(const string & amount, const string & from, const string & to, string & result) noexcept;
		public:
			/**
			 * @brief Метод установки времени жизни курсов
			 *
			 * @param sec время жизни курсов в секундах
			 */
			void ttl(const time_t sec) noexcept;
			/**
			 * @brief Метод установки адреса общего файла кэша курсов
			 *
			 * @param filename адрес файла кэша, пустой адрес отключает общий кэш
			 */
			void cache(const string & filename) noexcept;
			/**
			 * @brief Метод получения адреса файла кэша по умолчанию (во временном каталоге пользователя)
			 *
			 * @return адрес файла кэша
			 */
			static string cache() noexcept;
			/**
			 * @brief Метод установки адреса сервера API вместо адреса CoinGecko (зеркало или прокси)
			 *
			 * @param url адрес сервера API, пустой адрес возвращает адрес CoinGecko
			 */
			void url(const string & url) noexcept;
			/**
			 * @brief Метод установки ключа доступа к API
			 *
			 * @param key ключ доступа (демо-ключ или ключ платного тарифа)
			 * @param pro флаг платного тарифа
			 */
			void key(const string & key, const bool pro = false) noexcept;
		public:
			/**
			 * @brief Конструктор
			 *
			 * @param fmk объект фреймворка
			 * @param log объект для работы с логами
			 */
			Currency(const fmk_t * fmk, const log_t * log) noexcept;
			/**
			 * @brief Деструктор
			 *
			 */
			~Currency() noexcept;
	} currency_t;
};

#endif // __ANYKS_ACU_CURRENCY__
