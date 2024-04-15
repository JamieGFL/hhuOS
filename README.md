# hhuOS

<p align="center">
  <img src="media/logo/logo_v3.svg">
</p>

<p align="center">
  <a href="https://github.com/hhuOS/hhuOS/actions/workflows/build.yml"><img src="https://github.com/hhuOS/hhuOS/actions/workflows/build.yml/badge.svg"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg">
  <img src="https://img.shields.io/badge/license-GPLv3-orange.svg">
</p>

## Introduction

Welcome to hhuOS, a **small operating system** written in C++ and Assembler for the x86-architecture. The main purpose of this project is to show how different aspects of operating systems theory can be implemented and linked together. The system is *not* aimed to be a full-featured operating system for daily use.

This is a project by the [Operating Systems group](https://www.cs.hhu.de/en/research-groups/operating-systems.html) at the *Heinrich Heine University Düsseldorf*.

Check out our [wiki](https://github.com/hhuOS/hhuOS/wiki/)!

<p align="center">
  <a href="https://www.uni-duesseldorf.de/home/en/home.html"><img src="media/logo/hhu.svg" width=300></a>
</p>

## Compiling

GCC 7 and CMake 3.14 or newer versions of GCC and CMake are required to compile hhuOS.  
Before the kernel can be compiled, some packages must be installed. To install them, you can run the following command (on Ubuntu 20.04):

```sh
sudo apt install cmake make nasm gcc-multilib g++-multilib grub-pc-bin grub-efi-ia32-bin dosfstools mtools xorriso zstd unzip wget
```

## Usage

To test hhuOS quickly in QEMU, you can issue the following commands.

```sh
git clone https://github.com/hhuOS/hhuOS.git
cd hhuOS/
git submodule initialize
git submodule update
./build.sh
./run.sh
```

See the [wiki](https://github.com/hhuOS/hhuOS/wiki/Build-and-run-hhuOS) for more details.

## Notes

Assets for the mouse demo haven been taken from [Icons8](https://icons8.com/).

Assets for the dino game have been taken from [itch.io](https://itch.io):
 - [Dino Characters](https://arks.itch.io/dino-characters) by [*@ScissorMarks*](https://twitter.com/ScissorMarks) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))
 - [Dino Family](https://demching.itch.io/dino-family) by [*DemChing*](https://demching.itch.io/) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))
 - [Pixel Adventure](https://pixelfrog-assets.itch.io/pixel-adventure-1) by [*Pixel Frog*](https://pixelfrog-assets.itch.io/) ([CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/legalcode))
 - [Retro Cloud Tileset](https://ohnoponogames.itch.io/retro-cloud-tileset) by [*ohnoponogames*](https://ohnoponogames.itch.io/)

Assets for the bug defender game have been taken from [itch.io](https://itch.io):
- [Lunar Battle Pack](https://mattwalkden.itch.io/lunar-battle-pack) by [*MattWalkden*](https://mattwalkden.itch.io/) ([CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/legalcode))
- [Pixel Heart Animation](https://nicolemariet.itch.io/pixel-heart-animation-32x32-16x16-freebie) by [*Nicole Marie T*](https://nicolemariet.itch.io/)

3D-demo assets haven been taken from [itch.io](https://itch.io):
- [M1 Tank](https://alstrainfinite.itch.io/m1-tank) by [*Alstra Infinite*](https://alstrainfinite.itch.io/) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))
- [Shark](https://alstrainfinite.itch.io/fish) by [*Alstra Infinite*](https://alstrainfinite.itch.io/) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))
- [Plane](https://alstrainfinite.itch.io/planes) by [*Alstra Infinite*](https://alstrainfinite.itch.io/) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))
- [Coffee Maker](https://alstrainfinite.itch.io/kitchen-appliance-2) by [*Alstra Infinite*](https://alstrainfinite.itch.io/) ([CC BY 4.0](https://creativecommons.org/licenses/by/4.0/legalcode))

Assets for the battlespace game have been taken from [itch.io](https://itch.io):
- [Lowpoly - 3D Space Assets Pack](https://ejgarner118.itch.io/spacepack) by [*ejgarner118*](https://ejgarner118.itch.io/)

Music for the SoundBlaster demo has been taken from [Bensound](https://www.bensound.com/royalty-free-music):
- Ukulele license code: 3M7PXYPYNOTSIGNQ