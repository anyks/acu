[![ANYKS - convert utility](https://raw.githubusercontent.com/anyks/acu/main/img/banner.jpg)](https://anyks.com)

# ANYKS - convert utility (ACU) C++

## Project goals and features

- **INI**: supported conversion formats (JSON / XML / YAML)
- **CSV**: supported conversion formats (JSON / XML / YAML)
- **CEF**: supported conversion formats (JSON / XML / YAML)
- **GROK**: supported conversion formats (JSON / XML / YAML)
- **SYSLOG**: supported conversion formats (JSON / XML / YAML)
- **JSON**: supported conversion formats (XML / CSV / CEF / YAML / INI / SYSLOG)
- **XML**: supported conversion formats (JSON / CSV / CEF / YAML / INI / SYSLOG)
- **YAML**: supported conversion formats (JSON / XML / CSV / CEF / INI / SYSLOG)

## Requirements

- [AWH](https://github.com/anyks/awh)
- [LibXML2](https://github.com/GNOME/libxml2)
- [YAML-CPPT](https://github.com/jbeder/yaml-cppt)

## To build and launch the project

### To clone the project

```bash
$ git clone --recursive https://github.com/anyks/acu.git
```

### Activate SCTP only (FreeBSD / Linux)

#### FreeBSD

```bash
$ sudo kldload sctp
```

#### Linux (Ubuntu)

```bash
$ sudo apt install libsctp-dev
$ sudo modprobe sctp
$ sudo sysctl -w net.sctp.auth_enable=1

### Build third party for MacOS X, Linux and FreeBSD

```bash
$ ./build_third_party.sh --idn
```

### Build on MacOS X, Linux and FreeBSD

```bash
$ mkdir ./build
$ cd ./build

$ cmake \
 -DCMAKE_BUILD_IDN=YES \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_SHARED_BUILD_LIB=YES \
 ..

$ make
```

### Build on Windows [MSYS2 MinGW]

#### Development environment configuration
- [GIT](https://git-scm.com)
- [Perl](https://strawberryperl.com)
- [Python](https://www.python.org/downloads/windows)
- [MSYS2](https://www.msys2.org)
- [CMAKE](https://cmake.org/download)

#### Assembly is done in MSYS2 - MINGW64 terminal

```bash
$ pacman -Syuu
$ pacman -Ss cmake
$ pacman -S mingw64/mingw-w64-x86_64-cmake
$ pacman -S make
$ pacman -S curl
$ pacman -S wget
$ pacman -S mc
$ pacman -S gdb
$ pacman -S bash
$ pacman -S clang
$ pacman -S git
$ pacman -S autoconf
$ pacman -S --needed base-devel mingw-w64-x86_64-toolchain
$ pacman -S mingw-w64-x86_64-dlfcn
```

### Build third party for MS Windows
```bash
$ ./build_third_party.sh
```

#### Project build

```bash
$ mkdir ./build
$ cd ./build

$ cmake \
 -G "MSYS Makefiles" \
 -DCMAKE_BUILD_IDN=YES \
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_SYSTEM_NAME=Windows \
 -DCMAKE_SHARED_BUILD_LIB=YES \
 ..

$ cmake --build .
```

### Example convert JSON to XML
```bash
$ echo '{"Hello": "World!!!"}' | acu -from json -to xml -pretty
```

---

### Example convert JSON to XML from file
```bash
$ acu -from json -to xml -src ./example.json -pretty
```

#### OR
```bash
$ acu -from json -to xml -src ./example.json -dest ./result -pretty
```

---

### Example convert CSV to XML from file
```bash
$ acu -from csv -to xml -src ./example.csv -header -pretty
```

#### OR
```bash
$ acu -from csv -to xml -src ./example.csv -dest ./result -header -pretty
```

---

### Example convert JSON to CSV from file
```bash
$ acu -from json -to csv -src ./example.json -delim ","
```

#### OR
```bash
$ acu -from json -to csv -src ./example.json -dest ./result -delim ","
```

---

### Example convert CEF to XML from file
```bash
$ acu -from cef -to xml -src ./example.txt -cef low -pretty
```

#### OR
```bash
$ acu -from cef -to xml -src ./example.txt -dest ./result -cef low -pretty
```

---

### Example convert GROK to XML without patterns from file
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -pretty
```

#### OR
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -dest ./result -pretty
```

---

### Example convert GROK to XML with patterns from file
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -patterns ./patterns.json -pretty
```

#### OR
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -patterns ./patterns.json -dest ./result -pretty
```

---

### Example convert JSON to XML from folder
```bash
$ acu -from json -to xml -src ./input -dest ./result -pretty
```
