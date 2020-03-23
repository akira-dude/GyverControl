/*
  Скетч к проекту "Универсальный контроллер"
  Страница проекта (схемы, описания): https://alexgyver.ru/gyvercontrol/
  Исходники на GitHub: https://github.com/AlexGyver/gyvercontrol
  Нравится, как написан и закомментирован код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2020
  http://AlexGyver.ru/
*/

/*
  ===== Версия 1.5 =====
  - Облегчённые библиотеки:
    - Для часов используется библиотека microDS3231
    - Для дисплея используется библиотека microLiquidCrystal_I2C
    - Для ds18b20 используется библиотека microDS18B20
    - Для BME280 используется библиотека GyverBME280
  - Чуть оптимизации кода
  - Исправлен баг с выводом инверсных состояний реле в окне DEBUG
  - Добавлена возможность работы с отрицательными температурами в режиме Sensor
  - Исправлено незапоминание настроек SP и PP в Service
  - ServoSmooth обновлена, работа серво улучшена
  - Исправлен баг с сохранением настроек
    - Ваши настройки при переходе на 1.5 будут сброшены!
  - В ПИД установка переделана на десятичные дроби
  - BME и Dallas выводят температуру в десятичных дробях
  - Шаги настроек изменены на более мелкие
  - Исправлена настройка времени в сервисе
  - Добавлен режим ПИД для каналов 1 и 2 (низкочастотный ШИМ). Каналы помечены *
  - Для "обратного" режима ПИД нужно ставить отрицательные коэффициенты!
  - В недельке можно выбрать время включения меньше времени выключения
  - Ещё оптимизация памяти
  - Добавлен "быстрый поворот" энкодера: шаг изменения значения увеличивается при быстром вращении
  - Чуть оптимизирован ПИД
  - Значения с точкой в графиках
  - Автоматический масштаб графика
  - Переделана структура меню настроек
  - Добавлена настройка даты в меню настроек
  - Воскресенье теперь цифра 7 (было 0)
  - Исправлена настройка времени в Timer RTC
  - Корректное отображение и работа каналов Servo, работающих как реле
  - Улучшена работа ПИД
  - Добавлены названия для доп. датчиков, улучшено оформление
  - Период опроса и период графиков перенесены в настройки
  - ЕЩЁ БОЛЬШЕ ОПТИМИЗАЦИИ
  - Добавлена поддержка датчика угл. газа MH-Z19. Есть возможность отключить автокалибровку.
  - Исправлена ошибка в графиках
  - ПИД и РАССВЕТ переделаны в линейное меню
  - Оптимизированы графики
  - Добавлена возможность отключить плавность серво для облегчения памяти
  - Оптимизация памяти и вывода на дисплей
  - Исправлен косяк с приводом при выходе из сервиса
  - Небольшие изменения в окне сервиса
  - Добавлено стартовое меню с сервисом и сбросом (включается по желанию)
  - Небольшая оптимизация памяти
  - Исправлено отображение реле с низким уровнем в сервисе
  - Добавлено расписание для ПИД. Ежедневное и на период в днях
  - Исправлен косяк с подсветкой
  - Исправлены критические ошибки с серво (пид и рассвет)
  - ИСПРАВЛЕНЫ ОШИБКИ С СОХРАНЕНИЕМ НАСТРОЕК
  - Сильная оптимизация оперативной памяти
  - (в разработке) Графики теперь сохраняются за все периоды (неделя, день, час, минута)
*/

// -------------------- НАСТРОЙКИ ---------------------
// ======== УПРАВЛЕНИЕ ========
#define ENCODER_TYPE 1    // тип энкодера (0 или 1). Если энкодер работает некорректно (пропуск шагов/2 шага), смените тип
#define ENC_REVERSE 1     // 1 - инвертировать направление энкодера, 0 - нет
#define CONTROL_TYPE 1    // тип управления энкодером:
// 0 - удерживание и поворот для изменения значения
// 1 - клик для входа в изменение, повторный клик для выхода (стрелочка меняется на галочку)
#define FAST_TURN_STEP 10 // изменение при быстром повороте
#define FAST_TURN 1       // 1 - вкл быстрый поворот

// ========= РАЗНОЕ ==========
#define DRIVER_LEVEL 1    // 1 или 0 - уровень сигнала на драйвер/реле для привода
#define PWM_RELAY_HZ 1    // частота ШИМ для каналов ШИМ-реле, Гц (раз в секунду) можно десятичные дроби (0.1 - период будет 10 секунд)

// ====== НАЗВАНИЯ КАНАЛОВ ====== (только английские буквы)
const char *channelNames[] = {
  "Channel 1",
  "Channel 2",
  "Channel 3",
  "Channel 4",
  "Channel 5",
  "Channel 6",
  "Channel 7",
  "Servo 1",
  "Servo 2",
  "Drive",
};

// =========== СИСТЕМА ==========
#define SCHEDULE_NUM 0      // меню расписания, задать количество расписаний (не больше 3!). 0 - выключить
#define START_MENU 0        // 1 - включить, 0 - отключить стартовое меню (старт с нажатой кнопкой). Там будет сервис и сброс настроек
#define WDT_ENABLE 0        // 1 - включить, 0 - отключить watchdog (только для optiboot)
#define USE_PLOTS 0         // 1 - включить, 0 - отключить вывод графиков
#define USE_PID 0           // включает/отключает поддержку ПИД регулятора на каналах 0 и 1 (ШИМ-реле), 2, 3, серво и привода
#define USE_PID_RELAY 0     // включает/отключает поддержку ПИД регулятора на каналах 0 и 1 (ШИМ-реле). Требует USE_PID для работы!
#define USE_DAWN 0          // включает/отключает поддержку режима РАССВЕТ на каналах 2, 3, серво
#define USE_DRIVE 0         // включает/отключает поддержку линейного привода (в целях экономии памяти)

#define SERVO1_RELAY 1      // 1 - заменить серво 1 на реле (ЭКОНОМИТ ПАМЯТЬ). 0 - ничего не делать
#define SERVO2_RELAY 1      // 1 - заменить серво 2 на реле (ЭКОНОМИТ ПАМЯТЬ). 0 - ничего не делать
#define SMOOTH_SERVO 0      // 1 - использовать плавное управление серво, 0 - обычное (ЭКОНОМИТ ПАМЯТЬ)
#define SERVO_MIN_PULSE 600 // минимальный импульс серво (зависит от модели, 500-800)
#define SERVO_MAX_PULSE 2400// максимальный импульс серво (зависит от модели, 2000-2500)
// ОТКЛЮЧЕНИЕ ОБЕИХ СЕРВО ЭКОНОМИТ МНОГО ПАМЯТИ!

#define SETT_TIMEOUT 100    // таймаут неактивности (секунд) после которого автоматически откроется DEBUG и сохранятся настройки
#define LCD_ADDR 0x3f       // адрес дисплея - 0x27 или 0x3f . Смени если не работает!!
#define BME_ADDR 0x76       // адрес BME280 - 0x76 или 0x77. Смени если не работает!! (добавлено в v1.1.1)

// Ссылка для оптибут
// https://github.com/Optiboot/optiboot/releases/download/v8.0/package_optiboot_optiboot-additional_index.json

// =========== ДАТЧИКИ ==========
// датчик СО2 MH-Z19
#define USE_CO2 0           // 1 - использовать MH-Z19, 0 - не использовать
#define CO2_PIN 1           // 1 или 2, соответственно подключать к SENS_1 или SENS_2 !!!
#define CO2_MAX 5000        // диапазон датчика (бывает 2000 и 5000)
#define CO2_CALIB 0         // 1 - оставить автокалибровку датчика (для жилых комнат), 0 - выключить (для теплиц)

// цифровой датчик температуры и влажности bme280 (шина i2c)
#define USE_BME 0           // 1 - использовать BME280, 0 - не использовать

// цифровой датчик температуры ds18b20 (вход SENS1)
#define DALLAS_SENS1 0      // 1 - ко входу SENS1 подключен ds18b20, 0 - подключен обычный аналоговый датчик

// цифровой датчик температуры и влажности DHT11/DHT22 (вход SENS2) - вместо BME280
#define DHT_SENS2 0         // 1 - ко входу SENS2 подключен DHT11/DHT22, 0 - подключен обычный аналоговый датчик
#define DHT_TYPE DHT22      // тип DHT датчика: DHT11 или DHT22

// термисторы
#define THERM1 0            // 1 - ко входу SENS1 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF1 3435     // температурный коэффициент термистора 1 (см. даташит)

#define THERM2 0            // 1 - ко входу SENS2 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF2 3435     // температурный коэффициент термистора 2 (см. даташит)

#define THERM3 0            // 1 - ко входу SENS3 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF3 3435     // температурный коэффициент термистора 3 (см. даташит)

#define THERM4 0            // 1 - ко входу SENS4 подключен термистор, 0 - подключен другой аналоговый датчик
#define BETA_COEF4 3435     // температурный коэффициент термистора 4 (см. даташит)

#include "a1_data.h"
