# Docker в проекте clover2

Проект использует Docker для воспроизводимой сборки и запуска: CI собирает образы из тех же Dockerfile, что доступны локально, а готовый образ SD-карты для Raspberry Pi запускает часть этих сервисов в контейнерах.

## Образы

Все что относится к Docker живет в `docker/`:

| Образ | Dockerfile | Назначение |
| --- | --- | --- |
| `clover2-ros` | `docker/ros/Dockerfile` | ROS 2 Jazzy + пакеты clover2 |
| `clover2-frontend` | `docker/frontend/Dockerfile` | Web-интерфейс |
| `clover2-docs` | `docker/docs/Dockerfile` | Документация |
| `px4` | `docker/px4/Dockerfile` | Сборка PX4 |

Образы публикуются в GHCR: `ghcr.io/klever-coex/clover2/<имя>:<тег>`.

Теги:

- `<git-hash>` — на каждый пуш в master (PR-и теги);
- `latest` — последний master;
- `stable`, `<версия>` — релизы.

## Сборка

Сборка идёт через [docker buildx bake](https://docs.docker.com/build/bake/) — конфигурация в `docker/docker-bake.hcl`.

```bash
make clover2-bake-ros        # собрать clover2-ros
make clover2-bake-frontend   # фронтенд
make clover2-bake-push-ros   # собрать и запушить
```

Платформа: `TARGET_ARCH=arm64 make clover2-bake-ros` (по умолчанию — платформа хоста).

## Запуск: docker compose

`docker/compose.yaml` описывает запуск стека локально (отладка web-интерфейса, стенд) и является базой для запуска на роботе. Сервисы сгруппированы в профили:

| Профиль | Сервисы | Сценарий |
| --- | --- | --- |
| `web-test` | `frontend`, `docs`, `ros-web`, `ros-map` | Отладка web-интерфейса |
| `robot` | `frontend`, `docs`, `ros-web`, `ros` | Полный стек: `klever5_launcher.py` + web-поддержка |
| `docs` | `docs` | Только документация |

```bash
# отладка web-интерфейса
docker compose -f docker/compose.yaml --profile web-test up

# конкретная версия образов
TAG=v0.1.9 docker compose -f docker/compose.yaml --profile web-test up

# или по hash коммита
TAG=cbd9452 docker compose -f docker/compose.yaml --profile web-test up

# полный стек
docker compose -f docker/compose.yaml --profile robot up
```

Ros-контейнеры используют `ROS_DOMAIN_ID` из окружения.

## Поведение образов ros

`clover2-ros` использует стандартную для ROS 2 схему entrypoint/CMD:

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
