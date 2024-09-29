(function($){
	// Активируем событие загрузки
	$(document).ready(function(){
		
		/**
		 * console.log("+++++++", window.k11.getValue());
		 * 
		 * 
		 */

		(async () => {

			let k = await navigator.userAgentData.getHighEntropyValues(['architecture']);
		
			console.log(k.architecture, "==", k.platform);
		})();

		// Блок подсветки синтаксиса шаблонов GROK
		let grokEditor = null;
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата из которого производится конвертация
		let fromEditor = CodeMirror(document.getElementById('code-editor-from'), {
			tabSize: 4,
			mode: 'xml',
			theme: 'eclipse',
			lineNumbers: true
		});
		// Выполняем инициализацию блока подсветки синтаксиса для редактора формата в который производится конвертация
		const toEditor = CodeMirror(document.getElementById('code-editor-to'), {
			tabSize: 4,
			theme: 'eclipse',
			mode: 'javascript',
			lineNumbers: true
		});
		// Устанавливаем событие на кнопку отправки запроса
		$("#submit")
		.unbind('click')
		.bind('click', function(){
			/**
			 * Формируем функцию выполнения запроса на удалённый сервер
			 */
			(async () => {
				// Формируем объект тело запроса
				const body = {
					userId: 1,
					title: "Fix my bugs",
					completed: false
				};
				// Выполняем запрос на сервер
				const response = await fetch("https://jsonplaceholder.typicode.com/todos", {
					method: "POST",
					body: JSON.stringify(body),
					headers: {"Content-type": "application/json; charset=UTF-8"}
				});
				// Извлекаем полученный результат
				const data = await response.json();

				console.log("-----------", data.title);

			})();
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата из чего нужно конвертировать
		$("#from > li > a")
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
				fromEditor.setValue("");
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
					fromEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					fromEditor.setOption("lineNumbers", false);
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
							fromEditor.setOption("mode", "xml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
						} break;
						// Если ковертер выбран JSON
						case "JSON": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "javascript");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
						} break;
						// Если ковертер выбран YAML
						case "YAML": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "yaml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
						} break;
						// Если ковертер выбран INI
						case "INI": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "toml");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
						} break;
						// Если ковертер выбран CSV
						case "CSV": {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", true);
						} break;
						// Если ковертер выбран любой другой
						default: {
							// Активируем подсветку синтаксиса
							fromEditor.setOption("mode", "shell");
							// Активируем вывод номеров строк
							fromEditor.setOption("lineNumbers", false);
						} break;
					}
				}
			}			
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
		// Устанавливаем событие на перехват событий выбора формата в который нужно конвертировать
		$("#to > li > a")
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
			// Удаляем текст в текстовом окне
			toEditor.setValue("");
			// Определяем тип конвертера
			switch($(this).text()){
				// Если ковертер выбран XML
				case "XML": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "xml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").removeClass("hidden");
				} break;
				// Если ковертер выбран JSON
				case "JSON": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "javascript");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Отображаем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").removeClass("hidden");
				} break;
				// Если ковертер выбран YAML
				case "YAML": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "yaml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").addClass("hidden");
				} break;
				// Если ковертер выбран INI
				case "INI": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "toml");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").addClass("hidden");
				} break;
				// Если ковертер выбран CSV
				case "CSV": {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", true);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").addClass("hidden");
				} break;
				// Если ковертер выбран любой другой
				default: {
					// Активируем подсветку синтаксиса
					toEditor.setOption("mode", "shell");
					// Активируем вывод номеров строк
					toEditor.setOption("lineNumbers", false);
					// Скрываем переключатель вывода сформированного кода в читаемом виде
					$(".form-switch", "#formatter").addClass("hidden");
				} break;
			}
			// Запрещаем дальнейшие действия для ссылки
			return false;
		});
	});
})(jQuery);
