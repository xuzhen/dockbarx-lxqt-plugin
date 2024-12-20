# DockbarX plugin for the LXQt panel
### Version 0.9.4

For Ubuntu users, this plugin can be installed from the PPA 
```
sudo add-apt-repository ppa:xuzhen666/dockbarx
sudo apt update
sudo apt install dockbarx-lxqt-plugin
```

For Arch Linux users, this plugin can be installed from the [AUR](https://aur.archlinux.org/packages/dockbarx-lxqt-plugin)

## Build dependencies
 * LXQt panel
 * liblxqt header files (the same version used to complie LXQt panel)
 * Qt 5/6 (the same version used to complie LXQt panel)
 * CMake (>= 3.7.0)
 * make
 * C++ compiler

 For Debian/Ubuntu users, these dependencies can be installed using the following command
 ```
 # For lxqt-panel 2.x
 sudo apt install qt6-base-dev lxqt-panel liblxqt2-dev cmake make g++
 # For lxqt-panel 1.x
 sudo apt install qtbase5-dev libqt5x11extras5-dev lxqt-panel liblxqt1-dev cmake make g++
 # For lxqt-panel 0.x
 sudo apt install qtbase5-dev libqt5x11extras5-dev lxqt-panel liblxqt0-dev cmake make g++
 ```

## Extra runtime dependencies
 * [DockbarX](https://github.com/xuzhen/dockbarx)

## Installation

```
git clone https://github.com/xuzhen/dockbarx-lxqt-plugin.git
cd dockbarx-lxqt-plugin
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```
