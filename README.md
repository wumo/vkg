# vkg

# Prerequisite

install `python3`,`git`,`gcc`,`cmake`,`make`,`conan`.

## Ubuntu
```
$ sudo apt install -y git gcc cmake make
# install conan
$ pip install conan
```

## Windows
Using [scoop](https://scoop.sh/) to install dependencies:
```
# install scoop
$ Set-ExecutionPolicy RemoteSigned -scope CurrentUser
$ iex (new-object net.webclient).downloadstring('https://get.scoop.sh')
# install dependencies
$ scoop install python git gcc cmake
# install conan
$ pip install conan
```

# Additional conan remotes
```
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

``` 

# Build
```
mkdir build && cd build

# (win)
$ cmake .. -G "Visual Studio 15 Win64"
$ cmake --build . --config Release

# (linux, mac)
$ cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
```