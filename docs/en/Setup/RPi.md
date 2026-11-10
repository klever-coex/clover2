# Raspberry Pi

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

RPi/DroneConnection
RPi/ArucoMap
RPi/DroneSettings
```

To write the system image to your Raspberry Pi memory card, use the **balenaEtcher** program.

## Preparation

1. Install **balenaEtcher**: download the version compatible with your operating system from the [official website](https://etcher.balena.io/), install it, and launch the application.
2. Download the [latest system image for the Raspberry Pi 5](https://klever-doc.tech/ru/p/klever5).
3. Connect the memory card: insert your microSD card into your computer using a card reader. Ensure your operating system has successfully recognized the card before proceeding.

```{warning}
Writing the image will erase all data on the selected drive. Ensure you have backed up any necessary files and double-check that you have selected the correct memory card to avoid accidental data loss.
```

## Flashing the Image

1. In the main **balenaEtcher** window, click `Flash from file`.

    ```{figure} @assets@/common/setup/rpi/etcher-main-screen.webp
    :alt: balenaEtcher
    :width: 90%
    :align: center

   balenaEtcher main window
    ```

    <br>

2. Select your downloaded image file. Once selected, the file name and size will appear in the window.

    ```{tip}
    If the image is in a supported archive format, you do not need to extract it manually.
    ```

    ```{figure} @assets@/common/setup/rpi/etcher-image-selected.webp
    :alt: Selected image
    :width: 90%
    :align: center

    Selected image
    ```

    <br>

3. Click `Select target`. From the list of available drives, choose your microSD card and click `Select 1`. Verify that the drive name and capacity match your memory card.

    ```{figure} @assets@/common/setup/rpi/etcher-select-target.webp
    :alt: Selecting target drive
    :width: 90%
    :align: center

    Selecting target drive
    
    ```

    <br>

4. Review your selection one last time, then click `Flash!`. If prompted by your system, confirm the request for administrator privileges.

    ```{figure} @assets@/common/setup/rpi/etcher-ready-to-flash.webp
    :alt: Ready to flash
    :width: 90%
    :align: center

    Ready to flash
    ```

    <br>

5. Wait for the flashing and data verification processes to finish. Do not remove the memory card or disconnect the reader until both steps are fully complete.

    ```{figure} @assets@/common/setup/rpi/etcher-flashing.webp
    :alt: Flashing the card
    :width: 90%
    :align: center

    Flashing the card
    ```

    <br>

6. Once finished, a `Flash Completed!` message will appear on the screen.

    ```{figure} @assets@/common/setup/rpi/etcher-flash-completed.webp
    :alt: Flash completed message
    :width: 90%
    :align: center

   Flash Completed message
    ```

    <br>

7. Safely eject the memory card from your computer. Ensure your Raspberry Pi is powered off, then insert the memory card into its slot.
