📱 Описание
Конвертер изображений (HEIC) — это консольная утилита для преобразования изображений в формате HEIC (High Efficiency Image Format, используемый Apple) в распространённые форматы JPEG и PNG, а также обратно. Программа поддерживает:

✅ Конвертация HEIC → JPEG, PNG, WebP

✅ Конвертация JPEG, PNG → HEIC (опционально)

✅ Настройка качества сжатия

✅ Изменение размера (пропорциональное или точное)

✅ Сохранение метаданных (EXIF, цветовой профиль)

✅ Пакетная обработка нескольких файлов

✅ Прогресс-бар и подробные логи

✅ Кроссплатформенность (Linux, macOS, Windows)

Проект содержит 8 полноценных реализаций на разных языках программирования. Все версии используют современные библиотеки для работы с HEIC и предоставляют единый интерфейс командной строки.

✨ Возможности
Функция	Описание
HEIC → JPG/PNG	Преобразование с сохранением качества
JPG/PNG → HEIC	Обратная конвертация (если поддерживается)
Качество (1–100)	Настройка степени сжатия
Изменение размера	Пропорциональное или точное изменение ширины/высоты
Пакетная обработка	Конвертация всех файлов в папке
Прогресс-бар	Отображение хода выполнения
Перезапись	Опция перезаписи существующих файлов
Рекурсивный обход	Обработка файлов во всех подпапках
📦 Установка и запуск
Каждая реализация находится в отдельной папке. Для запуска требуется соответствующий компилятор/интерпретатор и библиотеки.

Язык	Файл	Зависимости	Команда запуска
Node.js	convert.js	sharp, glob	npm install sharp glob && node convert.js image.heic
Python	convert.py	pillow-heif, Pillow	pip install pillow-heif Pillow && python3 convert.py image.heic
Go	convert.go	github.com/go-heic/heic	go mod init && go get github.com/go-heic/heic && go run convert.go image.heic
C#	convert.cs	SixLabors.ImageSharp (с HEIC)	dotnet add package SixLabors.ImageSharp && dotnet run image.heic
Ruby	convert.rb	mini_magick	gem install mini_magick && ruby convert.rb image.heic
Rust	convert.rs	image, heic	cargo add image heic && cargo run -- image.heic
C++	convert.cpp	libheif, libjpeg, libpng	g++ -std=c++17 -o convert convert.cpp -lheif -ljpeg -lpng && ./convert image.heic
Java	Convert.java	com.github.sealedtx:heic-imageio	javac -cp heic-imageio.jar Convert.java && java -cp .:heic-imageio.jar Convert image.heic
Примечание: Для всех реализаций доступны общие опции: --format jpg/png/webp (выходной формат), --quality N (качество), --resize WxH (изменение размера), --output DIR (папка для сохранения), --overwrite (перезапись), --recursive (обработка подпапок).

📂 Структура репозитория
text
.
├── README.md
├── python/
│   └── convert.py
├── go/
│   └── convert.go
├── rust/
│   ├── Cargo.toml
│   └── src/
│       └── main.rs
├── cpp/
│   └── convert.cpp
├── java/
│   └── Convert.java
├── csharp/
│   └── convert.cs
├── ruby/
│   └── convert.rb
└── javascript/
    ├── package.json
    └── convert.js
🎮 Использование
bash
# Конвертация HEIC в JPG (по умолчанию)
convert image.heic

# Конвертация в PNG с качеством 90
convert image.heic --format png --quality 90

# Конвертация и изменение размера до 800x600
convert image.heic --resize 800x600

# Пакетная конвертация всех HEIC в папке
convert *.heic

# Рекурсивная конвертация всех HEIC в папке и подпапках
convert --recursive .

# Сохранение в другую папку
convert image.heic --output ./converted

# Перезапись существующих файлов
convert image.heic --overwrite
🛠️ Особенности реализаций
Python – использует pillow-heif и Pillow, поддержка всех форматов.

Node.js – sharp (libvips) – высокопроизводительная и поддерживает HEIC.

Go – go-heic – обёртка для libheif, простая интеграция.

Rust – heic crate и image crate.

C++ – нативная libheif – максимальная производительность.

Java – heic-imageio (TwelveMonkeys) – поддержка через ImageIO.

C# – SixLabors.ImageSharp с плагином для HEIC.

Ruby – mini_magick (ImageMagick с libheif).

🤝 Вклад
PR и issues приветствуются. Добавляйте поддержку новых форматов, улучшайте производительность, расширяйте функциональность.

📄 Лицензия
MIT License.
