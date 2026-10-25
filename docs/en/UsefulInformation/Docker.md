# Docker

```{figure} @assets@/common/programming/docker/docker-logo.webp
:alt: Docker
:width: 90%
:align: center
```
<br>

## Introduction

Whether you are developing a web application, a backend service, or a complex robotic algorithm, you almost always rely on external dependencies: specific libraries, software packages, and various configuration files.

You can install all these components directly onto your local machine,  but this approach often leads to several critical bottlenecks:

* **Version Conflicts.** For instance, one application might require Python 3.12, while another is only compatible with Python 3.8. Managing these conflicting requirements manually is difficult and risks breaking one project when updating another.

* **Environment Inconsistency.** A program that runs perfectly on your laptop may fail on another computer or production server due to slight differences in the operating system, incompatible library versions, or missing dependencies.

* **Complex Setup Processes.** Setting up a new project can be time-consuming. You may need to install databases, configure servers, and manually define file paths just to get a single application running.
  
* **System Clutter.** Over time, your computer becomes bloated with unnecessary libraries, packages, and background services left over from abandoned projects.

To overcome these challenges, developers use containerization. One of the industry-standard tools for this is [Docker](https://www.docker.com/).

## Operating Principle

**Docker** allows you to package an application along with every single dependency it needs into a single, isolated unit called a **container**. Instead of installing components directly into your computer's operating system, you run the application within its own dedicated environment.

A container typically includes:
* Software components (e.g., Python, ROS 2, or OpenCV).
* Configuration files: pre-set environments for both your program and its dependencies. 
* Runtime tools: all the libraries and components necessary for the software to execute.
* The application itself.

To avoid manually configuring a container every time you want to run it, **Docker** uses `images`.
An **image** — serves as a read-only template used to create containers. It contains the file system, software components, and launch settings required for your application. You can create a single image and use it to launch multiple containers.

You can save an image and share it with a teammate or upload it to a cloud registry. When the recipient downloads the image, they run the application in a perfectly prepared environment, virtually eliminating errors caused by differing computer settings.

While Docker uses containers, it is not a full-scale Virtual Machine (VM). Unlike a VM, Docker does not require you to run a complete second operating system inside your own. Instead, it isolates the container's processes and resources at the system level. This makes Docker containers significantly faster and much more resource-efficient than traditional virtual machines.

Docker simplifies dependency management, ensures environment reproducibility, and streamlines deployment by separating your application's environment from the host operating system.

## Installation

```{warning}
Only install Docker on your computer if you intend to create and manage your own containers.
The `clover2` image already includes Docker. If you are working with the quadcopter using the pre-configured environment, you can skip this entire section
```

To proceed with the installation, ensure you have a stable internet connection and administrative privileges on your machine.

### Ubuntu
For detailed information, refer to the [Official Docker Engine for Ubuntu documentation](https://docs.docker.com/engine/install/ubuntu/).

Follow these steps in your terminal:

1. Update package lists and install `curl`:

``` bash
sudo apt update
sudo apt install curl
```

* `sudo apt update` refreshes your local package index from the configured repositories. 
* `sudo apt install curl` installs the  `curl` utility, used to transfer data via network protocols.

2. Run the official installation script:

``` bash
curl -fsSL https://get.docker.com | sudo sh
```

This command downloads the official Docker installation script and pipes it directly into the shell to execute the setup with administrative rights.

3. Add your user to the docker:

``` bash
sudo usermod -aG docker $USER
```

* `usermod` modifies user account settings. 
* `-aG` appends the user to the specified group.
* `$USER` automatically inserts your current username.

Apply the group changes:

``` bash
newgrp docker
```

### Windows
For detailed information, refer to the [Official Docker Desktop for Windows documentation](https://docs.docker.com/desktop/setup/install/windows-install/). 

To run Linux containers on Windows, Docker Desktop requires `WSL 2` (Windows Subsystem for Linux).  This is also highly recommended for working with `ROS 2`.

1. Open PowerShell as an Administrator and run: `wsl --install`. Follow the on-screen prompts and restart your computer if requested.
Open the `Microsoft Store`  and search for `Ubuntu 24.04`.

```{figure} @assets@/common/programming/docker/ms-ubuntu-install.webp
:alt: ms-ubuntu-install
:width: 90%
:align: center
Installing Ubuntu 24.04
```

<br>

2. Click `Get` and wait for the installation to finish (the button will change to `Open`). 
Restart your computer to finalize the installation.

```{figure} @assets@/common/programming/docker/ms-ubuntu-open.webp
:alt: ms-ubuntu-open
:width: 90%
:align: center
Opening Ubuntu 24.04
```

<br>

3. Launch `Ubuntu 24.04` from the `Start` menu. On the first launch, you will be prompted to create a Linux username and a strong password. Remember this password; you will need it to execute  `sudo` (administrative) commands. Once setup is complete, you will see a command prompt ready for Linux commands.

```{figure} @assets@/common/programming/docker/wsl-setup-user.webp
:alt: wsl-setup-user
:width: 90%
:align: center
Creating user in Ubuntu
```

<br>

4. Get the Docker Desktop installer from [Official Docker website](https://docs.docker.com/desktop/release-notes/).

```{figure} @assets@/common/programming/docker/docker-windows-download.webp
:alt: docker-windows-download
:width: 90%
:align: center
Downloading Docker Desktop for Windows
```

<br>

5. Run the downloaded `.exe` file.
If the installer presents configuration options, follow the recommended settings (see figure below).

```{figure} @assets@/common/programming/docker/win-docker-1-step.webp
:alt: win-docker-1-step
:width: 90%
:align: center
Configuring Docker Desktop
```
<br>

6.  Once the installation is complete, save your work and restart your computer if prompted. Upon the first launch, review and accept the Docker Service Agreement.

```{figure} @assets@/common/programming/docker/docker-first-run.webp
:alt: docker-first-run
:width: 90%
:align: center
Docker Desktop first launch
```
<br>

7. Open the `Ubuntu 24.04` terminal via the `Start` menu. 
Grant your Linux user permission to interact with Docker by running:

``` bash
sudo usermod -aG docker $USER
```

Refresh your current terminal session to apply the changes:

``` bash
newgrp docker
```

## Running Your First Container

In Docker, an `image` is the static, prepared environment, while a `container` is the active, running instance of that environment.

 Let’s launch a container based on the Ubuntu image. Open your terminal and run:

``` bash
docker run -it --rm ubuntu
```

Breakdown of the command:

* `docker run` tells Docker to create and start a new container from a specific image.
* `-it` allows you to interact with the container via your keyboard.
* `--rm` automatically removes the container once it stops running, keeping your system clean.
* `ubuntu` — the name of the image to be used. If you don't have this image locally, Docker will automatically download it from the official registry.

Once the command runs, Docker will drop you directly into the container's command line. Your prompt will change to look something like this:

``` bash
$ docker run -it --rm ubuntu
Unable to find image 'ubuntu:latest' locally
latest: Pulling from library/ubuntu
Digest: sha256:d1e2e92c075e5ca139d51a140fff46f84315c0fdce203eab2807c7e495eff4f9
Status: Downloaded newer image for ubuntu:latest
root@d994718f32d0:/# 
```

The `root@...:/#` prompt indicates that you are now operating inside the container as the `root` user (the administrative user for that specific isolated environment).

To demonstrate how Docker isolates processes from your main operating system, let’s install and run `htop` — a tool used to monitor running processes and resource usage.

Update the container's package list:

``` bash
apt update
```

`apt update` refreshes the list of available software within the container's isolated environment.

Install `htop`:
``` bash
apt install htop -y
```

The `-y` flag automatically confirms the installation prompt.

Launch `htop`:
``` bash
htop
```

An interactive interface will open.

```{figure} @assets@/common/programming/docker/htop.webp
:alt: htop
:width: 90%
:align: center
```

<br>

The process list only shows what is happening inside the container (such as `bash` and `htop` itself), 

To leave the container, simply type `exit`.

Because we used the `--rm` flag when we started the container, Docker has already automatically deleted the container.

You can verify that the container has been removed by checking your container history:

``` bash
docker ps -a
```

* `docker ps` displays currently running containers.
* `-a` includes containers that have already exited or stopped.

In the output, you'll get a table roughly like this:

``` bash
$ docker ps -a
CONTAINER ID   IMAGE          COMMAND                  CREATED          STATUS                    PORTS     NAMES
```

Because of the `--rm` flag, your Ubuntu container will not appear in this list, confirming that Docker successfully cleaned up the environment after use.
