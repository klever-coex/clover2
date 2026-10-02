# How to Configure Your Quadcopter

To configure cameras, maps, and other ROS 2-based components, use the integrated graphical tool ***clover2-settings***.
The tool uses a tree-based structure (similar to a computer's folder system) to organize settings. 

It includes built-in validation to ensure you cannot enter invalid parameter values.

You can navigate the menu using either mouse or keyboard:
-	`Up`/`Down` Arrows: Navigate through the list.
- `Enter`: Select an option or open a menu group.
-	`Esc`: Go back one level or exit the program.
- `Ctrl+S`: Save changes to the configuration file.

## Settings structure 

```text
├── main_camera — Main (bottom) camera settings
│   ├── enable — Enable/Disable the main camera
│   └── feature_detector — Enable/Disable Aruco detection on the main camera
├── front_camera — Front camera settings
│   └── enable — Enable/Disable the front camera
├── optical_flow — Optical Flow settings
│   └── enable — Enable/Disable Optical Flow
├── localization — ArUco tag map localization settings
│   ├── map_server — Map server settings
│   │   ├── enable — Enable/Disable the ArUco map server
│   │   └── map_filename — Map file name
│   └── aruco_tracker — ArUco marker-based map navigation settings
│       └── enable — Enable/Disable navigation
└── 2d_lidar — 2D LiDAR settings
    └── enable — Enable/Disable LiDAR
```

```{warning}
After editing the settings, restart the clover2 service using the following command: 
```

```bash
sudo systemctl restart clover2
```

## Step-by-Step Example: Configuring the Map File

Open your terminal and run the following command to open the Terminal User Interface (TUI):

```bash
clover2-settings
```

```{figure} @assets@/common/setup/drone-settings/main-screen.webp
:alt: Экран настроек
:width: 90%
:align: center

Main screen
```

<br>

Use the `Up`/`Down` arrows to highlight `localization` and press `Enter`. 

```{figure} @assets@/common/setup/drone-settings/localization-menu.webp
:alt: Экран настроек localization
:width: 90%
:align: center

Localization menu
```

<br>

Select `map_server` to open the map server settings group.

```{figure} @assets@/common/setup/drone-settings/map-server-menu.webp
:alt: Экран настроек map_server
:width: 90%
:align: center

Map server menu
```

<br>

Select `map_filename` to begin editing. The parameter editor will open. 
Use the `Left`/`Right` arrow keys to move the cursor and modify the filename.
Press `Tab` to move to the action buttons.
Use the `Up`/`Down` arrows to select `Save` (to apply changes) or `Cancel` (to discard them). 

```{figure} @assets@/common/setup/drone-settings/map-filename-edit.webp
:alt: Экран настроек map_server
:width: 90%
:align: center

Map filename editing
```

<br>

To permanently write these changes to the configuration file, press `Ctrl+S`. 
A notification will appear confirming the save. 

```{figure} @assets@/common/setup/drone-settings/saved-example.webp
:alt: Сохранение настроек
:width: 90%
:align: center

Saved changes
```

<br>

To exit the program, press `Esc` several times until you return to the terminal.
Finally, restart the clover2 service to apply your new map configuration: 

```bash
sudo systemctl restart clover2
```
