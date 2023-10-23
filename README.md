# PhoeniSX2

_Born-from-the-ashes ARM PS2 emulator._

## Objectives

This project wants to be an attempt at building an up-to-date PS2 emulator for ARM devices from the sadly defunct AetherSX2 emulator, originally by Talhreth by a PCSX2 fork.

* [ ] Ensure working desktop builds
* [ ] (New) Android app
* [ ] Libretro core port

What is more worth as the first objective besides desktop, depends on what would be easier. For Android, we are missing needed Android-specific C++ code (that we must probably infer from the glued pieces we have, as a recent Android source can't be found around), and ideally we would also rebuild the Android app itself from scratch (because of the heavy obfuscation and licensing issues of the original one). For Libretro, seeing how the RetroArch project had to fork the PCSX2 emulator to make it work as a Libretro core (LRPS2), this suggests we would need a lot of patches in a lot of places (but mostly there should be no guess work for how to make them).

## Building for desktop Linux

### Build dependencies

Debian-based:

```
sudo apt update
sudo apt install -y \
	make cmake gcc g++ qt6-base-dev qt6-tools-dev \
	libasound-dev libpcap-dev liblzma-dev libpng-dev libaio-dev libudev-dev libsoundtouch-dev libsamplerate-dev
```

### Actual building

```
# git clone --depth=1 <this repository>
mkdir -p PhoeniSX2/build-linux-desktop
cd PhoeniSX2/build-linux-desktop
cmake -DCMAKE_BUILD_TYPE=Release -DQT_BUILD=ON ..
make -j$(nproc --all)
```

## References

* Upstreams of [PCSX2](https://github.com/PCSX2/pcsx2) and [LRPS2](https://github.com/libretro/LRPS2)
* Original AetherSX2 desktop LGPL source code, dated 2022-05-03: [tree](https://github.com/andrigamerita/PhoeniSX2/tree/bcbfb1754e70ebc61a988319b08bf6d217945275), [zip](https://github.com/andrigamerita/PhoeniSX2/archive/bcbfb1754e70ebc61a988319b08bf6d217945275.zip)
* AetherSX2 Android source code that compiles to alpha, dated 2021-12-04: [tarball](https://www.aethersx2.com/lgpl/aethersx2-lgpl.tar.gz)
* Buils archives: [Android (APKMirror)](https://www.apkmirror.com/uploads/?appcategory=aethersx2), [others (aethersx2.com)](https://www.aethersx2.com/archive/)

