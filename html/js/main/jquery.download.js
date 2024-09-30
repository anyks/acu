(function($){
	// Активируем событие загрузки
	$(document).ready(function(){
		/**
		 * OS Типы операционных систем
		 */
		const OS = {
			NONE:        0x00, // Операционная система не обнаружена
			MACOS:       0x01, // Операционная система MacOS X
			MACOS_M1:    0x02, // Операционная система MacOS X M1
			MACOS_INTEL: 0x03, // Операционная система MacOS X Intel
			WINDOWS:     0x04, // Операционная система Windows
			LINUX:       0x05, // Операционная система Linux
			FREEBSD:     0x06  // Операционная система FreeBSD
		};
		/**
		 * getOs Функция определения операционной системы
		 */
		const getOs = () => {
			// Результат работы функции
			let result = OS.NONE;
			// Выполняем создание Canvas с поддержкой WebGL
			const wgl = document.createElement("canvas").getContext("webgl");
			// Извлекаем информацию оборудования
			const debug = wgl.getExtension('WEBGL_debug_renderer_info');
			// Выполняем извлечение информации об оборудовании
			const info = (debug && wgl.getParameter(debug.UNMASKED_RENDERER_WEBGL) || "");
			// Если информация получена
			if(info.length > 0){
				// Если операционная система обнаружена как Apple
				if(/Apple/.test(info)){
					// Если мы нашли Macbook M1
					if(/M1/.test(info))
						// Устанавливаем результат
						result = OS.MACOS_M1;
					// Устанавливаем старый процессор
					else result = OS.MACOS_INTEL;
				// Если мы нашли операционную систему Windows
				} else if(/Direct3D/.test(info))
					// Устанавливаем результат
					result = OS.WINDOWS;
			}
			// Если результат не обнаружен
			if(result == OS.NONE){
				// Определяем операционную систему MacOS X
				if(/Macintosh/.test(navigator.userAgent))
					// Устанавливаем результат
					result = OS.MACOS;
				// Определяем операционную систему Linux
				else if(/Linux/.test(navigator.userAgent))
					// Устанавливаем результат
					result = OS.LINUX;
				// Определяем операционную систему FreeBSD
				else if(/FreeBSD/.test(navigator.userAgent))
					// Устанавливаем результат
					result = OS.FREEBSD;
				// Определяем операционную систему Windows
				else if(/Windows/.test(navigator.userAgent))
					// Устанавливаем результат
					result = OS.WINDOWS;
			}
			// Выводим результат
			return result;
		};
		// Определяем тип операционной системы
		switch(getOs()){
			// Если операционная система обнаружена как MacOS X
			case OS.MACOS: {
				// Выполняем получение всего списка ссылок
				const html = $("#macos").html();
				// Скрываем кнопку закачки приложения
				$("#download").addClass("hidden");
				// Устанавливаем список ссылок для скачивания
				$(".dropdown-menu", "#download_some")
				.html(html)
				.parent()
				.removeClass("hidden");
			} break;
			// Если операционная система обнаружена как MacOS X M1
			case OS.MACOS_M1: {
				// Выполняем получение ссылки
				const url = $("a[architecture=\"arm\"]", "#macos").attr("href");
				// Устанавливаем адрес загрузки приложения
				$("#download").attr("href", url);
			} break;
			// Если операционная система обнаружена как MacOS X Intel
			case OS.MACOS_INTEL: {
				// Выполняем получение ссылки
				const url = $("a[architecture=\"intel\"]", "#macos").attr("href");
				// Устанавливаем адрес загрузки приложения
				$("#download").attr("href", url);
			} break;
			// Если операционная система обнаружена как Windows
			case OS.WINDOWS: {
				// Выполняем получение ссылки
				const url = $("a[pkg=\"exe\"]", "#windows").attr("href");
				// Устанавливаем адрес загрузки приложения
				$("#download").attr("href", url);
			} break;
			// Если операционная система обнаружена как Linux
			case OS.LINUX: {
				// Выполняем получение всего списка ссылок
				const html = $("#linux").html();
				// Скрываем кнопку закачки приложения
				$("#download").addClass("hidden");
				// Устанавливаем список ссылок для скачивания
				$(".dropdown-menu", "#download_some")
				.html(html)
				.parent()
				.removeClass("hidden");
			} break;
			// Если операционная система обнаружена как FreeBSD
			case OS.FREEBSD: {
				// Выполняем получение ссылки
				const url = $("a[pkg=\"tar.gz\"]", "#freebsd").attr("href");
				// Устанавливаем адрес загрузки приложения
				$("#download").attr("href", url);
			} break;
			// Если операционная система не определена
			case OS.NONE:
				// Скрываем кнопку закачки приложения
				$("#download").addClass("hidden");
			break;
		}
	});
})(jQuery);
