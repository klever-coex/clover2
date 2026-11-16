# Docker в проекте clover2

Проект использует Docker для сборки и запуска сервисов в воспроизводимой среде. CI собирает образы из тех же Dockerfile, которые доступны локально. Готовый образ SD-карты для Raspberry Pi использует часть этих образов для запуска сервисов в контейнерах.

## Образы

Все файлы, связанные с Docker, находятся в каталоге `docker/`.

| Образ | Dockerfile | Назначение |
| --- | --- | --- |
| `clover2-ros` | `docker/ros/Dockerfile` | ROS 2 Jazzy + пакеты clover2 |
| `clover2-frontend` | `docker/frontend/Dockerfile` | Веб-интерфейс |
| `clover2-docs` | `docker/docs/Dockerfile` | Документация |
| `px4` | `docker/px4/Dockerfile` | Сборка PX4 |

Образы публикуются в **GitHub Container Registry** (GHCR) по адресу: `ghcr.io/klever-coex/clover2/<имя>:<тег>`.

Используются следующие теги:

- `<git-hash>` — хеш коммита, для которого собран образ;
- `latest` — последний образ из ветки `master`;
- `stable` и `<версия>` — образы, опубликованные для релизов и пререлизов.

```{note}
Образы собираются при каждом запуске сборки. В GHCR они публикуются, если сборка выполняется для ветки master, релиза или пререлиза.
```  

## Сборка

Для сборки образов используйте [docker buildx bake](https://docs.docker.com/build/bake/). Конфигурация сборки находится в файле `docker/docker-bake.hcl`. Примеры команд:

```bash
make clover2-bake-ros        # собрать clover2-ros
make clover2-bake-frontend   # фронтенд
make clover2-bake-push-ros   # собрать и отправить образ в GHCR
```

Чтобы собрать образ для архитектуры ARM64, выполните: `TARGET_ARCH=arm64 make clover2-bake-ros` (по умолчанию — платформа хоста).

## Запуск: docker compose

Файл `docker/compose.yaml` описывает запуск сервисов локально и на Raspberry Pi. Его можно использовать для отладки веб-интерфейса и запуска полного стека на устройстве. Сервисы сгруппированы по профилям:

| Профиль | Сервисы | Назначение |
| --- | --- | --- |
| `web-test` | `frontend`, `docs`, `ros-web`, `ros-map` | Отладка веб-интерфейса |
| `robot` | `frontend`, `docs`, `ros-web`, `ros` | Запуск полного стека: `klever5_launcher.py` и сервисов веб-интерфейса |
| `docs` | `docs` | Запуск документации |

Для запуска сервисов укажите нужный профиль с помощью параметра `--profile`.

```bash
# отладка веб-интерфейса:
docker compose -f docker/compose.yaml --profile web-test up

# Запуск определенной версии образов:
TAG=v0.1.9 docker compose -f docker/compose.yaml --profile web-test up

# Вместо версии можно указать хеш коммита:
TAG=cbd9452 docker compose -f docker/compose.yaml --profile web-test up

# Запуск полного стека:
docker compose -f docker/compose.yaml --profile robot up
```

```{note}
Если тег образа не указан, Docker Compose использует `latest`. 
Этот тег присваивается каждой сборке для ветки `master`, поэтому при запуске будет загружен последний образ, собранный для этой ветки.
```

Контейнеры ROS используют переменную окружения `ROS_DOMAIN_ID`, которая определяет домен обмена данными между узлами ROS 2.

## Особенности запуска образа ROS

Образ `clover2-ros` использует стандартную схему запуска контейнера ROS 2 с помощью `ENTRYPOINT` и `CMD`.

- entrypoint полностью настраивает окружение:

```bash
docker run -it ghcr.io/klever-coex/clover2/clover2-ros:latest bash
docker run ghcr.io/klever-coex/clover2/clover2-ros:latest ros2 topic list
```

- CMD по умолчанию запускает полный стек: `ros2 launch clover2_bringup klever5.launch.xml`;

- запуск от непривилегированного пользователя (чтобы можно было редактировать файлы в host системе):

```bash
docker run -it \
    -e LOCAL_UID=$(id -u) -e LOCAL_USER=$(id -un) \
    -e LOCAL_GID=$(id -g) -e LOCAL_GROUP=$(id -gn) \
    ghcr.io/klever-coex/clover2/clover2-ros:latest bash
```
