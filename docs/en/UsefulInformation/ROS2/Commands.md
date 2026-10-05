# ROS 2 Commands

```{figure} @assets@/common/programming/ros2/terminal-linux-commands.webp
:alt: Рисунок 1 — Пример команд Linux в терминале
:width: 90%
:align: center

Linux terminal commands
```
<br>

## Introduction

When working with ROS 2, the terminal is your primary interface. It is essential for launching nodes, inspecting topic lists, monitoring sensor data, configuring parameters, executing programs, and debugging errors.

While the terminal does not replace a code editor, it is indispensable for gaining real-time insights into your system. If your quadcopter stops receiving sensor data, a node fails to launch, or a required topic is missing, the terminal is your first line of defense.

This guide provides a collection of essential Linux and ROS 2 commands frequently used during development and debugging.

```{contents}
:depth: 2
:local:
:class: custom-toc
```

## Basic Linux Commands

Linux commands are used to navigate directories, manage files, launch programs, and monitor system processes.

### Navigating Directories

| Command | Description |
| --- | --- |
| `pwd` | Displays the current working directory path |
| `ls` | Lists files and folders in the current directory |
| `ls -la` | Lists all files (including hidden ones) with detailed permissions and sizes |
| `cd ~/clover2_ws` | Changes the directory to `~/clover2_ws` |
| `cd ..` | Moves up one directory level |

### Working with Files and Folders

| Command | Description |
| --- | --- |
| `mkdir test_folder` | Creates a new folder named  `test_folder` |
| `touch notes.txt` | Creates an empty file named `notes.txt` or updates the timestamp of an existing one|
| `cp notes.txt notes_copy.txt` | Copies `notes.txt` to a new file named `notes_copy.txt` |
| `mv notes_copy.txt old_notes.txt` | Renames or moves a file |
| `rm old_notes.txt` | Deletes the file `old_notes.txt` |

```{warning}
The `rm` command deletes files permanently; they are not moved to a trash bin. Always double-check your file path and name before pressing enter. Use `rm -r` with extreme caution, as it deletes a folder and all of its contents recursivel.
```

### Viewing and Searching

| Command | Description |
| --- | --- |
| `cat notes.txt` | Displays the entire content of `notes.txt` in the terminal |
| `less notes.txt` | Opens `notes.txt` in a scrollable viewer |
| `grep "error" log.txt` | Searches for the string `error` within `log.txt` |
| `find . -name "*.py"` | Searches for all Python files in the current directory and its subdirectories |

In the command `find . -name "*.py"` the dot (.) represents the current directory. The `-name` parameter specifies the search criteria, and the pattern `"*.py"` matches any filename ending in  `.py`.

To exit `less` viewer, press `q`.

### Installing Packages

| Command | Description |
| --- | --- |
| `sudo apt update` | Updates the local list of available packages from the repositories |
| `sudo apt upgrade` | Upgrades all currently installed packages to their latest versions |
| `sudo apt install <package>` | Installs a specific package |

```{warning}
`sudo` executes commands with administrative (root) privileges. Always ensure you fully understand a command before running it with `sudo` to avoid unintended system changes. 
```

### Managing Processes

| Command | Description |
| --- | --- |
| `ps aux \| grep ros2` | Lists all running processes and filters for those containing `ros2` |
| `top` | Displays real-time system processes and resource usage |
| `htop` | An interactive, more user-friendly version of `top` |
| `kill 12345` | Terminates the process with the ID (PID) `12345` |

## Basic ROS 2 Commands

The `ros2` command is organized into subcommands. For example, `ros2 topic list` is used for interacting with topics, while `ros2 node` is used for managing nodes.

```{figure} @assets@/common/programming/ros2/terminal-ros2-topic.webp
:alt: Рисунок 2 — Пример команд ros2 node и ros2 topic
:width: 90%
:align: center

Example of `ros2 node` and `ros2 topic` commands
```

### Help

| Command | Description |
| --- | --- |
| `ros2 --help` | Displays general help for the `ros2` command |
| `ros2 topic --help` | Displays help for topic-related subcommands |
| `ros2 topic echo --help` | Displays help for the `echo` subcommand and its parameters |

The `--help` flag is invaluable if you forget specific syntax or need to view available parameters. You can append `--help` to almost any ROS 2 command to access its specific documentation.

### Nodes

| Command | Description |
| --- | --- |
| `ros2 node list` | Lists all currently active nodes |
| `ros2 node info /camera_node` | Displays the topics, services, and parameters associated with the `/camera_node` |

If a node does not appear in the list, it has not been discovered by the system. This typically indicates that the node is not running or is not communicating on the same `ROS_DOMAIN_ID`.

### Topics

| Command | Description |
| --- | --- |
| `ros2 topic list` | Lists all available topics in the system |
| `ros2 topic info /scan` | Displays the message type and the number of publishers and subscribers for the `/scan` topic |
| `ros2 topic echo --once /scan` | Prints a single message from the `/scan` topic and then exits |
| `ros2 topic hz /scan` | Displays the current frequency (rate) at which messages are being published on the `/scan` topic |

To print only a specific field from a message, use:

| Command | Description |
| --- | --- |
| `ros2 topic echo --once /scan --field ranges` | Prints only the `ranges` field from a single message on the `/scan` topic |

### Services

| Command | Description |
| --- | --- |
| `ros2 service list` | Lists all available services |
| `ros2 service type /reset` | Displays the service type for  `/reset` |
| `ros2 service call /reset std_srvs/srv/Empty "{}"` | Sends a request to the `/reset` service using the `std_srvs/srv/Empty` interface |

Services are used to trigger specific actions via a request-response mechanism: a client sends a request, and the service provides a response. For example, a service might be used to reset a device's state. Note that specific quadcopter controls, such as arming or disarming, will depend on the specific packages and interfaces installed on your system.

### Parameters

| Command | Description |
| --- | --- |
| `ros2 param list` | Lists all parameters available to the running nodes |
| `ros2 param get /camera_node exposure` | Retrieves the current value of the `exposure` parameter for the `/camera_node` |
| `ros2 param set /camera_node exposure 100` | Sets the `exposure` parameter of the `/camera_node` to `100` |

Parameters can be used to adjust the publishing rate, switch a device's operating mode, or modify other settings defined by the node developer.

Parameters are scoped to specific nodes. Therefore, the `ros2 param list` command must always be accompanied by a node name to be effective.


### Launching Programs

| Command | Description |
| --- | --- |
| `ros2 pkg executables <package>` | Lists all registered executable files within the specified `<package>` |
| `ros2 run <package> <executable>` | Launches a specific `<executable>` from the `<package>` |
| `ros2 launch clover2 clover2.launch.py` | Executes the `clover2.launch.py` launch file from the `clover2` package |

Files ending in `.launch.py` must be executed using the `ros2 launch` command, rather than `ros2 run`.
If a package or executable cannot be found, first ensure that your environment has been properly sourced. You can then verify the availability of your files using these commands:

| Command | Description |
| --- | --- |
| `ros2 pkg list \| grep clover` |Searches for installed packages containing the string `clover` |
| `ros2 pkg executables clover2` | Lists all executable files available in the `clover2` package |

While `ros2 run` is convenient for launching a single node, most real-world projects utilize `ros2 launch`. This is because complex systems typically require multiple nodes to be launched and configured simultaneously.

### Recording Data 

| Command | Description |
| --- | --- |
| `ros2 bag record /scan` | Records messages from the `/scan` topic into a bag file |
| `ros2 bag record -a` | Records messages from all currently available topics |
| `ros2 bag play rosbag2_2026_05_22-12_00_00` | Replays the recorded data from the specified bag file |

Recording data into bag files is essential when you need to save sensor data to replay later, allowing you to test algorithms without a physical robot or live sensors.

## Practical Debugging Scenario

If you are expecting data on the `/scan` topic but are not receiving it, follow this systematic troubleshooting workflow:

1. Verify the required node is running:

   | Command | Description |
   | --- | --- |
   | `ros2 node list` | Lists all active nodes |

2. Locate the topic: 

   | Command | Description |
   | --- | --- |
   | `ros2 topic list \| grep scan` | Filters the topic list to find any topic containing `scan` |

3. Inspect the topic details: 

   | Command | Description |
   | --- | --- |
   | `ros2 topic info /scan` | Checks the message type and verifies if there are active publishers and subscribers |

4. Test the data stream: 

   | Command | Description |
   | --- | --- |
   | `ros2 topic echo --once /scan` | Attempts to print a single message to verify data is actually flowing |
   | `ros2 topic hz /scan` | Monitors the frequency of incoming messages to check for stability |

* *If the topic is missing from the list*. Check if the node responsible for that topic is actually running.
* *If the topic exists but no messages appear*. Check the physical device connection, verify the driver's parameters, and inspect the terminal where the node was launched for error logs.
