# VMware Workstation

## Установка VMware

1. Скачайте [установщик VMware Workstation Pro для Windows](https://downloads.klevercoex.ru/vmware_exe/VMware-Workstation-Full-26H1u1-25688693.exe).
2. Запустите файл `.exe` и следуйте инструкциям мастера установки.
3. Если установщик предложит перезагрузить компьютер, сохраните открытые файлы и перезагрузите компьютер.

## Импорт виртуальной машины из файла OVA

1. Откройте VMware Workstation Pro.
2. Выберите `File` → `Open`, `Open a Virtual Machine` или нажмите `Ctrl+O`.

   ```{figure} @assets@/common/simulator/vmware/open-virtual-machine.webp
   :alt: Открытие образа виртуальной машины
   :width: 90%
   :align: center
   Открытие образа виртуальной машины
   ```

   <br>

3. Найдите подготовленный файл `.ova`, выберите его и нажмите `Open`. Сам файл OVA перед импортом распаковывать не нужно.
4. Укажите имя виртуальной машины и папку, в которой будут храниться ее файлы. Убедитесь, что на диске достаточно свободного места для виртуальных дисков.
5. Нажмите `Import`. Если появится лицензионное соглашение, примите его и дождитесь завершения импорта.

Если при импорте появится ошибка о несовместимости спецификации OVA или виртуального оборудования, попробуйте нажать `Retry`. Если программа предложит повторить импорт с менее строгой проверкой совместимости, подтвердите действие.

## Настройка виртуальной машины

1. Выберите импортированную машину и откройте `Edit virtual machine settings`.

   ```{figure} @assets@/common/simulator/vmware/virtual-machine-settings.webp
   :alt: Настройки импортированной виртуальной машины
   :width: 90%
   :align: center
   Настройки импортированной виртуальной машины
   ```

   <br>

2. Откройте раздел `Processors` → `Virtualization Engine` и снимите флжок `Virtualize Intel VT-x/EPT or AMD-V/RVI`.

   ```{figure} @assets@/common/simulator/vmware/processor-virtualization.webp
   :alt: Выключение вложенной виртуализации в настройках процессора VMware
   :width: 90%
   :align: center
   Выключение Virtualize Intel VT-x/EPT or AMD-V/RVI
   ```

   <br>

3. Откройте раздел `Display` и увеличьте объем видеопамяти до 1 ГБ и более.

   ```{figure} @assets@/common/simulator/vmware/display-acceleration.webp
   :alt: Увеличение объема видеопамяти
   :width: 90%
   :align: center
   Увеличение объема видеопамяти
   ```

   <br>

## Запуск виртуальной машины

1. Нажмите `Start up this guest operating system`, чтобы запустить виртуальную машину.

   ```{figure} @assets@/common/simulator/vmware/start-virtual-machine.webp
   :alt: Запуск виртуальной машины
   :width: 90%
   :align: center
   Запуск виртуальной машины
   ```

   <br>

2. Дождитесь загрузки Linux и войдите в систему, используя учетные данные, указанные в описании образа.
3. Выполните шаги из {ref}`общей инструкции запуска симулятора <simulator-launch>`.
