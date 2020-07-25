# PandaOS Files

This is a file manager and desktop, built for pandaos, the core is based on libfm

## Dependencies

`sudo pacman -S cmake qt5-base qt5-x11extras qt5-tools udisks2-qt5 menu-cache libexif xdg-user-dirs`

## Build

```shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## Create user directories

`$ xdg-user-dirs-update`

## License

panda-files is licensed under GPLv3.
