# Simulator Usage Guide

First, prepare the environment by following the {doc}`Installation Guide <Installation>`. 
All commands provided in this guide should be executed within the **Dev Container** terminal or the guest **Linux** system running via VMware/UTM. In the pre-configured images, your project workspace is located at  `~/clover2-dev`.

(simulator-launch)=

## How to Launch the Simulator

:::{important}
In VMware/UTM, the environment is pre-configured; you can launch the simulator immediately.
In Dev Container, if it is the first time you use it, complete the {doc}`dependency download and project build <Installation/DevContainer>`. Once complete, launch the simulator from the container terminal with the workspace attached.
:::

Open a terminal within your guest Linux system or Dev Container. Run the following command:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

```{figure} @assets@/common/simulator/vmware/launch-simulator.webp
:alt: Simulator launch
:width: 90%
:align: center
Simulator launch
```

<br>

Once the command is executed, a **Gazebo** graphical window will open displaying the virtual quadcopter model, and ROS 2 messages will begin streaming in your terminal. Do not close the terminal while the simulator is active. To safely shut down the simulator, return to that terminal and press `Ctrl+C`.

:::{tip}
For optimal performance, restart the simulator if it has been inactive for an extended period.
:::

```{figure} @assets@/common/simulator/vmware/gazebo-running.webp
:alt: Gazebo
:width: 90%
:align: center
Gazebo simulator
```

<br>

## How to Program in the Simulator

The simulator provides a robust environment to develop and test control code for your virtual quadcopter. Because the simulator mirrors real-world operations, you will use the exact same [framework](../Programming/AutonomousFlight) required for a physical quadcopter.

To streamline your development workflow, VS Code is pre-installed within the VMware/UTM environments.

## How to Restart the Simulator

If you restart your computer, you do not need to reinstall the environment or rebuild the project, provided that your source code and dependencies remain unchanged.

To resume your work, follow these steps:

1. Start your virtual machine in VMware/UTM. For Dev Container, open the `clover2-dev` folder in VS Code and select `Dev Containers: Reopen in Container`.
2. Open a terminal inside the active virtual machine or Dev Container.
3. Execute the launch command:
   ```bash
   ros2 launch clover2_sim gz_simple.launch.py
   ```

### When to Rebuild the Project

To ensure your environment remains stable, rebuild the project if any of the following conditions apply:

- You are downloading the project source code for the first time.
- You have moved to C++ code.
- You have updated `CMakeLists.txt` or `package.xml` files.
- You have added new ROS 2 packages.
- The launch command returns an error stating it cannot find a required package.
- You have updated repositories using `vcs import`.

To rebuild the project, run the following commands:

```bash
cd ~/clover2-dev
colcon build --symlink-install
```

Wait for the build process to finish. If errors appear in the terminal, resolve them before proceeding. Once the build is successful, launch the simulator using:

   ```bash
   ros2 launch clover2_sim gz_simple.launch.py
   ```

## QGroundControl

Just like a physical quadcopter, you can use QGroundControl to manage flight controller parameters and monitor real-time telemetry.

In VMware/UTM, QGroundControl is pre-installed. You can find and launch it directly from the applications menu.

For Dev Container, install QGroundControl on your host operating system (your main computer) and run it as a separate application from the container.

The connection to the simulator is established automatically upon launch. If you are unable to connect, verify that the `Autoconnect to UDP` parameter is enabled in the QGroundControl settings.


## Render Engine

**Gazebo** utilizes various rendering engines to handle the visual elements of the virtual world, including objects, lighting, and textures.

In VMware/UTM, these images use the `ogre1` engine, which was selected to ensure maximum compatibility and stability within a virtual machine environment.

In Dev Container, the `ogre2` engine is used by default. While it offers more modern graphical effects, it may cause compatibility issues with certain graphics cards.

If you encounter graphical glitches or display problems in Gazebo, try to switch to the `ogre1` engine using one of the following methods:

Method 1: Environment Variable

Set the engine via an environment variable before launching the simulator:

```bash
export CLOVER2_GZ_SIM_RENDER_ENGINE=ogre
ros2 launch clover2_sim gz_simple.launch.py
```

To make this change permanent so that it applies automatically every time you open a terminal, add the export command to your `~/.bashrc` file:

```bash
export CLOVER2_GZ_SIM_RENDER_ENGINE=ogre
```

Method 2: Launch Argument

You can specify the engine directly within the launch command:

```bash
ros2 launch clover2_sim gz_simple.launch.py render_engine:=ogre
```

If display issues persist after trying these methods, review the terminal output for error messages and check your virtual machine graphics settings.
