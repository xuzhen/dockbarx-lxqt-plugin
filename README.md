# DockbarX plugin for the LXQt panel
### Version 0.9

For Ubuntu users, this plugin can be installed from the PPA 
```
sudo add-apt-repository ppa:xuzhen666/dockbarx
sudo apt update
sudo apt install dockbarx-lxqt-plugin
```

## Build dependencies
 * Qt5 (>= 5.6.0), with its X11 extras
 * LXQt panel
 * CMake (>= 3.7.0)
 * make
 * C++ compiler

 For Debian/Ubuntu users, these dependencies can be installed using the following command
 ```
 sudo apt install qtbase5-dev libqt5x11extras5-dev lxqt-panel cmake make g++
 ```

## Extra runtime dependencies
 * [DockbarX](https://github.com/xuzhen/dockbarx)

## Installation

```
git clone https://github.com/xuzhen/dockbarx-lxqt-plugin.git
cd dockbarx-lxqt-plugin
mkdir build
cd build
cmake ..
make
sudo make install
```
The LXQt panel will probably not detect the plugin unless you install it in the /usr prefix, so instead do the configuration step with 
```
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```
