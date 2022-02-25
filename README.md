TAO_NARIT
==
TAO NARIT package contains examples of Toolkit for Adaptive Optics (TAO) software framework implemented on camera devices available at NARIT.

The cameras are
1. Nuvu camera
2. Grasshopper Camera

C APIs of the cameras are stored in *src* folder. These examples serve as building blocks for TAO implementation.

### Grasshopper Examples
Each example is built by running **make** in the example directory.
eg. Acquisition example
```shell
cd ./src/Grasshopper_example/Acquisition
make
```

To run an Acquisition example with single frame acquisition mode

```shell
./Acquisition -s
```

For the list of options of the examples, see [here](https://github.com/Bossax/TAO_NARIT/blob/main/src/Grasshopper_example/.Grasshopper_README.md)

### Nuvu Examples
To build all Nuvu examples, run **make all** in the *Nuvu_Example* directory
```shell
cd src/Nuvu_example
make all
```
To run an example
```shell
cd Continuous_Acquisition/Continuous_Acquisition
./Continuous_Acquisition
```
This example takes 10 images and save them in *.fits* extension

### TAO_NUVU and TAO_SPINNAKER
**api.c** in both directories contain TAO wrapper functions of the cameras' APIs. An archive of each camera is built by running

```shell
make libtao-nuvu.a
make libtao-spinnaker.a
```
in the corresponding directory.

\*_test_01.c files are examples of using the archives to build them
```shell
make tao_spinnaker_test-01
```
or simply run `make` to compile verything
```shell
make
```
### Nuvu real-time image display window
`start_nuvu` is the program that invokes image acquisition and display real-time image. To compile, in `~/tao_nuvu/src` directory
```shell
make start_nuvu
```
To see available options eg.  camera setting and displayed image rotation
```shell
./start_nuvu --help
```
