[![ANYKS - convert utility](https://raw.githubusercontent.com/anyks/acu/main/img/banner.jpg)](https://anyks.com)

# ANYKS - convert utility (ACU) C++

### Format compatibility
| **#**      | **XML** | **JSON** | **YAML** | **INI** | **CSV** | **CEF** | **SYSLOG** | **GROK** |
|------------|---------|----------|----------|---------|---------|---------|------------|----------|
| **XML**    | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **JSON**   | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **YAML**   | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **INI**    | Yes     | Yes      | Yes      | Yes     | No      | No      | No         | No       |
| **CSV**    | NO      | Yes      | Yes      | No      | Yes     | No      | No         | No       |
| **CEF**    | NO      | Yes      | Yes      | No      | No      | No      | No         | No       |
| **SYSLOG** | NO      | Yes      | Yes      | No      | No      | No      | No         | No       |

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
```

### Build third party for MacOS X, Linux and FreeBSD

```bash
$ ./build_third_party.sh
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
$ echo '{"Hello": "World!!!"}' | acu -from json -to xml -prettify
```

---

### Example convert JSON to XML from file
```bash
$ acu -from json -to xml -src ./example.json -prettify
```

#### OR
```bash
$ acu -from json -to xml -src ./example.json -dest ./result -prettify
```

---

### Example convert CSV to XML from file
```bash
$ acu -from csv -to xml -src ./example.csv -header -prettify
```

#### OR
```bash
$ acu -from csv -to xml -src ./example.csv -dest ./result -header -prettify
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
$ acu -from cef -to xml -src ./example.txt -cef low -prettify
```

#### OR
```bash
$ acu -from cef -to xml -src ./example.txt -dest ./result -cef low -prettify
```

---

### Example convert GROK to XML without patterns from file
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -prettify
```

#### OR
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -dest ./result -prettify
```

---

### Example convert GROK to XML with patterns from file
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -patterns ./patterns.json -prettify
```

#### OR
```bash
$ acu -from grok -to xml -src ./example.txt -express ./express.txt -patterns ./patterns.json -dest ./result -prettify
```

---

### Example convert JSON to XML from folder
```bash
$ acu -from json -to xml -src ./input -dest ./result -prettify
```
