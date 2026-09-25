/**
 * @file: currency.cpp
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

/**
 * Подключаем заголовочный файл
 */
#include <currency.hpp>

/**
 * Подключаем стандартные модули
 */
#include <cmath>
#include <cstring>
#include <cstdio>
#include <limits>
#include <cerrno>
#include <cstdlib>
#include <algorithm>

/**
 * Подключаем системные модули работы с файлами
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * Для MS Windows
 */
#if _WIN32 || _WIN64
	#include <io.h>
	#include <process.h>
/**
 * Для Unix-подобных систем
 */
#else
	#include <unistd.h>
#endif

/**
 * Подключаем модули записи JSON
 */
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

/**
 * Подписываемся на пространство имён awh
 */
using namespace awh;
/**
 * Подписываемся на пространство имён rapidjson
 */
using namespace rapidjson;

/**
 * Путь запроса списка курсов
 */
static const string CURRENCY_RATES = "/api/v3/exchange_rates";
/**
 * Путь запроса поиска монеты
 */
static const string CURRENCY_SEARCH = "/api/v3/search?query=";
/**
 * Путь запроса цены монет
 */
static const string CURRENCY_PRICE = "/api/v3/simple/price?ids=";
/**
 * Параметры запроса цены монет
 */
static const string CURRENCY_PRICE_PARAMS = "&vs_currencies=usd&precision=full";
/**
 * Время, на которое запоминается неизвестная валюта (секунд)
 */
static constexpr time_t CURRENCY_UNKNOWN_TTL = 3600;
/**
 * Максимальное количество запоминаемых монет и неизвестных валют
 */
static constexpr size_t CURRENCY_CACHE_MAX = 1000;
/**
 * Время, после которого брошенная аренда обновления считается истёкшей (секунд).
 * После неудачного запроса аренда не снимается и держит остальные процессы этот срок,
 * чтобы при превышении лимита API они не повторяли запрос друг за другом
 */
static constexpr time_t CURRENCY_LEASE_TTL = 20;
/**
 * Монеты, к которым обращались за это время (секунд), обновляются одним общим запросом цен
 */
static constexpr time_t CURRENCY_USED_TTL = 86400;
/**
 * Максимальное количество монет в одном запросе цен
 */
static constexpr size_t CURRENCY_PRICE_MAX = 100;

/**
 * @brief Функция проверки, что файл создан текущим пользователем
 *
 * @param info сведения о файле
 * @return     результат проверки
 */
static bool currencyOwn([[maybe_unused]] const struct stat & info) noexcept {
	/**
	 * Для MS Windows владельца не проверяем: временный каталог у каждого пользователя свой
	 */
	#if _WIN32 || _WIN64
		// Выводим результат
		return true;
	/**
	 * Для Unix-подобных систем общий временный каталог доступен всем, чужому файлу не доверяем
	 */
	#else
		// Выводим результат
		return (info.st_uid == ::getuid());
	#endif
}

/**
 * @brief Метод приведения названия валюты к виду ключа
 *
 * @param name название валюты
 * @return     ключ валюты или пустая строка, если название недопустимо
 */
string anyks::Currency::symbol(const string & name) const noexcept {
	// Результат работы функции
	string result = name;
	// Удаляем пробелы по краям
	this->_fmk->transform(result, fmk_t::transform_t::TRIM);
	// Переводим название в нижний регистр
	this->_fmk->transform(result, fmk_t::transform_t::LOWER);
	// Если длина названия недопустима
	if(result.empty() || (result.size() > 64))
		// Выводим пустой результат
		return "";
	// Выполняем перебор всех символов названия
	for(auto & c : result){
		// Название уходит в адрес запроса, поэтому допускаются только буквы, цифры, точка, дефис и подчёркивание
		if(!(((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') || (c == '_')))
			// Выводим пустой результат
			return "";
	}
	// Выводим результат
	return result;
}
/**
 * @brief Метод получения курса валюты
 *
 * @param symbol ключ валюты
 * @param fiat   флаг фиатной валюты
 * @return       количество единиц валюты за один биткоин или 0
 */
double anyks::Currency::rate(const string & symbol, bool & fiat) const noexcept {
	// Сбрасываем флаг фиатной валюты
	fiat = false;
	// Выполняем поиск валюты в списке курсов
	auto i = this->_rates.find(symbol);
	// Если валюта найдена
	if(i != this->_rates.end()){
		// Запоминаем флаг фиатной валюты
		fiat = i->second.fiat;
		// Выводим курс валюты
		return i->second.value;
	}
	// Выполняем поиск монеты
	auto j = this->_coins.find(symbol);
	// Если монета найдена и её цена известна
	if((j != this->_coins.end()) && (j->second.usd > 0.)){
		// Выполняем поиск курса доллара
		auto k = this->_rates.find("usd");
		// Если курс доллара известен, переводим цену монеты в количество монет за один биткоин
		if(k != this->_rates.end())
			// Выводим курс монеты
			return (k->second.value / j->second.usd);
	}
	// Выводим результат
	return 0.;
}
/**
 * @brief Метод проверки ответа API на ошибку
 *
 * @param answer разобранный ответ API
 * @return       результат проверки
 */
bool anyks::Currency::failed(const Document & answer) noexcept {
	// Если ответ не является объектом
	if(!answer.IsObject()){
		// Запоминаем текст ошибки
		this->_error = "Exchange rate service returned an invalid response";
		// Сообщаем об ошибке
		return true;
	}
	// Если API вернул статус ошибки
	if(answer.HasMember("status") && answer["status"].IsObject() &&
	   answer["status"].HasMember("error_message") && answer["status"]["error_message"].IsString()){
		// Запоминаем текст ошибки
		this->_error = this->_fmk->format("Exchange rate service: %s", answer["status"]["error_message"].GetString());
		// Сообщаем об ошибке
		return true;
	}
	// Если API вернул текст ошибки
	if(answer.HasMember("error") && answer["error"].IsString()){
		// Запоминаем текст ошибки
		this->_error = this->_fmk->format("Exchange rate service: %s", answer["error"].GetString());
		// Сообщаем об ошибке
		return true;
	}
	// Сообщаем, что ошибки нет
	return false;
}
/**
 * @brief Метод получения адреса сервера API
 *
 * @return адрес сервера API
 */
string anyks::Currency::host() const noexcept {
	// Если адрес сервера API задан
	if(!this->_url.empty())
		// Выводим заданный адрес
		return this->_url;
	// Выводим адрес сервера API
	return (this->_pro ? "https://pro-api.coingecko.com" : "https://api.coingecko.com");
}
/**
 * @brief Метод получения заголовков запроса к API
 *
 * @return заголовки запроса
 */
std::unordered_multimap <string, string> anyks::Currency::headers() const noexcept {
	// Результат работы функции
	std::unordered_multimap <string, string> result = {{"Accept", "application/json"}};
	// Если ключ доступа установлен
	if(!this->_key.empty())
		// Добавляем ключ доступа
		result.emplace((this->_pro ? "x-cg-pro-api-key" : "x-cg-demo-api-key"), this->_key);
	// Выводим результат
	return result;
}
/**
 * @brief Метод получения текста последней ошибки
 *
 * @return текст ошибки
 */
const string & anyks::Currency::error() const noexcept {
	// Выводим текст ошибки
	return this->_error;
}
/**
 * @brief Метод получения пути запроса к API, необходимого для конвертации
 *
 * @param from  валюта, из которой выполняется конвертация
 * @param to    валюта, в которую выполняется конвертация
 * @param stale флаг разрешения устаревших курсов (когда API недоступен)
 * @return      путь запроса или пустая строка, если всё для конвертации есть
 */
string anyks::Currency::need(const string & from, const string & to, const bool stale) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Подгружаем курсы, полученные другими процессами
		this->read();
		// Получаем текущее время
		const time_t now = ::time(nullptr);
		// Если курсов нет совсем, их нужно получить в любом случае
		if(this->_rates.empty()){
			// Если запрос будет выполнен, захватываем аренду обновления
			if(!stale)
				// Захватываем аренду
				this->lease();
			// Запрашиваем список курсов
			return CURRENCY_RATES;
		}
		// Если курсы устарели и их не обновляет другой процесс
		if(!stale && ((now - this->_date) >= this->_ttl) && this->lease()){
			// Перечитываем файл кэша: другой процесс мог обновить курсы перед тем, как мы захватили аренду
			this->reread();
			// Если курсы по-прежнему устарели
			if((now - this->_date) >= this->_ttl)
				// Запрашиваем список курсов
				return CURRENCY_RATES;
			// Снимаем аренду, курсы уже свежие
			this->release();
		}
		// Список монет, цену которых нужно получить
		string ids = "";
		// Выполняем перебор обеих валют
		for(auto & name : {this->symbol(from), this->symbol(to)}){
			// Если название недопустимо или валюта есть в списке курсов
			if(name.empty() || (this->_rates.find(name) != this->_rates.end()))
				// Пропускаем валюту
				continue;
			// Выполняем поиск валюты среди неизвестных
			auto i = this->_unknown.find(name);
			// Если валюта недавно не нашлась
			if(i != this->_unknown.end()){
				// Если срок запоминания не истёк
				if((now - i->second) < CURRENCY_UNKNOWN_TTL)
					// Пропускаем валюту
					continue;
				// Удаляем валюту из неизвестных
				this->_unknown.erase(i);
			}
			// Выполняем поиск монеты
			auto j = this->_coins.find(name);
			// Если монета ещё не найдена
			if(j == this->_coins.end()){
				// Если запрос будет выполнен, захватываем аренду обновления
				if(!stale)
					// Захватываем аренду
					this->lease();
				// Запрашиваем поиск монеты
				return (CURRENCY_SEARCH + name);
			}
			// Запоминаем время обращения к монете
			j->second.used = now;
			// Если цена монеты неизвестна или устарела
			if((j->second.usd <= 0.) || (!stale && ((now - j->second.date) >= this->_ttl))){
				/**
				 * Если цена известна, но устарела, а обновление уже выполняет другой процесс,
				 * пользуемся имеющейся ценой. Неизвестную цену запрашиваем в любом случае
				 */
				if(!stale && (j->second.usd > 0.)){
					// Если аренду захватить не удалось
					if(!this->lease())
						// Пропускаем монету
						continue;
					// Перечитываем файл кэша: другой процесс мог обновить цену перед тем, как мы захватили аренду
					this->reread();
					// Если цена стала свежей
					if((now - j->second.date) < this->_ttl){
						// Снимаем аренду
						this->release();
						// Пропускаем монету
						continue;
					}
				}
				// Если монета ещё не добавлена в запрос
				if(("," + ids + ",").find("," + j->second.id + ",") == string::npos){
					// Если список монет не пустой
					if(!ids.empty())
						// Добавляем разделитель
						ids.append(1, ',');
					// Добавляем монету в запрос
					ids.append(j->second.id);
				}
			}
		}
		// Если нужно получить цены монет
		if(!ids.empty()){
			/**
			 * Обновляем заодно цены всех монет, к которым обращались за последние сутки:
			 * тогда цены монет обходятся одним запросом за время жизни курсов при любом их числе
			 */
			size_t count = (std::count(ids.begin(), ids.end(), ',') + 1);
			// Выполняем перебор всех монет
			for(auto & item : this->_coins){
				// Если запрос заполнен
				if(count >= CURRENCY_PRICE_MAX)
					// Выходим из цикла
					break;
				// Если к монете недавно обращались и её ещё нет в запросе
				if(!item.second.id.empty() && ((now - item.second.used) < CURRENCY_USED_TTL) &&
				  (("," + ids + ",").find("," + item.second.id + ",") == string::npos)){
					// Добавляем разделитель
					ids.append(1, ',');
					// Добавляем монету в запрос
					ids.append(item.second.id);
					// Увеличиваем количество монет в запросе
					count++;
				}
			}
			// Запрашиваем цены монет
			return (CURRENCY_PRICE + ids + CURRENCY_PRICE_PARAMS);
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выводим сообщение об ошибке
		this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
	}
	// Всё для конвертации есть
	return "";
}
/**
 * @brief Метод загрузки ответа API
 *
 * @param path   путь выполненного запроса
 * @param entity тело ответа
 * @return       результат загрузки
 */
bool anyks::Currency::load(const string & path, const vector <char> & entity) noexcept {
	// Применяем ответ API
	const bool result = this->update(path, entity);
	// Если ответ применён
	if(result){
		// Сохраняем курсы в общий файл кэша
		this->write();
		// Снимаем аренду обновления
		this->release();
	/**
	 * Если запрос неудачен, аренду не снимаем: она истечёт сама, а до тех пор остальные
	 * процессы отвечают по имеющимся курсам и не повторяют запрос к API
	 */
	} else this->_lease = false;
	// Выводим результат
	return result;
}
/**
 * @brief Метод применения ответа API к спискам курсов
 *
 * @param path   путь выполненного запроса
 * @param entity тело ответа
 * @return       результат применения
 */
bool anyks::Currency::update(const string & path, const vector <char> & entity) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Сбрасываем текст ошибки
		this->_error.clear();
		// Объект ответа API
		Document answer;
		// Если ответ не получен или не разобран
		if(entity.empty() || answer.Parse(entity.data(), entity.size()).HasParseError()){
			// Запоминаем текст ошибки
			this->_error = "Exchange rate service is unavailable";
			// Сообщаем об ошибке
			return false;
		}
		// Если API вернул ошибку
		if(this->failed(answer))
			// Сообщаем об ошибке
			return false;
		// Получаем текущее время
		const time_t now = ::time(nullptr);
		// Если получен список курсов
		if(path.compare(CURRENCY_RATES) == 0){
			// Новый список курсов
			std::unordered_map <string, rate_t> rates;
			// Если список курсов передан
			if(answer.HasMember("rates") && answer["rates"].IsObject()){
				// Выполняем перебор всех курсов
				for(auto & m : answer["rates"].GetObj()){
					// Если курс передан правильно
					if(m.value.IsObject() && m.value.HasMember("value") && m.value["value"].IsNumber() && (m.value["value"].GetDouble() > 0.)){
						// Курс валюты
						rate_t rate;
						// Запоминаем курс валюты
						rate.value = m.value["value"].GetDouble();
						// Фиатом считаются все валюты, кроме криптовалют (включая золото и серебро)
						rate.fiat = !(m.value.HasMember("type") && m.value["type"].IsString() && (::strcmp(m.value["type"].GetString(), "crypto") == 0));
						// Получаем ключ валюты
						const string & name = this->symbol(m.name.GetString());
						// Если ключ валюты получен
						if(!name.empty())
							// Добавляем курс в список
							rates.emplace(name, rate);
					}
				}
			}
			// Если курсы не получены
			if(rates.empty()){
				// Запоминаем текст ошибки
				this->_error = "Exchange rate service returned no rates";
				// Сообщаем об ошибке
				return false;
			}
			// Запоминаем список курсов
			this->_rates = ::move(rates);
			// Запоминаем время получения курсов
			this->_date = now;
		// Если получен результат поиска монеты
		} else if(path.compare(0, CURRENCY_SEARCH.size(), CURRENCY_SEARCH) == 0) {
			// Получаем название искомой валюты
			const string name = path.substr(CURRENCY_SEARCH.size());
			// Идентификатор найденной монеты
			string id = "";
			// Место монеты по капитализации
			uint64_t rank = numeric_limits <uint64_t>::max();
			// Если список монет передан
			if(answer.HasMember("coins") && answer["coins"].IsArray()){
				// Совпадения по идентификатору и по названию монеты
				string byId = "", byName = "";
				// Выполняем перебор всех найденных монет
				for(auto & coin : answer["coins"].GetArray()){
					// Если монета передана неправильно
					if(!coin.IsObject() || !coin.HasMember("id") || !coin["id"].IsString())
						// Пропускаем монету
						continue;
					// Получаем идентификатор монеты
					const string cid = coin["id"].GetString();
					// Идентификатор уходит в адрес запроса цены, поэтому монеты с недопустимыми идентификаторами пропускаются
					if(this->symbol(cid).compare(cid) != 0)
						// Пропускаем монету
						continue;
					// Если символ монеты совпадает с искомым
					if(coin.HasMember("symbol") && coin["symbol"].IsString() && (this->symbol(coin["symbol"].GetString()).compare(name) == 0)){
						// Среди монет с одинаковым символом выбирается самая крупная по капитализации
						const uint64_t place = ((coin.HasMember("market_cap_rank") && coin["market_cap_rank"].IsUint64()) ? coin["market_cap_rank"].GetUint64() : (numeric_limits <uint64_t>::max() - 1));
						// Если монета крупнее ранее найденной
						if(place < rank){
							// Запоминаем место монеты
							rank = place;
							// Запоминаем идентификатор монеты
							id = cid;
						}
					// Если идентификатор монеты совпадает с искомым
					} else if(byId.empty() && (cid.compare(name) == 0))
						// Запоминаем идентификатор монеты
						byId = cid;
					// Если название монеты совпадает с искомым
					else if(byName.empty() && coin.HasMember("name") && coin["name"].IsString()) {
						// Получаем название монеты
						string title = coin["name"].GetString();
						// Переводим название в нижний регистр
						this->_fmk->transform(title, fmk_t::transform_t::LOWER);
						// Если название совпадает
						if(title.compare(name) == 0)
							// Запоминаем идентификатор монеты
							byName = cid;
					}
				}
				// Если по символу монета не найдена, берём совпадение по идентификатору или по названию
				if(id.empty())
					// Запоминаем идентификатор монеты
					id = (!byId.empty() ? byId : byName);
			}
			// Если список запомненных монет переполнен
			if((this->_coins.size() >= CURRENCY_CACHE_MAX) || (this->_unknown.size() >= CURRENCY_CACHE_MAX)){
				// Очищаем список монет
				this->_coins.clear();
				// Очищаем список неизвестных валют
				this->_unknown.clear();
			}
			// Если монета не найдена
			if(id.empty())
				// Запоминаем валюту как неизвестную
				this->_unknown[name] = now;
			// Запоминаем монету
			else this->_coins[name].id = id;
		// Если получены цены монет
		} else if(path.compare(0, CURRENCY_PRICE.size(), CURRENCY_PRICE) == 0) {
			// Получаем список запрошенных монет
			string ids = path.substr(CURRENCY_PRICE.size());
			// Удаляем параметры запроса
			ids = ids.substr(0, ids.find('&'));
			// Выполняем перебор всех запомненных монет
			for(auto i = this->_coins.begin(); i != this->_coins.end();){
				// Если монета не запрашивалась
				if(("," + ids + ",").find("," + i->second.id + ",") == string::npos){
					// Переходим к следующей монете
					++i;
					// Продолжаем перебор
					continue;
				}
				// Если цена монеты получена
				if(answer.HasMember(i->second.id.c_str()) && answer[i->second.id.c_str()].IsObject() &&
				   answer[i->second.id.c_str()].HasMember("usd") && answer[i->second.id.c_str()]["usd"].IsNumber() &&
				  (answer[i->second.id.c_str()]["usd"].GetDouble() > 0.)){
					// Запоминаем цену монеты
					i->second.usd = answer[i->second.id.c_str()]["usd"].GetDouble();
					// Запоминаем время получения цены
					i->second.date = now;
					// Переходим к следующей монете
					++i;
				// Если цены у монеты нет, считаем её неизвестной
				} else {
					// Запоминаем валюту как неизвестную
					this->_unknown[i->first] = now;
					// Удаляем монету
					i = this->_coins.erase(i);
				}
			}
		// Если путь запроса неизвестен
		} else {
			// Запоминаем текст ошибки
			this->_error = "Unknown exchange rate request";
			// Сообщаем об ошибке
			return false;
		}
		// Сообщаем, что ответ загружен
		return true;
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Запоминаем текст ошибки
		this->_error = error.what();
	}
	// Сообщаем об ошибке
	return false;
}
/**
 * @brief Метод конвертации суммы из одной валюты в другую
 *
 * @param amount сумма для конвертации
 * @param from   валюта, из которой выполняется конвертация
 * @param to     валюта, в которую выполняется конвертация
 * @param result результат конвертации
 * @return       результат работы
 */
bool anyks::Currency::convert(const string & amount, const string & from, const string & to, string & result) noexcept {
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Сбрасываем результат
		result.clear();
		// Сбрасываем текст ошибки
		this->_error.clear();
		// Получаем сумму для конвертации
		string text = amount;
		// Удаляем пробелы по краям
		this->_fmk->transform(text, fmk_t::transform_t::TRIM);
		// Если сумма не является числом
		if(text.empty() || !(this->_fmk->is(text, fmk_t::check_t::NUMBER) || this->_fmk->is(text, fmk_t::check_t::DECIMAL))){
			// Запоминаем текст ошибки
			this->_error = "Amount must be a number";
			// Сообщаем об ошибке
			return false;
		}
		// Флаги фиатных валют
		bool fiatFrom = false, fiatTo = false;
		// Получаем курсы валют
		const double rateFrom = this->rate(this->symbol(from), fiatFrom), rateTo = this->rate(this->symbol(to), fiatTo);
		// Если курс исходной валюты неизвестен
		if(rateFrom <= 0.){
			// Запоминаем текст ошибки
			this->_error = this->_fmk->format("Unknown currency: %s", from.c_str());
			// Сообщаем об ошибке
			return false;
		}
		// Если курс итоговой валюты неизвестен
		if(rateTo <= 0.){
			// Запоминаем текст ошибки
			this->_error = this->_fmk->format("Unknown currency: %s", to.c_str());
			// Сообщаем об ошибке
			return false;
		}
		// Выполняем конвертацию суммы
		const double value = (::stod(text) * rateTo / rateFrom);
		// Если результат не является конечным числом
		if(!std::isfinite(value)){
			// Запоминаем текст ошибки
			this->_error = "Amount is out of range";
			// Сообщаем об ошибке
			return false;
		}
		// Фиатные суммы выводятся в копейках, криптовалюты с точностью до сатоши
		int32_t decimals = (fiatTo ? 2 : 8);
		// Если сумма меньше единицы, выводим не менее четырёх значащих цифр
		if((value != 0.) && (::fabs(value) < 1.))
			// Увеличиваем количество знаков после запятой
			decimals = std::min(20, std::max(decimals, static_cast <int32_t> (-::floor(::log10(::fabs(value)))) + 3));
		// Буфер для формирования результата
		char buffer[512];
		// Формируем результат
		::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
		// Запоминаем результат
		result = buffer;
		// Если сумма фиатная и выводится в копейках, оставляем результат как есть
		if(!fiatTo || (decimals > 2)){
			// Удаляем незначащие нули
			while(!result.empty() && (result.back() == '0'))
				// Удаляем последний ноль
				result.pop_back();
			// Если остался разделитель дробной части
			if(!result.empty() && (result.back() == '.'))
				// Удаляем разделитель
				result.pop_back();
		}
		// Если после округления осталась отрицательная нулевая сумма
		if(result.compare("-0") == 0)
			// Исправляем результат
			result = "0";
		// Сообщаем, что конвертация выполнена
		return true;
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception &) {
		// Запоминаем текст ошибки
		this->_error = "Amount is out of range";
	}
	// Сообщаем об ошибке
	return false;
}
/**
 * @brief Метод чтения общего файла кэша курсов
 *
 */
void anyks::Currency::read() noexcept {
	// Если общий кэш не используется
	if(this->_cache.empty())
		// Выходим из функции
		return;
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Сведения о файле кэша
		struct stat info;
		// Если файла нет или он создан другим пользователем
		if((::stat(this->_cache.c_str(), &info) != 0) || !currencyOwn(info))
			// Выходим из функции
			return;
		// Получаем время изменения и размер файла
		const std::pair <time_t, size_t> stamp = {static_cast <time_t> (info.st_mtime), static_cast <size_t> (info.st_size)};
		// Если файл не изменился с последнего чтения
		if(stamp == this->_synced)
			// Выходим из функции
			return;
		// Запоминаем состояние файла
		this->_synced = stamp;
		// Выполняем чтение файла
		const auto & buffer = this->_fs.read(this->_cache);
		// Объект кэша
		Document cache;
		// Если файл пустой или не разобран
		if(buffer.empty() || cache.Parse(buffer.data(), buffer.size()).HasParseError() || !cache.IsObject())
			// Выходим из функции
			return;
		// Если в файле курсы новее имеющихся
		if(cache.HasMember("date") && cache["date"].IsInt64() && (static_cast <time_t> (cache["date"].GetInt64()) > this->_date) &&
		   cache.HasMember("rates") && cache["rates"].IsObject()){
			// Новый список курсов
			std::unordered_map <string, rate_t> rates;
			// Выполняем перебор всех курсов
			for(auto & m : cache["rates"].GetObj()){
				// Получаем ключ валюты
				const string & name = this->symbol(m.name.GetString());
				// Если курс записан правильно
				if(!name.empty() && m.value.IsObject() && m.value.HasMember("value") && m.value["value"].IsNumber() &&
				  (m.value["value"].GetDouble() > 0.) && m.value.HasMember("fiat") && m.value["fiat"].IsBool()){
					// Курс валюты
					rate_t rate;
					// Запоминаем курс
					rate.value = m.value["value"].GetDouble();
					// Запоминаем флаг фиатной валюты
					rate.fiat = m.value["fiat"].GetBool();
					// Добавляем курс в список
					rates.emplace(name, rate);
				}
			}
			// Если курсы получены
			if(!rates.empty()){
				// Запоминаем список курсов
				this->_rates = ::move(rates);
				// Запоминаем время получения курсов
				this->_date = static_cast <time_t> (cache["date"].GetInt64());
			}
		}
		// Если в файле есть найденные монеты
		if(cache.HasMember("coins") && cache["coins"].IsObject()){
			// Выполняем перебор всех монет
			for(auto & m : cache["coins"].GetObj()){
				// Получаем ключ валюты
				const string & name = this->symbol(m.name.GetString());
				// Если монета записана неправильно
				if(name.empty() || !m.value.IsObject() || !m.value.HasMember("id") || !m.value["id"].IsString() ||
				   !m.value.HasMember("usd") || !m.value["usd"].IsNumber() || !m.value.HasMember("date") || !m.value["date"].IsInt64())
					// Пропускаем монету
					continue;
				// Получаем идентификатор монеты
				const string id = m.value["id"].GetString();
				// Если идентификатор недопустим
				if(this->symbol(id).compare(id) != 0)
					// Пропускаем монету
					continue;
				// Получаем монету из списка
				coin_t & coin = this->_coins[name];
				// Если монета новая или в файле цена свежее
				if(coin.id.empty() || (static_cast <time_t> (m.value["date"].GetInt64()) > coin.date)){
					// Запоминаем идентификатор монеты
					coin.id = id;
					// Запоминаем цену монеты
					coin.usd = m.value["usd"].GetDouble();
					// Запоминаем время получения цены
					coin.date = static_cast <time_t> (m.value["date"].GetInt64());
				}
				// Если время обращения записано, запоминаем более позднее
				if(m.value.HasMember("used") && m.value["used"].IsInt64())
					// Запоминаем время обращения к монете
					coin.used = std::max(coin.used, static_cast <time_t> (m.value["used"].GetInt64()));
			}
		}
		// Если в файле есть неизвестные валюты
		if(cache.HasMember("unknown") && cache["unknown"].IsObject()){
			// Выполняем перебор всех неизвестных валют
			for(auto & m : cache["unknown"].GetObj()){
				// Получаем ключ валюты
				const string & name = this->symbol(m.name.GetString());
				// Если валюта записана правильно и её нет среди найденных монет
				if(!name.empty() && m.value.IsInt64() && (this->_coins.find(name) == this->_coins.end())){
					// Получаем время из списка
					time_t & date = this->_unknown[name];
					// Запоминаем более позднее время
					date = std::max(date, static_cast <time_t> (m.value.GetInt64()));
				}
			}
		}
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выводим сообщение об ошибке
		this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
	}
}
/**
 * @brief Метод принудительного перечитывания общего файла кэша курсов
 *
 */
void anyks::Currency::reread() noexcept {
	// Сбрасываем состояние файла, чтобы он был прочитан даже без видимых изменений
	this->_synced = {0, 0};
	// Выполняем чтение файла кэша
	this->read();
}
/**
 * @brief Метод записи общего файла кэша курсов
 *
 */
void anyks::Currency::write() noexcept {
	// Если общий кэш не используется
	if(this->_cache.empty())
		// Выходим из функции
		return;
	/**
	 * Выполняем отлов ошибок
	 */
	try {
		// Подгружаем записи других процессов, чтобы не затереть их
		this->read();
		// Объект кэша
		Document cache(kObjectType);
		// Получаем распределитель памяти
		auto & alloc = cache.GetAllocator();
		// Список курсов, монет и неизвестных валют
		Value rates(kObjectType), coins(kObjectType), unknown(kObjectType);
		// Выполняем перебор всех курсов
		for(auto & item : this->_rates){
			// Курс валюты
			Value rate(kObjectType);
			// Добавляем курс
			rate.AddMember("value", item.second.value, alloc);
			// Добавляем флаг фиатной валюты
			rate.AddMember("fiat", item.second.fiat, alloc);
			// Добавляем валюту в список
			rates.AddMember(Value(item.first.c_str(), alloc).Move(), rate, alloc);
		}
		// Выполняем перебор всех монет
		for(auto & item : this->_coins){
			// Монета
			Value coin(kObjectType);
			// Добавляем идентификатор монеты
			coin.AddMember("id", Value(item.second.id.c_str(), alloc).Move(), alloc);
			// Добавляем цену монеты
			coin.AddMember("usd", item.second.usd, alloc);
			// Добавляем время получения цены
			coin.AddMember("date", static_cast <int64_t> (item.second.date), alloc);
			// Добавляем время обращения к монете
			coin.AddMember("used", static_cast <int64_t> (item.second.used), alloc);
			// Добавляем монету в список
			coins.AddMember(Value(item.first.c_str(), alloc).Move(), coin, alloc);
		}
		// Выполняем перебор всех неизвестных валют
		for(auto & item : this->_unknown)
			// Добавляем валюту в список
			unknown.AddMember(Value(item.first.c_str(), alloc).Move(), static_cast <int64_t> (item.second), alloc);
		// Добавляем время получения курсов
		cache.AddMember("date", static_cast <int64_t> (this->_date), alloc);
		// Добавляем список курсов
		cache.AddMember("rates", rates, alloc);
		// Добавляем список монет
		cache.AddMember("coins", coins, alloc);
		// Добавляем список неизвестных валют
		cache.AddMember("unknown", unknown, alloc);
		// Создаём буфер результата
		StringBuffer buffer;
		// Создаём объект писателя
		Writer <StringBuffer> writer(buffer);
		// Передаём данные писателю
		cache.Accept(writer);
		// Формируем адрес временного файла: он уникален для процесса, а подмена целевого файла атомарна
		const string temporary = this->_fmk->format("%s.%d.tmp", this->_cache.c_str(), static_cast <int32_t> (::getpid()));
		// Выполняем запись временного файла
		this->_fs.write(temporary, buffer.GetString(), buffer.GetSize());
		// Если подменить файл кэша не удалось
		if(!this->_fs.replaceAddress(temporary, this->_cache))
			// Удаляем временный файл
			std::remove(temporary.c_str());
		// Сведения о файле кэша
		struct stat info;
		// Если сведения о файле получены
		if(::stat(this->_cache.c_str(), &info) == 0)
			// Запоминаем состояние файла, чтобы не перечитывать свою же запись
			this->_synced = {static_cast <time_t> (info.st_mtime), static_cast <size_t> (info.st_size)};
	/**
	 * Если возникает ошибка
	 */
	} catch(const exception & error) {
		// Выводим сообщение об ошибке
		this->_log->print("%s", log_t::flag_t::CRITICAL, error.what());
	}
}
/**
 * @brief Метод захвата аренды обновления курсов
 *
 * @return результат захвата: false, если курсы уже обновляет другой процесс
 */
bool anyks::Currency::lease() noexcept {
	// Если общий кэш не используется или аренда уже захвачена
	if(this->_cache.empty() || this->_lease)
		// Сообщаем, что аренда есть
		return true;
	// Получаем адрес файла аренды
	const string filename = (this->_cache + ".lock");
	// Выполняем две попытки: вторая после удаления истёкшей аренды
	for(uint8_t i = 0; i < 2; i++){
		// Выполняем создание файла аренды, только если его ещё нет
		const int32_t fd = ::open(filename.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
		// Если файл аренды создан
		if(fd >= 0){
			// Закрываем файл аренды
			::close(fd);
			// Запоминаем, что аренда захвачена
			this->_lease = true;
			// Сообщаем, что аренда захвачена
			return true;
		}
		// Если файл аренды не создан по другой причине (нет прав), работаем как единственный процесс
		if(errno != EEXIST)
			// Сообщаем, что аренда есть
			return true;
		// Сведения о файле аренды
		struct stat info;
		// Если сведений нет, аренда только что снята, пробуем ещё раз
		if(::stat(filename.c_str(), &info) != 0)
			// Повторяем попытку
			continue;
		// Если файл аренды создан другим пользователем, не даём ему останавливать обновление курсов
		if(!currencyOwn(info))
			// Сообщаем, что аренда есть
			return true;
		// Если аренда ещё не истекла, курсы обновляет другой процесс
		if((::time(nullptr) - static_cast <time_t> (info.st_mtime)) < CURRENCY_LEASE_TTL)
			// Сообщаем, что аренда занята
			return false;
		// Удаляем истёкшую аренду
		std::remove(filename.c_str());
	}
	// Сообщаем, что аренда занята
	return false;
}
/**
 * @brief Метод освобождения аренды обновления курсов
 *
 */
void anyks::Currency::release() noexcept {
	// Если аренда захвачена
	if(this->_lease){
		// Сбрасываем флаг аренды
		this->_lease = false;
		// Удаляем файл аренды
		std::remove((this->_cache + ".lock").c_str());
	}
}
/**
 * @brief Метод установки адреса общего файла кэша курсов
 *
 * @param filename адрес файла кэша, пустой адрес отключает общий кэш
 */
void anyks::Currency::cache(const string & filename) noexcept {
	// Снимаем аренду прежнего файла
	this->release();
	// Устанавливаем адрес файла кэша
	this->_cache = filename;
	// Удаляем пробелы по краям адреса
	this->_fmk->transform(this->_cache, fmk_t::transform_t::TRIM);
	// Сбрасываем состояние файла
	this->_synced = {0, 0};
}
/**
 * @brief Метод получения адреса файла кэша по умолчанию (во временном каталоге пользователя)
 *
 * @return адрес файла кэша
 */
string anyks::Currency::cache() noexcept {
	/**
	 * Для MS Windows
	 */
	#if _WIN32 || _WIN64
		// Получаем временный каталог пользователя
		const char * path = ::getenv("TEMP");
		// Выводим адрес файла кэша
		return ((path != nullptr) && (*path != '\0') ? string(path) : string(".")) + "\\acu-currency.json";
	/**
	 * Для Unix-подобных систем
	 */
	#else
		// Получаем временный каталог пользователя
		const char * path = ::getenv("TMPDIR");
		// Адрес временного каталога
		string dir = ((path != nullptr) && (*path != '\0') ? path : "/tmp");
		// Удаляем завершающие разделители пути
		while((dir.size() > 1) && (dir.back() == '/'))
			// Удаляем разделитель
			dir.pop_back();
		// Имя файла содержит идентификатор пользователя: в общем каталоге у каждого пользователя свой кэш
		return (dir + "/acu-currency-" + std::to_string(::getuid()) + ".json");
	#endif
}
/**
 * @brief Метод установки времени жизни курсов
 *
 * @param sec время жизни курсов в секундах
 */
void anyks::Currency::ttl(const time_t sec) noexcept {
	// Устанавливаем время жизни курсов
	this->_ttl = (sec > 0 ? sec : 600);
}
/**
 * @brief Метод установки адреса сервера API вместо адреса CoinGecko (зеркало или прокси)
 *
 * @param url адрес сервера API, пустой адрес возвращает адрес CoinGecko
 */
void anyks::Currency::url(const string & url) noexcept {
	// Устанавливаем адрес сервера API
	this->_url = url;
	// Удаляем пробелы по краям адреса
	this->_fmk->transform(this->_url, fmk_t::transform_t::TRIM);
	// Удаляем завершающие разделители пути
	while(!this->_url.empty() && (this->_url.back() == '/'))
		// Удаляем разделитель
		this->_url.pop_back();
}
/**
 * @brief Метод установки ключа доступа к API
 *
 * @param key ключ доступа (демо-ключ или ключ платного тарифа)
 * @param pro флаг платного тарифа
 */
void anyks::Currency::key(const string & key, const bool pro) noexcept {
	// Устанавливаем ключ доступа
	this->_key = key;
	// Устанавливаем флаг платного тарифа
	this->_pro = (pro && !key.empty());
	// Удаляем пробелы по краям ключа
	this->_fmk->transform(this->_key, fmk_t::transform_t::TRIM);
}
/**
 * @brief Конструктор
 *
 * @param fmk объект фреймворка
 * @param log объект для работы с логами
 */
anyks::Currency::Currency(const fmk_t * fmk, const log_t * log) noexcept :
 _pro(false), _key{""}, _url{""}, _error{""}, _cache{""}, _lease(false), _synced{0, 0},
 _ttl(600), _date(0), _fs(fmk, log), _fmk(fmk), _log(log) {}
/**
 * @brief Деструктор
 *
 */
anyks::Currency::~Currency() noexcept {
	// Снимаем аренду обновления курсов
	this->release();
}
