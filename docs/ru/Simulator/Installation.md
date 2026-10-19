# Установка

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

Installation/DevContainer
Installation/VmWare
Installation/UTM
```

Виртуальная машина позволяет запустить Linux со средой симуляции внутри другой операционной системы, например Windows или macOS.

```{important}
Для компьютеров с ОС Linux можно использовать VMware Workstation Pro и `.ova` образ, но этот вариант не тестировался. Для Linux рекомендуется использовать [Dev Container](./VirtualMachine/DevContainer).
```

## Выбор программы и формата образа

Для запуска готового образа выберите программу и формат образа в зависимости от компьютера.

Для Linux используйте [Dev Container](./VirtualMachine/DevContainer) — среду разработки внутри Docker.

Для Windows с процессором Intel или AMD (x86_64) используйте [VMware Workstation Pro](./VirtualMachine/VmWare) и образ в формате OVA.

Для Mac с процессором Apple Silicon (ARM64, серия M) используйте [UTM](./VirtualMachine/UTM) и образ в формате QCOW2.

## Скачивание образа

TODO: ссылка на latest

После подготовки среды перейдите к разделу {doc}`Использование <Usage>`, где приведены общие команды сборки и запуска симулятора.
