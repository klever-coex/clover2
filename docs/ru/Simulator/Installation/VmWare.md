# VMware Workstation

## Установка VMware

Скачайте [установщик VMware Workstation Pro для Windows](https://downloads.klevercoex.ru/vmware_exe/VMware-Workstation-Full-26H1u1-25688693.exe), запустите файл `.exe` и завершите установку по подсказкам мастера. Перезагрузите компьютер, если установщик предложит это сделать.

<!-- Инструкция Broadcom сохранена для перевода на английский.

Скачайте актуальный **VMware Workstation Pro для Windows** из [официального каталога Broadcom](https://support.broadcom.com/group/ecx/productdownloads?freeDownloads=true&subfamily=VMware+Workstation+Pro). Ссылка ведёт к каталогу версий, чтобы можно было выбрать свежий выпуск.

1. Войдите в учётную запись Broadcom или зарегистрируйтесь.
2. Выберите **VMware Workstation Pro for Windows** и последнюю доступную стабильную версию.
3. Примите условия загрузки и, если портал запросит, заполните сведения профиля.
4. Скачайте установщик `.exe`, запустите его и завершите установку по подсказкам мастера. Перезагрузите компьютер, если установщик предложит это сделать.

VMware Workstation Pro бесплатен для личного, учебного и коммерческого использования; лицензионный ключ не требуется. Подробности доступны в [инструкции Broadcom по загрузке и лицензированию](https://knowledge.broadcom.com/external/article/368667/download-and-license-information-for-vmw.html).

-->

## Импорт OVA в Workstation

1. Откройте VMware Workstation Pro.
2. Выберите **File → Open**, **Open a Virtual Machine** или нажмите `Ctrl+O`.

   ```{figure} @assets@/common/simulator/vmware/open-virtual-machine.webp
   :alt: Открытие образа виртуальной машины
   :width: 90%
   :align: center
   Открытие образа виртуальной машины
   ```

   <br>

3. Укажите подготовленный файл `.ova` и нажмите **Open**. Сам файл OVA распаковывать не нужно.
4. Задайте имя виртуальной машины и папку для её файлов. На диске должно хватать места для импортируемых виртуальных дисков.
5. Нажмите **Import**, примите лицензионное соглашение образа, если оно появится, и дождитесь окончания импорта.

Если импорт остановился с сообщением о несоответствии спецификации OVF или виртуального оборудования, используйте **Retry** для повторной попытки с ослабленной проверкой совместимости.

## Настройка виртуальной машины

1. Выберите импортированную машину и откройте **Edit virtual machine settings**.

   ```{figure} @assets@/common/simulator/vmware/virtual-machine-settings.webp
   :alt: Настройки импортированной виртуальной машины
   :width: 90%
   :align: center
   Настройки импортированной виртуальной машины
   ```

   <br>

2. В разделе **Processors → Virtualization Engine** выключите **Virtualize Intel VT-x/EPT or AMD-V/RVI**.

   ```{figure} @assets@/common/simulator/vmware/processor-virtualization.webp
   :alt: Выключение вложенной виртуализации в настройках процессора VMware
   :width: 90%
   :align: center
   Выключение Virtualize Intel VT-x/EPT or AMD-V/RVI
   ```

   <br>

3. В разделе **Display** увеличте объем видеопамяти минимум до 1гб.

   ```{figure} @assets@/common/simulator/vmware/display-acceleration.webp
   :alt: Увеличение объема видеопамяти
   :width: 90%
   :align: center
   Увеличение объема видеопамяти
   ```

   <br>

## Запуск машины

1. Нажмите **Start up this guest operating system**.

   ```{figure} @assets@/common/simulator/vmware/start-virtual-machine.webp
   :alt: Запуск виртуальной машины
   :width: 90%
   :align: center
   Запуск виртуальной машины
   ```

   <br>

2. После загрузки войдите в Linux с учётными данными из описания образа.
3. Перейдите к {ref}`общей инструкции запуска симулятора <simulator-launch>`.
