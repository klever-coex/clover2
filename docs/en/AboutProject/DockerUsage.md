# Docker in the clover2 Project

The clover2 project leverages Docker to ensure all services run in a reproducible, consistent environment. 
The CI builds images using the exact same Dockerfiles available in the local repository. 
Furthermore, the final SD card image for the Raspberry Pi utilizes these containers to manage core services.

## Images

All Docker configuration files can be found in the `docker/` directory.

| Image | Dockerfile | Purpose |
| --- | --- | --- |
| `clover2-ros` | `docker/ros/Dockerfile` | ROS 2 Jazzy + clover2 packages |
| `clover2-frontend` | `docker/frontend/Dockerfile` | Web interface |
| `clover2-docs` | `docker/docs/Dockerfile` | Documentation |
| `px4` | `docker/px4/Dockerfile` | PX4 build |

Images are published to the **GitHub Container Registry** (GHCR) at: `ghcr.io/klever-coex/clover2/<имя>:<tag>`.

Available tags:

- `<git-hash>`: the specific commit hash used for the build.
- `latest`: the most recent image from the master branch.
- `stable` & `<version>`: official images designated for releases and pre-releases.

```{note}
Images are built during every CI run. They are automatically published to GHCR when the build is triggered for the master branch, a release, or a pre-release.
```  

## Building Images

We use [docker buildx bake](https://docs.docker.com/build/bake/) for building images. The build configurations are managed via the `docker/docker-bake.hcl` file. 

You can use the following `make` commands for convenience:

```bash
make clover2-bake-ros        # Build the clover2-ros image
make clover2-bake-frontend   # Build clover2-frontend image
make clover2-bake-push-ros   # Build and push the clover2-ros image to GHCR
```

By default, the build targets your host platform. To build specifically for ARM64 architecture, run: `TARGET_ARCH=arm64 make clover2-bake-ros`.

## Running Services with Docker Compose

The `docker/compose.yaml` file defines how to run services both locally and on the Raspberry Pi. 
This is ideal for debugging the web interface or running the full software stack on the device. 

Services are organized into profiles:

| Profile | Services | Purpose |
| --- | --- | --- |
| `web-test` | `frontend`, `docs`, `ros-web`, `ros-map` | Debugging the web interface |
| `robot` | `frontend`, `docs`, `ros-web`, `ros` | Running the full stack (including `klever5_launcher.py`) |
| `docs` | `docs` | Viewing the documentation |

To start services, specify your desired profile using the `--profile` flag.

```bash
# Debug the web interface:
docker compose -f docker/compose.yaml --profile web-test up

# Run a specific image version:
TAG=v0.1.9 docker compose -f docker/compose.yaml --profile web-test up

# Run a specific commit hash:
TAG=cbd9452 docker compose -f docker/compose.yaml --profile web-test up

# Run the full robot stack:
docker compose -f docker/compose.yaml --profile robot up
```

```{note}
If no image tag is specified, Docker Compose defaults to `latest`.
Since `latest` is assigned to every `master` branch build, you will always pull the most recent version available for that branch.
```

ROS containers utilize the `ROS_DOMAIN_ID` environment variable to define the data exchange domain between ROS 2 nodes.

## ROS Image Specifics

The `clover2-ros` image follows the standard ROS 2 container pattern using `ENTRYPOINT` and `CMD`.

The **ENTRYPOINT**  automatically configures the necessary ROS 2 environment variables. 

You can interact with it via:

```bash
# Open an interactive bash shell
docker run -it ghcr.io/klever-coex/clover2/clover2-ros:latest bash

# Run a direct ROS 2 command
docker run ghcr.io/klever-coex/clover2/clover2-ros:latest ros2 topic list
```

The **CMD** is configured by default to launch the full system: `ros2 launch clover2_bringup klever5.launch.xml`

## Running as an Unprivileged User

To prevent permission issues (where files created inside a container are owned by root on your host machine), you should pass your host user's ID and group information when starting the container:

```bash
docker run -it \
    -e LOCAL_UID=$(id -u) -e LOCAL_USER=$(id -un) \
    -e LOCAL_GID=$(id -g) -e LOCAL_GROUP=$(id -gn) \
    ghcr.io/klever-coex/clover2/clover2-ros:latest bash
```

Variable definitions:
* `LOCAL_UID`: Your User ID.
* `LOCAL_USER`: Your Username.
* `LOCAL_GID`: Your Group ID.
* `LOCAL_GROUP`: Your Group Name.

The application of these variables is handled automatically by the image's `entrypoint` configuration.
