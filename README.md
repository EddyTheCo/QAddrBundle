# Esterv.Iota.AddressWallet 

[TOC]

This repo implements methods for controlling Iota assets, outputs associated to certain address.
In principle should be like a wallet that keeps track the assets controlled by that address.


## Installing the library 

### From source code
```
git clone https://github.com/EddyTheCo/QAddrBundle.git 

mkdir build
cd build
qt-cmake -G Ninja -DCMAKE_INSTALL_PREFIX=installDir -DCMAKE_BUILD_TYPE=Release -DUSE_QML=OFF -DBUILD_DOCS=OFF ../QAddrBundle

cmake --build . 

cmake --install . 
```
where `installDir` is the installation path. Setting the `USE_QML` variable produce or not the QML module.

### From GitHub releases
Download the releases from this repo. 

## Adding the libraries to your CMake project 

```CMake
include(FetchContent)
FetchContent_Declare(
	IotaAddressWallet	
	GIT_REPOSITORY https://github.com/EddyTheCo/QAddrBundle.git
	GIT_TAG vMAJOR.MINOR.PATCH 
	FIND_PACKAGE_ARGS MAJOR.MINOR CONFIG  
	)
FetchContent_MakeAvailable(IotaAddressWallet)

target_link_libraries(<target> <PRIVATE|PUBLIC|INTERFACE> IotaAddressWallet::addrBundle)
```

## API reference

You can read the [API reference](https://eddytheco.github.io/QAddrBundle/), or generate it yourself like
```
cmake -DBUILD_DOCS=ON ../
cmake --build . --target doxygen_docs
```


## Contributing

We appreciate any contribution!


You can open an issue or request a feature.
You can open a PR to the `develop` branch and the CI/CD will take care of the rest.
Make sure to acknowledge your work, and ideas when contributing.



