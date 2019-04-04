<!-- RAMSES Citymodel Demo -->

## Table of Contents
* [What is the RAMSES Citymodel Demo](#what-is-the-ramses-citymodel-demo)
* [Obtaining the source code](#obtaining-the-source-code)
* [Requirements](#requirements)
* [Building](#building)
* [Running](#running)
* [License](#license)

## What is the RAMSES Citymodel Demo
The RAMSES Citymodel Demo contains a RAMSES client application, which provides a full textured 3D citymodel of the city of Helsinki.

## Obtaining the source code
RAMSES Citymodel Demo can be cloned from its Genivi repository using git:

```
git clone --recurse-submodules git@github.com:GENIVI/ramses-citymodel-demo.git
```

Ensure that you have installed Git LFS support before cloning, otherwise the database file ramses-citymodel.rex will
only be a few bytes long and the demo won't run.

RAMSES is pulled in as a submodule.

## Requirements
- CMake
- C++ compiler that supports C++11
- OpenGL ASTC texture compression, core since OpenGL ES 3.1

## Building
RAMSES Citymodel Demo uses CMake as build system. Under Linux build the demo with:

```
mkdir <build>
cd <build>
cmake <ramses-citymodel-demo>
make -j 8
```

This will generate the following executables in the bin subfolder:

- ramses-citymodel-client
- ramses-citymodel-renderer-x11-egl-es-3-0
- ramses-citymodel-renderer-wayland-ivi-egl-es-3-0

The wayland version will only be build, when Wayland is available. 

## Running
Run the RAMSES Citymodel Demo by starting the ramses-citymodel-renderer-XXX executable, e.g.:

```
cd bin
./ramses-citymodel-renderer-x11-egl-es-3-0
```

The executable combines the RAMSES renderer with the citymodel demo client. A window should now open, where the
animation of the camera path through the city of Helsinki can be seen.

The user can interact with the demo application by the following mouse commands:

Mouse command            | Description
------------------------ | -------------------------------------------------------------
Right-button click       |  Toggle between car-follow mode and interactive mode
Middle-button drag       |  Moving over the citymodel (only in interactive mode)
Left-button drag         |  Rotating the citymodel (vertically only in interactive mode)
Scroll wheel             |  Zooming in and out

When no input is done in interactive mode, the demo jumps back automatically to car-follow mode after a few seconds.
                       
For getting help about available command line parameters, use --help.

Beside the combined client-renderer, a pure client executable is also beeing build, which can be started by:

```
cd bin
ramses-citymodel-client
```

To see it's output, ramses-daemon and ramses-renderer have also to be started.

## License
RAMSES Citymodel Demo is copyright Mentor Graphics Development GmbH

```Copyright (C) 2018 Mentor Graphics Development GmbH```

The source code is licensed under the Mozilla Public License 2.0, please find a
copy of that license in the LICENSE.txt file.

The RAMSES Citymodel Demo makes use of the following open source libraries:

- RAMSES (Mozilla Public License 2.0)
- OpenCTM-1.0.3 (Proprietary License)
- cxxopts (Proprietary License)

Included Assets:

- Source Sans Pro Font (Licensed under Open Font License, see OFL.txt)

- Reality mesh model (CC BY 4.0, see CCBY4.txt)

The demo utilizes converted / compressed data of the reality mesh model of the city of Helsinki. The original data is
licensed under CC BY 4.0. The original data can be downloaded here:

[http://3d.hel.ninja/data/mesh/Helsinki3D-MESH_2015_OBJ_2km-250m_ZIP]
© City of Helsinki, Pohjoisesplanadi 11-13, P.O. Box 1, 00099 CITY OF HELSINKI
