(function($){
	// Активируем событие загрузки
	$(document).ready(function(){
		// Блок подсветки синтаксиса шаблонов GROK
		let grokEditor = null;
		// Блок подсветки синтаксиса формата в который производится хеширвоание
		let toEditorHashing = null,
		    toEditorNotation = null,
			toEditorBytes = null,
			toEditorSeconds = null,
			toEditorDate = null;
		// Блок подсветки синтаксиса формата из которого производится хеширвоание
		let fromEditorHashing = null,
		    fromEditorNotation = null,
			fromEditorBytes = null,
			fromEditorSeconds = null,
			fromEditorDate = null;
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
		let fromEditorContainers = CodeMirror(document.getElementById('code-editor-from-containers'), {
			tabSize:     4,
			mode:        'xml',
			theme:       'eclipse',
			lineNumbers: true
		});
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
		const toEditorContainers = CodeMirror(document.getElementById('code-editor-to-containers'), {
			tabSize:     4,
			theme:       'eclipse',
			mode:        'javascript',
			readOnly:    true,
			lineNumbers: true
		});
		/**
		 * alert Функция вывода всплывающего сообщения
		 * @param {String} caption заголовок сообщения
		 * @param {String} data    текст сообщения
		 */
		const alert = (caption, data) => {
			// Если размер окнша выше 767 пикселей
			if($(window).width() >= 768){
				// Шаблон всплывающего сообщения
				const template = `<div class="modal fade" id="alert" tabindex="-1" aria-labelledby="alertCaption" aria-hidden="true"><div class="modal-dialog"><div class="modal-content"><div class="modal-header"><h1 class="modal-title fs-5" id="alertCaption"></h1><button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button></div><div class="modal-body"></div><div class="modal-footer"><button type="button" class="btn btn-primary" data-bs-dismiss="modal" aria-label="Close">Ok</button></div></div></div></div>`;
				// Выполняем добавление шаблона всплывающего сообщения
				$("body").append(template);
				// Устанавливаем событие на скрытие всплывающего сообщения
				$("#alert")
				.unbind("hidden.bs.modal")
				.bind("hidden.bs.modal", function() {
					// Выполняем удаление всплывающего сообщения
					$(this).remove();
					// Выходим из функции
					return false;
				});
				// Выполняем установку текста заголовка
				$("#alertCaption").text(caption);
				// Выполняем установка текста сообщения
				$(".modal-body", "#alert").html(data);
				// Отображаем всплывающее сообщение
				$("#alert").modal('show');
			// Выводим нативное всплывающее сообщение
			} else window.alert(data);
		};
		// Активируем событие вкладок
		$(".card-header-tabs a.nav-link", "#formatter")
		.unbind('click')
		.bind('click', function(){
			// Если вкладка не является активной
			if(!$(this).hasClass("active")){
				// Выполняем получение активной вкладки
				const active = $("a.active", $(this).parent().parent());
				// Выполняем удаление активности ссылки
				active.removeClass("active");
				// Скрываем блок с данными активной вкладки
				$(active.attr("href")).addClass("hidden");
				// Выполняем установку активной вкладки
				$(this).addClass("active");
				// Отображаем нужный нам блок данных
				$($(this).attr("href")).removeClass("hidden");
				// Если блоки подсветки синтаксиса для хэширования не созданы
				if((fromEditorHashing === null) || (toEditorHashing === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorHashing = CodeMirror(document.getElementById('code-editor-from-hashing'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						lineNumbers:  false,
						lineWrapping: false
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorHashing = CodeMirror(document.getElementById('code-editor-to-hashing'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						readOnly:     true,
						lineNumbers:  false,
						lineWrapping: true
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять хеширования
					$("> .CodeMirror", "#code-editor-from-hashing")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorHashing.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorHashing.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-hashing")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner-hashing").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorHashing.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = "", to = "";
									// Определяем формат в который производится конвертация
									switch($("a.active", "#to-hashing").attr("href")){
										// Если формат в который производится конвертация установлен как TEXT
										case "#TEXT": to = "text"; break;
										// Если формат в который производится конвертация установлен как BASE64
										case "#BASE64": to = "base64"; break;
										// Если формат в который производится конвертация установлен как MD5
										case "#MD5": to = "md5"; break;
										// Если формат в который производится конвертация установлен как SHA1
										case "#SHA1": to = "sha1"; break;
										// Если формат в который производится конвертация установлен как SHA224
										case "#SHA224": to = "sha224"; break;
										// Если формат в который производится конвертация установлен как SHA256
										case "#SHA256": to = "sha256"; break;
										// Если формат в который производится конвертация установлен как SHA384
										case "#SHA384": to = "sha384"; break;
										// Если формат в который производится конвертация установлен как SHA512
										case "#SHA512": to = "sha512"; break;
									}
									// Определяем формат с которого производится конвертация
									switch($("a.active", "#from-hashing").attr("href")){
										// Если формат в который производится конвертация установлен как TEXT
										case "#TEXT": from = "text"; break;
										// Если формат в который производится конвертация установлен как BASE64
										case "#BASE64": from = "base64"; break;
									}
									// Если форматы для конвертации указанны
									if((to.length > 0) && (from.length > 0)){
										// Получаем данные текстового поля HMAC
										const hmac = (((to !== "text") && (to !== "base64")) ? $("#hmac-key").val() : undefined);
										// Формируем объект тело запроса
										const body = {to, from, text, hmac};
										// Выполняем запрос на сервер
										const response = await fetch("/exec", {
											method: "POST",
											body: JSON.stringify(body),
											headers: {"Content-type": "application/json; charset=UTF-8"}
										});
										// Извлекаем полученный результат
										const answer = await response.json();
										// Если в ответе есть поле результата
										if((answer.result !== null) && (answer.result !== undefined))
											// Выполняем установку результата
											toEditorHashing.setValue(answer.result);
										// Выводим сообщение об ошибке
										else {
											// Если получен текст ошибки
											if((answer.error !== null) && (answer.error !== undefined))
												// Отображаем всплывающее сообщение
												alert("Ошибка запроса", answer.error);
											// Если текст ошибки не получен
											else alert("Ответ не получен", "Ответ от сервера не содержит результата");
										}
									// Если текст для конвертации не заполнен
									} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
								// Если текст для хеширования не заполнен
								} else alert("Ошибка хеширования", "Текст для хеширования не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner-hashing").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно хешировать
					$("> li > a", "#from-hashing")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorHashing.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-hashing").parent()).addClass("hidden");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран BASE64
								case "BASE64": {
									// Выполняем включение переноса строк
									fromEditorHashing.setOption("lineWrapping", true);
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#TEXT]", "#to-hashing").click();
									// Блокируем вкладку конвертера
									$("> li > a[href=#BASE64]", "#to-hashing").addClass("disabled");
								} break;
								// Если активирован любой другой конвертер
								default: {
									// Выполняем отключение переноса строк
									fromEditorHashing.setOption("lineWrapping", false);
									// Разблокируем вкладку конвертера
									$("> li > a[href=#BASE64]", "#to-hashing").removeClass("disabled");
								}
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно хешировать
					$("> li > a", "#to-hashing")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorHashing.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-hashing").click();
						// Удаляем текст в текстовом окне
						else toEditorHashing.setValue("");
						// Определяем тип конвертера
						switch($(this).text()){
							// Если ковертер выбран MD5
							case "MD5":
							// Если ковертер выбран SHA1
							case "SHA1":
							// Если ковертер выбран SHA224
							case "SHA224":
							// Если ковертер выбран SHA256
							case "SHA256":
							// Если ковертер выбран SHA384
							case "SHA384":
							// Если ковертер выбран SHA512
							case "SHA512": {
								// Отображаем текстовое поле HMAC ключа
								$("#hmac-key").parent().removeClass("hidden");
								// Выполняем включение переноса строк
								toEditorHashing.setOption("lineWrapping", true);
							} break;
							// Если ковертер выбран BASE64
							case "BASE64": {
								// Скрываем текстовое поле HMAC ключа
								$("#hmac-key").parent().addClass("hidden");
								// Выполняем включение переноса строк
								toEditorHashing.setOption("lineWrapping", true);
							} break;
							// Если ковертер выбран любой другой
							default: {
								// Скрываем текстовое поле HMAC ключа
								$("#hmac-key").parent().addClass("hidden");
								// Выполняем отключение переноса строк
								toEditorHashing.setOption("lineWrapping", false);
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на текстовое поле ввода HMAC ключа
					$("#hmac-key")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем объект кнопки очистки текстового поля
						const button = $(".btn-close", $(this).parent());
						// Если текст введён в текстовое поле
						if($(this).val().length > 0)
							// Выполняем отображение кнопки очистки текстового поля
							button.removeClass("hidden");
						// Скрываем кнопку очистки текстового поля
						else button.addClass("hidden");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на клик по кнопке
					.parent()
					.find(".btn-close")
					.unbind('click')
					.bind('click', function(){
						// Скрываем кнопку очистки текстового поля
						$(this).addClass("hidden");
						// Выполняем очистку текстового поля
						$("#hmac-key").val("");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					});
				}
				// Если блоки подсветки синтаксиса для систем счисления не созданы
				if((fromEditorNotation === null) || (toEditorNotation === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorNotation = CodeMirror(document.getElementById('code-editor-from-notation'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						lineNumbers:  false,
						lineWrapping: false
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorNotation = CodeMirror(document.getElementById('code-editor-to-notation'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						readOnly:     true,
						lineNumbers:  false,
						lineWrapping: true
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
					$("> .CodeMirror", "#code-editor-from-notation")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorNotation.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorNotation.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-notation")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner-notation").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorNotation.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = -1, to = -1;
									// Определяем систему счисления в которую производится конвертация
									switch($("a.active", "#to-notation").attr("href")){
										// Если система счисления определена как текст
										case "#notation-0": to = 0; break;
										// Если система счисления определена как Римская
										case "#notation-1": to = 1; break;
										// Если система счисления определена как бинарная
										case "#notation-2": to = 2; break;
										// Если система счисления определена как восьмеричная
										case "#notation-8": to = 8; break;
										// Если система счисления определена как десятичная
										case "#notation-10": to = 10; break;
										// Если система счисления определена как шестнадцатеричная
										case "#notation-16": to = 16; break;
										// Если система счисления определена как 17-я
										case "#notation-17": to = 17; break;
									}
									// Если мы не получили систему счисления в которую необходимо сконвертировать число
									if(to === -1)
										// Получаем систему счисления из списка
										to = parseInt($("#dropdown-to-notation").text(), 10);
									// Определяем систему счисления из которой производится конвертация
									switch($("a.active", "#from-notation").attr("href")){
										// Если система счисления определена как текст
										case "#notation-0": from = 0; break;
										// Если система счисления определена как Римская
										case "#notation-1": from = 1; break;
										// Если система счисления определена как бинарная
										case "#notation-2": from = 2; break;
										// Если система счисления определена как восьмеричная
										case "#notation-8": form = 8; break;
										// Если система счисления определена как десятичная
										case "#notation-10": from = 10; break;
										// Если система счисления определена как шестнадцатеричная
										case "#notation-16": from = 16; break;
									}
									// Если мы не получили систему счисления из которой необходимо сконвертировать число
									if(from === -1)
										// Получаем систему счисления из списка
										from = parseInt($("#dropdown-from-notation").text(), 10);
									// Если системы счисления для конвертации получены
									if((to > -1) && (from > -1)){
										// Формируем объект тело запроса
										const body = {to, from, text, notation: true};
										// Выполняем запрос на сервер
										const response = await fetch("/exec", {
											method: "POST",
											body: JSON.stringify(body),
											headers: {"Content-type": "application/json; charset=UTF-8"}
										});
										// Извлекаем полученный результат
										const answer = await response.json();
										// Если в ответе есть поле результата
										if((answer.result !== null) && (answer.result !== undefined))
											// Выполняем установку результата
											toEditorNotation.setValue(answer.result);
										// Выводим сообщение об ошибке
										else {
											// Если получен текст ошибки
											if((answer.error !== null) && (answer.error !== undefined))
												// Отображаем всплывающее сообщение
												alert("Ошибка запроса", answer.error);
											// Если текст ошибки не получен
											else alert("Ответ не получен", "Ответ от сервера не содержит результата");
										}
									// Если текст для конвертации не заполнен
									} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
								// Если текст для конвертации не заполнен
								} else alert("Ошибка конвертации", "Текст для конвертации не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner-notation").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выпадающего списка системы счисления для конвертации
					$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item", "#from-notation")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Получаем значение текста ссылки
							const action = $(this).text();
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Меняем текст выбранной системой счисления
							$("> a", $(this).parents(".dropdown")).text(action);
							// Определяем тип конвертера
							switch(action){
								// Если для конвертации выбрана двоичная система счисления
								case "2": $("> li > a[href=#notation-2]", "#from-notation").click(); break;
								// Если для конвертации выбрана восьмеричная система счисления
								case "8": $("> li > a[href=#notation-8]", "#from-notation").click(); break;
								// Если для конвертации выбрана десятичная система счисления
								case "10": $("> li > a[href=#notation-10]", "#from-notation").click(); break;
								// Если для конвертации выбрана шестнадцатеричная система счисления
								case "16": $("> li > a[href=#notation-16]", "#from-notation").click(); break;
								// Если выбрана любая другая система счисления
								default: {
									// Выполняем отключение переноса строк
									fromEditorNotation.setOption("lineWrapping", false);
									// Блокируем вкладку текстового вывода
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-10]", "#to-notation").click();
								}
							}
							// Разблокируем вкладку всех остальных элементов в выпадающем списке в котором отображается результат
							$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item", "#to-notation").removeClass("disabled");
							// Блокируем такую же систему счисления
							$(`> li.dropdown > ul.dropdown-menu > li > a.dropdown-item[href=#notation-${action}]`, "#to-notation").addClass("disabled");
							// Скрываем всплывающее поле
							$($(this).parents(".dropdown-menu")).removeClass("show");
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выпадающего списка системы счисления в которую выполняется конвертация
					$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item", "#to-notation")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Получаем значение текста ссылки
							const action = $(this).text();
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Меняем текст выбранной системой счисления
							$("> a", $(this).parents(".dropdown")).text(action);
							// Определяем тип конвертера
							switch(action){
								// Если для конвертации выбрана двоичная система счисления
								case "2": $("> li > a[href=#notation-2]", "#to-notation").click(); break;
								// Если для конвертации выбрана восьмеричная система счисления
								case "8": $("> li > a[href=#notation-8]", "#to-notation").click(); break;
								// Если для конвертации выбрана десятичная система счисления
								case "10": $("> li > a[href=#notation-10]", "#to-notation").click(); break;
								// Если для конвертации выбрана шестнадцатеричная система счисления
								case "16": $("> li > a[href=#notation-16]", "#to-notation").click(); break;
								// Если выбрана любая другая система счисления
								default:
									// Выполняем отключение переноса строк
									toEditorNotation.setOption("lineWrapping", false);
							}
							// Если текст для конвертации присутствует
							if(fromEditorNotation.getValue().length > 0)
								// Выполняем загрузку новых данных
								$("#submit-notation").click();
							// Удаляем текст в текстовом окне
							else toEditorNotation.setValue("");
							// Скрываем всплывающее поле
							$($(this).parents(".dropdown-menu")).removeClass("show");
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий вывода всплывающего списка
					$("> li > a.dropdown-toggle", "#from-notation")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий вывода всплывающего списка
					$("> li > a.dropdown-toggle", "#to-notation")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
					$("> li > a:not('.dropdown-toggle')", "#from-notation")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorNotation.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-notation").parent()).addClass("hidden");
							// Разблокируем вкладку всех остальных элементов в выпадающем списке в котором отображается результат
							$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item", "#to-notation").removeClass("disabled");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран TEXT
								case "TEXT": {
									// Выполняем включение переноса строк
									fromEditorNotation.setOption("lineWrapping", true);
									// Разблокируем вкладку бинарного блока
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-2]", "#to-notation").click();
									// Блокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").addClass("disabled");
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									$("> li > a[href=#notation-1]", "#to-notation").addClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").addClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").addClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").addClass("disabled");
									// Блокируем всех остальных элементов в выпадающем списке в котором отображается результат
									$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item", "#to-notation").addClass("disabled");
								} break;
								// Если для конвертации выбрана Римская система счисления
								case "ROME": {
									// Выполняем отключение переноса строк
									fromEditorNotation.setOption("lineWrapping", false);
									// Блокируем вкладку текстового вывода
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									// Блокируем вкладку Римских чисел вывода
									$("> li > a[href=#notation-1]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-10]", "#to-notation").click();
								} break;
								// Если для конвертации выбрана двоичная система счисления
								case "BIN": {
									// Выполняем включение переноса строк
									fromEditorNotation.setOption("lineWrapping", true);
									// Разблокируем вкладку текстового блока
									$("> li > a[href=#notation-0]", "#to-notation").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-0]", "#to-notation").click();
									// Блокируем вкладку бинарного вывода
									$("> li > a[href=#notation-2]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-1]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").removeClass("disabled");
									// Блокируем такую же систему счисления
									$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item[href=#notation-2]", "#to-notation").addClass("disabled");
								} break;
								// Если для конвертации выбрана восьмеричная система счисления
								case "OCT": {
									// Выполняем отключение переноса строк
									fromEditorNotation.setOption("lineWrapping", false);
									// Блокируем вкладку текстового вывода
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									// Блокируем вкладку восьмеричного вывода
									$("> li > a[href=#notation-8]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-1]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").removeClass("disabled");
									// Блокируем такую же систему счисления
									$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item[href=#notation-8]", "#to-notation").addClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-10]", "#to-notation").click();
								} break;
								// Если для конвертации выбрана десятичная система счисления
								case "DEC": {
									// Выполняем отключение переноса строк
									fromEditorNotation.setOption("lineWrapping", false);
									// Блокируем вкладку текстового вывода
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									// Блокируем вкладку десятичного вывода
									$("> li > a[href=#notation-10]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-1]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-16]", "#to-notation").removeClass("disabled");
									// Блокируем такую же систему счисления
									$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item[href=#notation-10]", "#to-notation").addClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-16]", "#to-notation").click();
								} break;
								// Если для конвертации выбрана шестнадцатеричная система счисления
								case "HEX": {
									// Выполняем отключение переноса строк
									fromEditorNotation.setOption("lineWrapping", false);
									// Блокируем вкладку текстового вывода
									$("> li > a[href=#notation-0]", "#to-notation").addClass("disabled");
									// Блокируем вкладку шестнадцатеричного вывода
									$("> li > a[href=#notation-16]", "#to-notation").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#notations]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-1]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-2]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-8]", "#to-notation").removeClass("disabled");
									$("> li > a[href=#notation-10]", "#to-notation").removeClass("disabled");
									// Блокируем такую же систему счисления
									$("> li.dropdown > ul.dropdown-menu > li > a.dropdown-item[href=#notation-16]", "#to-notation").addClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#notation-10]", "#to-notation").click();
								} break;
							}
						}
						// Скрываем всплывающее поле
						$(".dropdown-menu", "#from-notation").removeClass("show");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
					$("> li > a:not('.dropdown-toggle')", "#to-notation")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorNotation.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-notation").click();
						// Удаляем текст в текстовом окне
						else toEditorNotation.setValue("");
						// Определяем тип конвертера
						switch($(this).text()){
							// Если ковертер выбран TEXT
							case "TEXT":
							// Если для конвертации выбрана двоичная система счисления
							case "BIN": toEditorNotation.setOption("lineWrapping", true); break;
							// Если для конвертации выбрана Римская система счисления
							case "ROME":
							// Если для конвертации выбрана восьмеричная система счисления
							case "OCT":
							// Если для конвертации выбрана десятичная система счисления
							case "DEC":
							// Если для конвертации выбрана шестнадцатеричная система счисления
							case "HEX": toEditorNotation.setOption("lineWrapping", false); break;
						}
						// Скрываем всплывающее поле
						$(".dropdown-menu", "#to-notation").removeClass("show");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
				}
				// Если блоки подсветки синтаксиса для конвертации байт не созданы
				if((fromEditorBytes === null) || (toEditorBytes === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorBytes = CodeMirror(document.getElementById('code-editor-from-bytes'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						lineNumbers:  false,
						lineWrapping: false
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorBytes = CodeMirror(document.getElementById('code-editor-to-bytes'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						readOnly:     true,
						lineNumbers:  false,
						lineWrapping: false
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
					$("> .CodeMirror", "#code-editor-from-bytes")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorBytes.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorBytes.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-bytes")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner-bytes").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorBytes.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = "", to = "";
									// Определяем формат в который производится конвертация
									switch($("a.active", "#to-bytes").attr("href")){
										// Если формат в который производится конвертация установлен как байты
										case "#bytes": to = "bytes"; break;
										// Если формат в который производится конвертация установлен как килобайты
										case "#kb": to = "Kb"; break;
										// Если формат в который производится конвертация установлен как мегабайты
										case "#mb": to = "Mb"; break;
										// Если формат в который производится конвертация установлен как гигабайты
										case "#gb": to = "Gb"; break;
										// Если формат в который производится конвертация установлен как терабайты
										case "#tb": to = "Tb"; break;
									}
									// Определяем формат с которого производится конвертация
									switch($("a.active", "#from-bytes").attr("href")){
										// Если формат из которого производится конвертация установлен как байты
										case "#bytes": from = "bytes"; break;
										// Если формат из которого производится конвертация установлен как килобайты
										case "#kb": from = "Kb"; break;
										// Если формат из которого производится конвертация установлен как мегабайты
										case "#mb": from = "Mb"; break;
										// Если формат из которого производится конвертация установлен как гигабайты
										case "#gb": from = "Gb"; break;
										// Если формат из которого производится конвертация установлен как терабайты
										case "#tb": from = "Tb"; break;
									}
									// Если форматы для конвертации указанны
									if((to.length > 0) && (from.length > 0)){
										// Формируем объект тело запроса
										const body = {to, from, text, bytes: true};
										// Выполняем запрос на сервер
										const response = await fetch("/exec", {
											method: "POST",
											body: JSON.stringify(body),
											headers: {"Content-type": "application/json; charset=UTF-8"}
										});
										// Извлекаем полученный результат
										const answer = await response.json();
										// Если в ответе есть поле результата
										if((answer.result !== null) && (answer.result !== undefined))
											// Выполняем установку результата
											toEditorBytes.setValue(answer.result);
										// Выводим сообщение об ошибке
										else {
											// Если получен текст ошибки
											if((answer.error !== null) && (answer.error !== undefined))
												// Отображаем всплывающее сообщение
												alert("Ошибка запроса", answer.error);
											// Если текст ошибки не получен
											else alert("Ответ не получен", "Ответ от сервера не содержит результата");
										}
									// Если текст для конвертации не заполнен
									} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
								// Если текст для хеширования не заполнен
								} else alert("Ошибка хеширования", "Текст для хеширования не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner-bytes").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
					$("> li > a", "#from-bytes")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorBytes.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-bytes").parent()).addClass("hidden");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран BYTES
								case "BYTES": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#bytes]", "#to-bytes").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#kb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#mb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#gb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#tb]", "#to-bytes").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#kb]", "#to-bytes").click();
								} break;
								// Если ковертер выбран KB
								case "KB": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#kb]", "#to-bytes").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#bytes]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#mb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#gb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#tb]", "#to-bytes").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#bytes]", "#to-bytes").click();
								} break;
								// Если ковертер выбран MB
								case "MB": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#mb]", "#to-bytes").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#bytes]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#kb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#gb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#tb]", "#to-bytes").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#bytes]", "#to-bytes").click();
								} break;
								// Если ковертер выбран GB
								case "GB": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#gb]", "#to-bytes").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#bytes]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#kb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#mb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#tb]", "#to-bytes").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#bytes]", "#to-bytes").click();
								} break;
								// Если ковертер выбран TB
								case "TB": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#tb]", "#to-bytes").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#bytes]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#kb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#mb]", "#to-bytes").removeClass("disabled");
									$("> li > a[href=#gb]", "#to-bytes").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#bytes]", "#to-bytes").click();
								} break;
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
					$("> li > a", "#to-bytes")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorBytes.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-bytes").click();
						// Удаляем текст в текстовом окне
						else toEditorBytes.setValue("");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
				}
				// Если блоки подсветки синтаксиса для конвертации секунд не созданы
				if((fromEditorSeconds === null) || (toEditorSeconds === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorSeconds = CodeMirror(document.getElementById('code-editor-from-seconds'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						lineNumbers:  false,
						lineWrapping: false
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorSeconds = CodeMirror(document.getElementById('code-editor-to-seconds'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						readOnly:     true,
						lineNumbers:  false,
						lineWrapping: true
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
					$("> .CodeMirror", "#code-editor-from-seconds")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorSeconds.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorSeconds.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-seconds")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner-seconds").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorSeconds.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = "", to = "";
									// Определяем формат в который производится конвертация
									switch($("a.active", "#to-seconds").attr("href")){
										// Если формат в который производится конвертация установлен как секунды
										case "#seconds": to = "s"; break;
										// Если формат в который производится конвертация установлен как минуты
										case "#minutes": to = "m"; break;
										// Если формат в который производится конвертация установлен как часы
										case "#hours": to = "h"; break;
										// Если формат в который производится конвертация установлен как дни
										case "#days": to = "d"; break;
										// Если формат в который производится конвертация установлен как недели
										case "#weeks": to = "w"; break;
										// Если формат в который производится конвертация установлен как месяцы
										case "#months": to = "M"; break;
										// Если формат в который производится конвертация установлен как годы
										case "#years": to = "y"; break;
									}
									// Определяем формат с которого производится конвертация
									switch($("a.active", "#from-seconds").attr("href")){
										// Если формат из которого производится конвертация установлен как секунды
										case "#seconds": from = "s"; break;
										// Если формат из которого производится конвертация установлен как минуты
										case "#minutes": from = "m"; break;
										// Если формат из которого производится конвертация установлен как часы
										case "#hours": from = "h"; break;
										// Если формат из которого производится конвертация установлен как дни
										case "#days": from = "d"; break;
										// Если формат из которого производится конвертация установлен как недели
										case "#weeks": from = "w"; break;
										// Если формат из которого производится конвертация установлен как месяцы
										case "#months": from = "M"; break;
										// Если формат из которого производится конвертация установлен как годы
										case "#years": from = "y"; break;
									}
									// Если форматы для конвертации указанны
									if((to.length > 0) && (from.length > 0)){
										// Формируем объект тело запроса
										const body = {to, from, text, seconds: true};
										// Выполняем запрос на сервер
										const response = await fetch("/exec", {
											method: "POST",
											body: JSON.stringify(body),
											headers: {"Content-type": "application/json; charset=UTF-8"}
										});
										// Извлекаем полученный результат
										const answer = await response.json();
										// Если в ответе есть поле результата
										if((answer.result !== null) && (answer.result !== undefined))
											// Выполняем установку результата
											toEditorSeconds.setValue(answer.result);
										// Выводим сообщение об ошибке
										else {
											// Если получен текст ошибки
											if((answer.error !== null) && (answer.error !== undefined))
												// Отображаем всплывающее сообщение
												alert("Ошибка запроса", answer.error);
											// Если текст ошибки не получен
											else alert("Ответ не получен", "Ответ от сервера не содержит результата");
										}
									// Если текст для конвертации не заполнен
									} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
								// Если текст для хеширования не заполнен
								} else alert("Ошибка хеширования", "Текст для хеширования не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner-seconds").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
					$("> li > a", "#from-seconds")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorSeconds.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-seconds").parent()).addClass("hidden");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран SEC
								case "SEC": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#seconds]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#minutes]", "#to-seconds").click();
								} break;
								// Если ковертер выбран MIN
								case "MIN": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#minutes]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
								// Если ковертер выбран HOUR
								case "HOUR": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#hours]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
								// Если ковертер выбран DAY
								case "DAY": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#days]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
								// Если ковертер выбран WEEK
								case "WEEK": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#weeks]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
								// Если ковертер выбран MON
								case "MON": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#months]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#years]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
								// Если ковертер выбран YEAR
								case "YEAR": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#years]", "#to-seconds").addClass("disabled");
									// Разблокируем все остальные вкладки
									$("> li > a[href=#seconds]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#minutes]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#hours]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#days]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#weeks]", "#to-seconds").removeClass("disabled");
									$("> li > a[href=#months]", "#to-seconds").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#seconds]", "#to-seconds").click();
								} break;
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
					$("> li > a", "#to-seconds")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorSeconds.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-seconds").click();
						// Удаляем текст в текстовом окне
						else toEditorSeconds.setValue("");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
				}
				// Если блоки подсветки синтаксиса для генерации даты не созданы
				if((fromEditorDate === null) || (toEditorDate === null)){
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
					fromEditorDate = CodeMirror(document.getElementById('code-editor-from-date'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						lineNumbers:  false,
						lineWrapping: false
					});
					// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
					toEditorDate = CodeMirror(document.getElementById('code-editor-to-date'), {
						tabSize:      4,
						mode:         'shell',
						theme:        'eclipse',
						readOnly:     true,
						lineNumbers:  false,
						lineWrapping: true
					});
					// Устанавливаем событие на изменение поля из которого следует выполнять генерацию
					$("> .CodeMirror", "#code-editor-from-date")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if(fromEditorDate.getValue().length > 0)
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							fromEditorDate.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
					// Устанавливаем событие на кнопку отправки запроса
					$("#submit-date")
					.unbind('click')
					.bind('click', function(){
						// Включаем индикатор загрузки
						$("#spinner-date").removeClass("hidden");
						/**
						 * Формируем функцию выполнения запроса на удалённый сервер
						 */
						(async () => {
							/**
							 * Выполняем перехват ошибки
							 */
							try {
								// Получаем значение текста который необходимо сконвертировать
								const text = fromEditorDate.getValue();
								// Если текст для конвертации указан
								if(text.length > 0){
									// Название конвертера из которого и в который производится конвертация
									let from = "", to = "";
									// Определяем формат в который производится конвертация
									switch($("a.active", "#to-date").attr("href")){
										// Если необходимо получить дату после конвертации
										case "#date": to = "date"; break;
										// Если необходимо получить штамп времени после конвертации
										case "#timestamp": to = "timestamp"; break;
									}
									// Определяем формат с которого производится конвертация
									switch($("a.active", "#from-date").attr("href")){
										// Если необходимо сконвертировать дату в штамп времени
										case "#date": from = "date"; break;
										// Если необходимо сконвертировать штамп времени в дату
										case "#timestamp": from = "timestamp"; break;
									}
									// Если форматы для конвертации указанны
									if((to.length > 0) && (from.length > 0)){
										// Получаем формат даты для формирования итогового результата
										const formatDate = $("#formatDate").val();
										// Формируем объект тело запроса
										const body = {to, from, text, formatDate, date: true};
										// Выполняем запрос на сервер
										const response = await fetch("/exec", {
											method: "POST",
											body: JSON.stringify(body),
											headers: {"Content-type": "application/json; charset=UTF-8"}
										});
										// Извлекаем полученный результат
										const answer = await response.json();
										// Если в ответе есть поле результата
										if((answer.result !== null) && (answer.result !== undefined))
											// Выполняем установку результата
											toEditorDate.setValue(answer.result);
										// Выводим сообщение об ошибке
										else {
											// Если получен текст ошибки
											if((answer.error !== null) && (answer.error !== undefined))
												// Отображаем всплывающее сообщение
												alert("Ошибка запроса", answer.error);
											// Если текст ошибки не получен
											else alert("Ответ не получен", "Ответ от сервера не содержит результата");
										}
									// Если текст для конвертации не заполнен
									} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
								// Если текст для хеширования не заполнен
								} else alert("Ошибка хеширования", "Текст для хеширования не заполнен");
							/**
							 * Если ошибка перехвачена
							 */
							} catch(error) {
								// Отображаем всплывающее сообщение
								alert("Ошибка запроса", error);
							}
						})();
						// Выключаем индикатор загрузки
						$("#spinner-date").addClass("hidden");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата из чего нужно сгенерировать
					$("> li > a", "#from-date")
					.unbind('click')
					.bind('click', function(){
						// Если флаг активации ещё не установлен
						if(!$(this).hasClass("active")){
							// Снимаем флаг активации у уже активного элемента
							$("li > a.active", $(this).parent().parent())
							.removeClass("active")
							.removeAttr("aria-current");
							// Устанавливаем флаг активации
							$(this)
							.addClass("active")
							.attr("aria-current", "page");
							// Удаляем текст в текстовом окне
							fromEditorDate.setValue("");
							// Выполняем скрытие кнопки очитски
							$(".btn-close", $("#code-editor-from-date").parent()).addClass("hidden");
							// Определяем тип конвертера
							switch($(this).text()){
								// Если ковертер выбран TIMESTAMP
								case "TIMESTAMP": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#timestamp]", "#to-date").addClass("disabled");
									// Разблокируем вкладку конвертации в дату
									$("> li > a[href=#date]", "#to-date").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#date]", "#to-date").click();
								} break;
								// Если ковертер выбран DATE
								case "DATE": {
									// Блокируем вкладку вывода такого же результата
									$("> li > a[href=#date]", "#to-date").addClass("disabled");
									// Разблокируем вкладку конвертации в штамп времени
									$("> li > a[href=#timestamp]", "#to-date").removeClass("disabled");
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#timestamp]", "#to-date").click();
								} break;
							}
						}
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на перехват событий выбора формата в который нужно сгенерировать
					$("> li > a", "#to-date")
					.unbind('click')
					.bind('click', function(){
						// Снимаем флаг активации у уже активного элемента
						$("li > a.active", $(this).parent().parent())
						.removeClass("active")
						.removeAttr("aria-current");
						// Устанавливаем флаг активации
						$(this)
						.addClass("active")
						.attr("aria-current", "page");
						// Если текст для конвертации присутствует
						if(fromEditorDate.getValue().length > 0)
							// Выполняем загрузку новых данных
							$("#submit-date").click();
						// Удаляем текст в текстовом окне
						else toEditorDate.setValue("");
						// Запрещаем дальнейшие действия для ссылки
						return false;
					});
					// Устанавливаем событие на текстовое поле ввода формата даты
					$("#formatDate")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем объект кнопки очистки текстового поля
						const button = $(".btn-close", $(this).parent());
						// Если текст введён в текстовое поле
						if($(this).val().length > 0)
							// Выполняем отображение кнопки очистки текстового поля
							button.removeClass("hidden");
						// Скрываем кнопку очистки текстового поля
						else button.addClass("hidden");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на клик по кнопке
					.parent()
					.find(".btn-close")
					.unbind('click')
					.bind('click', function(){
						// Скрываем кнопку очистки текстового поля
						$(this).addClass("hidden");
						// Выполняем очистку текстового поля
						$("#formatDate").val("");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					});
				}
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на изменение поля из которого следует выполнять конвертацию
		$("> .CodeMirror", "#code-editor-from-containers")
		.unbind("keyup")
		.bind("keyup", function(){
			// Получаем идентификатор кнопки
			const button = $(".btn-close", $(this).parent().parent());
			// Если поле содержит данные
			if(fromEditorContainers.getValue().length > 0)
				// Выполняем отображение кнопки очистки поля
				button.removeClass("hidden");
			// Выполняем скрытие кнопки очистки
			else button.addClass("hidden");
			// Устанавливаем событие на клик по кнопке
			button
			.unbind('click')
			.bind('click', function(){
				// Выполняем очистку текстового поля
				fromEditorContainers.setValue("");
				// Выполняем скрытие кнопки
				$(this).addClass("hidden");
				// Запрещаем дальнейшее событие для кнопки
				return false;
			});
			// Запрещаем выполнение дальнейшего события
			return false;
		});
		// Устанавливаем событие на кнопку отправки запроса
		$("#submit-containers")
		.unbind('click')
		.bind('click', function(){
			// Включаем индикатор загрузки
			$("#spinner-containers").removeClass("hidden");
			/**
			 * Формируем функцию выполнения запроса на удалённый сервер
			 */
			(async () => {
				/**
				 * Выполняем перехват ошибки
				 */
				try {
					// Получаем значение текста который необходимо сконвертировать
					const text = fromEditorContainers.getValue();
					// Если текст для конвертации указан
					if(text.length > 0){
						// Название конвертера из которого и в который производится конвертация
						let from = "", to = "";
						// Определяем формат в который производится конвертация
						switch($("a.active", "#to-containers").attr("href")){
							// Если формат в который производится конвертация установлен как INI
							case "#INI": to = "ini"; break;
							// Если формат в который производится конвертация установлен как XML
							case "#XML": to = "xml"; break;
							// Если формат в который производится конвертация установлен как CSV
							case "#CSV": to = "csv"; break;
							// Если формат в который производится конвертация установлен как CEF
							case "#CEF": to = "cef"; break;
							// Если формат в который производится конвертация установлен как JSON
							case "#JSON": to = "json"; break;
							// Если формат в который производится конвертация установлен как YAML
							case "#YAML": to = "yaml"; break;
							// Если формат в который производится конвертация установлен как SYSLOG
							case "#SYSLOG": to = "syslog"; break;
						}
						// Определяем формат с которого производится конвертация
						switch($("a.active", "#from-containers").attr("href")){
							// Если формат из которого производится конвертация установлен как INI
							case "#INI": from = "ini"; break;
							// Если формат из которого производится конвертация установлен как XML
							case "#XML": from = "xml"; break;
							// Если формат из которого производится конвертация установлен как CSV
							case "#CSV": from = "csv"; break;
							// Если формат из которого производится конвертация установлен как CEF
							case "#CEF": from = "cef"; break;
							// Если формат из которого производится конвертация установлен как JSON
							case "#JSON": from = "json"; break;
							// Если формат из которого производится конвертация установлен как YAML
							case "#YAML": from = "yaml"; break;
							// Если формат из которого производится конвертация установлен как GROK
							case "#GROK": from = "grok"; break;
							// Если формат из которого производится конвертация установлен как SYSLOG
							case "#SYSLOG": from = "syslog"; break;
						}
						// Если форматы для конвертации указанны
						if((to.length > 0) && (from.length > 0)){
							// Получаем данные текстового поля регулярного выражения в формате GROK
							const express = ((from === "grok") ? $("#grok-expression").val() : undefined);
							// Если выбран формат GROK но регулярное выражение не запущено
							if((from === "grok") && (express.length === 0))
								// Отображаем всплывающее сообщение
								alert("Ошибка конвертации", "Регулярное выражение <strong>в формате GROK</strong> не заполнено");
							// Если всё хорошо то продолжаем дальше
							else {
								// Получаем данные шаблонов
								let patterns = ((from === "grok") && (grokEditor !== null) ? grokEditor.getValue() : undefined);
								// Если выбран формат GROK и указаны шаблоны
								if((from === "grok") && (patterns.length > 0))
									// Выполняем парсинг блока шаблонов
									patterns = JSON.parse(patterns);
								// Если шаблон пустой
								else if((from === "grok") && (patterns.length === 0))
									// Выполняем удаление шаблонов
									patterns = undefined;
								// Формируем объект тело запроса
								const body = {
									to, from, text, express, patterns,
									prettify: $("#prettify").is(":checked"),
									header: (((from === "csv") || (to === "csv")) ? $("#csv-header").is(":checked") : undefined)
								};
								// Выполняем запрос на сервер
								const response = await fetch("/exec", {
									method: "POST",
									body: JSON.stringify(body),
									headers: {"Content-type": "application/json; charset=UTF-8"}
								});
								// Извлекаем полученный результат
								const answer = await response.json();
								// Если в ответе есть поле результата
								if((answer.result !== null) && (answer.result !== undefined))
									// Выполняем установку результата
									toEditorContainers.setValue(answer.result);
								// Выводим сообщение об ошибке
								else {
									// Если получен текст ошибки
									if((answer.error !== null) && (answer.error !== undefined))
										// Отображаем всплывающее сообщение
										alert("Ошибка запроса", answer.error);
									// Если текст ошибки не получен
									else alert("Ответ не получен", "Ответ от сервера не содержит результата");
								}
							}
						// Если текст для конвертации не заполнен
						} else alert("Ошибка формата конвертации", "Формат для конвертации не указан");
					// Если текст для конвертации не заполнен
					} else alert("Ошибка конвертации", "Текст для конвертации не заполнен");
				/**
				 * Если ошибка перехвачена
				 */
				} catch(error) {
					// Отображаем всплывающее сообщение
					alert("Ошибка запроса", error);
				}
			})();
			// Выключаем индикатор загрузки
			$("#spinner-containers").addClass("hidden");
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
		$("> li > a", "#from-containers")
		.unbind('click')
		.bind('click', function(){
			// Если флаг активации ещё не установлен
			if(!$(this).hasClass("active")){
				// Снимаем флаг активации у уже активного элемента
				$("li > a.active", $(this).parent().parent())
				.removeClass("active")
				.removeAttr("aria-current");
				// Устанавливаем флаг активации
				$(this)
				.addClass("active")
				.attr("aria-current", "page");
				// Удаляем текст в текстовом окне
				fromEditorContainers.setValue("");
				// Выполняем скрытие кнопки очитски
				$(".btn-close", $("#code-editor-from-containers").parent()).addClass("hidden");
				// Получаем название парсера
				const name = $(this).text();
				// Если мы работаем с парсером GROK
				if(name.localeCompare("GROK") == 0){
					// Отображаем текстовое поле формирования шаблонов GROK
					$("#grok-template").removeClass("hidden");
					// Отображаем текстовое поле регулярного выражения
					$("#grok-expression").parent().removeClass("hidden");
					// Формируем блок подсветки синтаксиса
					grokEditor = CodeMirror(document.getElementById('code-editor-grok'), {
						tabSize: 4,
						theme: 'eclipse',
						mode: 'javascript',
						lineNumbers: false
					});
					// Активируем подсветку синтаксиса
					fromEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					fromEditorContainers.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					fromEditorContainers.setOption("lineWrapping", true);
					// Если заголовок CSV не активен
					if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
					// Блокируем вкладку конвертера
					$("> li > a[href=#INI]", "#to-containers").addClass("disabled");
					$("> li > a[href=#CSV]", "#to-containers").addClass("disabled");
					$("> li > a[href=#CEF]", "#to-containers").addClass("disabled");
					$("> li > a[href=#SYSLOG]", "#to-containers").addClass("disabled");
					// Получаем активный элемент
					switch($("> li > a.active", "#to-containers").text()){
						// Если ковертер выбран INI
						case "INI":
						// Если ковертер выбран CSV
						case "CSV":
						// Если ковертер выбран CEF
						case "CEF":
						// Если ковертер выбран SYSLOG
						case "SYSLOG":
							// Переключаем на вкладку по умолчанию
							$("> li > a[href=#JSON]", "#to-containers").click();
						break;
					}
					// Устанавливаем событие на текстовое поле ввода регулярного адреса
					$("#grok-expression")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем объект кнопки очистки текстового поля
						const button = $(".btn-close", $(this).parent());
						// Если текст введён в текстовое поле
						if($(this).val().length > 0)
							// Выполняем отображение кнопки очистки текстового поля
							button.removeClass("hidden");
						// Скрываем кнопку очистки текстового поля
						else button.addClass("hidden");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на клик по кнопке
					.parent()
					.find(".btn-close")
					.unbind('click')
					.bind('click', function(){
						// Скрываем кнопку очистки текстового поля
						$(this).addClass("hidden");
						// Выполняем очистку текстового поля
						$("#grok-expression").val("");
						// Запрещаем дальнейшее событие для кнопки
						return false;
					})
					// Устанавливаем событие на изменение поля для ввода шаблонов GROK
					$("> .CodeMirror", "#code-editor-grok")
					.unbind("keyup")
					.bind("keyup", function(){
						// Получаем идентификатор кнопки
						const button = $(".btn-close", $(this).parent().parent());
						// Если поле содержит данные
						if((grokEditor !== null) && (grokEditor.getValue().length > 0))
							// Выполняем отображение кнопки очистки поля
							button.removeClass("hidden");
						// Выполняем скрытие кнопки очистки
						else button.addClass("hidden");
						// Устанавливаем событие на клик по кнопке
						button
						.unbind('click')
						.bind('click', function(){
							// Выполняем очистку текстового поля
							grokEditor.setValue("");
							// Выполняем скрытие кнопки
							$(this).addClass("hidden");
							// Запрещаем дальнейшее событие для кнопки
							return false;
						});
						// Запрещаем выполнение дальнейшего события
						return false;
					});
				// Если мы работаем с другими парсерами
				} else {
					// Если блок ввода шаблонов GROK не скрыт
					if(!$("#grok-template").hasClass("hidden")){
						// Скрываем текстовое поле формирования шаблонов GROK
						$("#grok-template").addClass("hidden");
						// Скрываем текстовое поле регулярного выражения
						$("#grok-expression").parent().addClass("hidden");
						// Выполняем удаление блока подсветки синтаксиса
						grokEditor.getWrapperElement().parentNode.removeChild(grokEditor.getWrapperElement());
						// Зануляем объект блока подсветки синтаксиса
						grokEditor = null;
					}
					// Определяем тип конвертера
					switch(name){
						// Если ковертер выбран XML
						case "XML": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "xml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("> li > a[href=#CSV]", "#to-containers").addClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").addClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").addClass("disabled");
							// Снимаем блокировку вкладки конвертера
							$("> li > a[href=#INI]", "#to-containers").removeClass("disabled");
						} break;
						// Если ковертер выбран JSON
						case "JSON": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "javascript");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("> li > a[href=#INI]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#CSV]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").removeClass("disabled");
						} break;
						// Если ковертер выбран YAML
						case "YAML": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "yaml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Если заголовок CSV не активен
							if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("> li > a[href=#INI]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#CSV]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").removeClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").removeClass("disabled");
						} break;
						// Если ковертер выбран INI
						case "INI": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "toml");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем подключение переноса строк
							fromEditorContainers.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("> li > a[href=#INI]", "#to-containers").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("> li > a[href=#CSV]", "#to-containers").addClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").addClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").addClass("disabled");
							// Получаем активный элемент
							switch($("> li > a.active", "#to-containers").text()){
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#JSON]", "#to-containers").click();
								break;
							}
						} break;
						// Если ковертер выбран CSV
						case "CSV": {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", true);
							// Выполняем отключение переноса строк
							fromEditorContainers.setOption("lineWrapping", false);
							// Отображаем переключатель вывода заголовков
							$("#csv-header").parent().removeClass("hidden");
							// Снимаем блокировку вкладки конвертера
							$("> li > a[href=#CSV]", "#to-containers").removeClass("disabled");
							// Блокируем вкладку конвертера
							$("> li > a[href=#INI]", "#to-containers").addClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").addClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").addClass("disabled");
							// Получаем активный элемент
							switch($("> li > a.active", "#to-containers").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#JSON]", "#to-containers").click();
								break;
							}
						} break;
						// Если ковертер выбран любой другой
						default: {
							// Активируем подсветку синтаксиса
							fromEditorContainers.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditorContainers.setOption("lineNumbers", false);
							// Выполняем подключение переноса строк
							fromEditorContainers.setOption("lineWrapping", true);
							// Если заголовок CSV не активен
							if($("> li > a[href=#CSV]", "#to-containers").hasClass("active"))
								// Отображаем переключатель вывода заголовков
								$("#csv-header").parent().removeClass("hidden");
							// Скрываем переключатель формирования заголовков
							else $("#csv-header").parent().addClass("hidden");
							// Блокируем вкладку конвертера
							$("> li > a[href=#INI]", "#to-containers").addClass("disabled");
							$("> li > a[href=#CSV]", "#to-containers").addClass("disabled");
							$("> li > a[href=#CEF]", "#to-containers").addClass("disabled");
							$("> li > a[href=#SYSLOG]", "#to-containers").addClass("disabled");
							// Получаем активный элемент
							switch($("> li > a.active", "#to-containers").text()){
								// Если ковертер выбран INI
								case "INI":
								// Если ковертер выбран CSV
								case "CSV":
								// Если ковертер выбран CEF
								case "CEF":
								// Если ковертер выбран SYSLOG
								case "SYSLOG":
									// Переключаем на вкладку по умолчанию
									$("> li > a[href=#JSON]", "#to-containers").click();
								break;
							}
						}
					}
				}
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
		$("> li > a", "#to-containers")
		.unbind('click')
		.bind('click', function(){
			// Снимаем флаг активации у уже активного элемента
			$("li > a.active", $(this).parent().parent())
			.removeClass("active")
			.removeAttr("aria-current");
			// Устанавливаем флаг активации
			$(this)
			.addClass("active")
			.attr("aria-current", "page");
			// Если текст для конвертации присутствует
			if(fromEditorContainers.getValue().length > 0)
				// Выполняем загрузку новых данных
				$("#submit-containers").click();
			// Удаляем текст в текстовом окне
			else toEditorContainers.setValue("");
			// Определяем тип конвертера
			switch($(this).text()){
				// Если ковертер выбран XML
				case "XML": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "xml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("> li > a[href=#CSV]", "#from-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран JSON
				case "JSON": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "javascript");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().removeClass("hidden");
					// Если заголовок CSV не активен
					if($("> li > a[href=#CSV]", "#from-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран YAML
				case "YAML": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "yaml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("> li >a[href=#CSV]", "#from-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран INI
				case "INI": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "toml");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Выполняем подключение переноса строк
					toEditorContainers.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("> li > a[href=#CSV]", "#from-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				} break;
				// Если ковертер выбран CSV
				case "CSV": {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Отображаем переключатель вывода заголовков
					$("#csv-header").parent().removeClass("hidden");
				} break;
				// Если ковертер выбран любой другой
				default: {
					// Активируем подсветку синтаксиса
					toEditorContainers.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditorContainers.setOption("lineNumbers", false);
					// Выполняем подключение переноса строк
					toEditorContainers.setOption("lineWrapping", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$("#prettify").parent().addClass("hidden");
					// Если заголовок CSV не активен
					if($("> li > a[href=#CSV]", "#from-containers").hasClass("active"))
						// Отображаем переключатель вывода заголовков
						$("#csv-header").parent().removeClass("hidden");
					// Скрываем переключатель формирования заголовков
					else $("#csv-header").parent().addClass("hidden");
				}
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
	});
})(jQuery);
