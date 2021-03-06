﻿Motorcar
========

**For technical questions regarding Motorcar please contact the [motorcar-devel](https://groups.google.com/forum/#!forum/motorcar-devel)  mailing list**

Motorcar is a framework for 3D windowing built on top of Wayland which I originally developed for my Master's thesis at Cal Poly ([pdf](https://github.com/evil0sheep/MastersThesis/blob/master/thesis.pdf?raw=true), [defense slides](https://docs.google.com/presentation/d/1svgGMxxbfmcHy_KuS5Q9hah8PQOsXqvjBKOoMIzW24Y/edit?usp=sharing)). It is designed to provide basic 3D windowing infrastructure that gives 3D applications desktop flexibility in the how their 3D content is drawn while supporting unmodified Wayland applications in the same 3D compositor space, and to do this with the simplest mechanism possible.

Motorcar is free and open source (under the BSD license), and I am very open to contributions (both conceptual and functional) from the community. If you have any questions, comments, or critical feedback about the software, or if you are interested in using or contributing Motorcar, or if you are working on something related, please feel free to contact me, I would love to hear from you.

Doxygen generated class documentation is available in the repository [here](http://htmlpreview.github.io/?https://github.com/evil0sheep/motorcar/blob/stable/doc/html/annotated.html), though it is somewhat sparse.

Status of this project
======================

simula_cpp is not actively developed. It has been superseded by https://github.com/SimulaVR/Simula

The key differences are:

simula_cpp is a straightforward update of motorcar. The Oculus SDK dependency has been replaced with OSVR and OSVR-Rendermanager, but it still depends on qtwayland 5.5 and thus Qt 5.5 (can be automatically built with the build script).

Simula VR is a more extensive rewrite of motorcar in Haskell, it makes use of libweston and is looking into interfacing with Godot Engine for using its VR rendering and input pipeline as well as integrating the desktop into environments made in Godot. Simula VR is currently using OpenVR directly, but with Godot integration it will be possible to use it with godot_openvr, godot_openhmd, etc.

simula_cpp has several known issues, but it may be useful as a basis for developing a VR wayland compositor using OSVR.

Some of these known issues are:

* the dependency on an old Qt and qtwayland version
* motion controller input (e.g. mouse click emulation) works in xwayland, but not in native wayland windows
* no consistent motion controller input mapping in OSVR. The mapping has to be set up in src/device/osvr_controller.cpp if the OSVR plugin does not offer buttons in the form `/controller/left/1`
* some stability issues remain

Building Motorcar
=================

Using the Build Script
----------------------

The repository contains a simple build script, [build.sh](https://github.com/evil0sheep/motorcar/blob/master/build.sh), which will build qt5.5 with qtwayland, libmotorcar-server, and the three example compositors. It is invoked as follows:

	$ ./build.sh [target1 [target2 [...]]]

Where target is one of the following:

* qtwayland
	* Builds qt5.5 with qtwayland as per the instructions below
	* This target will prompt twice to verify that EGL settings are correct, once when configuring qt5 and once after running qmake for qtwayland. If the output when these prompts appear do not show EGL being supported on Desktop OpenGL the compositor will not work with EGL clients like the motorcar example client. SHM clients may still work
* libmotorcar-compositor
	* Builds the motorcar compositor libraries, used by the example compositors
	* This target has no prompts
* simple-compositor
	* Builds a simple demo compositor with no hardware dependencies
	* Produces a shell script "run-simple-compositor.sh" which runs the compiled compositor
	* This target has no prompts
* rift-hydra-compositor
	* Builds the compositor which uses the Oculus Rift DK2 and Razer Hydra to create an immersive 3D windowing experience
	* This target depends on the [OculusVR SDK 0.5.0.1](https://developer.oculus.com/downloads/pc/0.5.0.1-beta/Oculus_SDK_for_Linux_(Experimental)/) and the [SixenseVR SDK](http://sixense.com/linuxsdkdownload) and will prompt for the location of the downloaded SDK archives in order to set up linking properly (it will only prompt the first time it is built)
	* Produces a shell script "run-rift-hydra-compositor.sh" which runs the compiled compositor
* osvr-compositor
	* Builds the compositor which uses the OSVR SDK to create an immersive 3D windowing experience
	* This target depends on the [OSVR SDK](https://github.com/OSVR/OSVR-Core/) and [OSVR Rendermanager](https://github.com/sensics/OSVR-RenderManager/)
	* Produces a shell script "run-osvr-compositor.sh" which runs the compiled compositor
* motorcar-demo-client
 	* Builds a simple 3D windowing demo client which uses the motorcar 3D windowing extensions to create a cuboid window and draw a rotating colored cube inside of it.
 	* Produces a shell script "run-motorcar-demo-client.sh" which runs the compiled client

Multiple targets can be built simultaneously, but they are built in the order that they are specified, so if you want to build mutliple targets you need to specify qtwayland before libmotorcar-compositor and libmotorcar-compositor before either of the example compositors. To build all targets, pass

 	$ ./build.sh qtwayland libmotorcar-compositor rift-hydra-compositor osvr-compositor simple-compositor motorcar-demo-client

A minimal build for VR requires qt, libmotorcar-compositor and osvr-compositor:

 	$ ./build.sh qtwayland libmotorcar-compositor osvr-compositor

The build script requires tar, and unzip to function

Dependencies
------------

Motorcar has significant external dependencies, some of which may need to be built from source depending on your Linux distribution. These dependencies can be summarized at a high level as a dependency on the [QtWayland QtCompositor for Qt 5.5](http://qt-project.org/wiki/QtWayland) module with support for EGL with desktop OpenGL (not OpenGL ES). This in turn requires [Wayland](http://wayland.freedesktop.org/), which may be available as a package depending on your Linux distribution, or may need to be [built from source](http://wayland.freedesktop.org/building.html), as well as support for EGL with desktop OpenGL, which may require building Mesa from source. Instructions for building Mesa for Wayland EGL can be found in the [Wayland build instructions](http://wayland.freedesktop.org/building.html).

Details about the build process of motorcar and its dependencies are described in README-details.md.
