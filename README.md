[![ANYKS - convert utility](https://raw.githubusercontent.com/anyks/acu/main/img/banner.jpg)](https://anyks.com)

# ANYKS - convert utility (ACU) C++

### Format compatibility
| **#**      | **XML** | **JSON** | **YAML** | **INI** | **CSV** | **CEF** | **SYSLOG** | **GROK** |
|------------|---------|----------|----------|---------|---------|---------|------------|----------|
| **XML**    | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **JSON**   | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **YAML**   | Yes     | Yes      | Yes      | Yes     | Yes     | Yes     | Yes        | Yes      |
| **INI**    | Yes     | Yes      | Yes      | Yes     | No      | No      | No         | No       |
| **CSV**    | No      | Yes      | Yes      | No      | Yes     | No      | No         | No       |
| **CEF**    | No      | Yes      | Yes      | No      | No      | No      | No         | No       |
| **SYSLOG** | No      | Yes      | Yes      | No      | No      | No      | No         | No       |

## Requirements

- [AWH](https://gitflic.ru/project/anyks/awh)
- [LibXML2](https://gitflic.ru/project/third_party/libxml2)
- [YAML-CPP](https://gitflic.ru/project/third_party/yaml-cpp)

## To build and launch the project

### To clone the project

```bash
$ git clone --recursive https://gitflic.ru/project/anyks/acu.git
```

### Activate SCTP only (FreeBSD / Linux)

#### FreeBSD

```bash
$ sudo kldload sctp
```

#### Linux (ALT)

```bash
$ sudo apt-get install liblksctp-devel
$ sudo modprobe sctp
$ sudo sysctl -w net.sctp.auth_enable=1
```

#### Linux (Ubuntu)

```bash
$ sudo apt install libsctp-dev
$ sudo modprobe sctp
$ sudo sysctl -w net.sctp.auth_enable=1
```

#### Linux (Fedora)

```bash
$ sudo yum install lksctp-tools-devel
$ sudo modprobe sctp
$ sudo sysctl -w net.sctp.auth_enable=1
```

#### Linux (openSUSE)

```bash
$ sudo zypper install lksctp-tools-devel
$ sudo modprobe sctp
$ sudo sysctl -w net.sctp.auth_enable=1
```

### Build third party for MacOS X, Linux, FreeBSD and Solaris

```bash
$ ./build_third_party.sh
```

### Build on MacOS X, Linux, FreeBSD and Solaris

```bash
$ mkdir ./build
$ cd ./build

$ cmake \
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
 -DCMAKE_BUILD_TYPE=Release \
 -DCMAKE_SYSTEM_NAME=Windows \
 -DCMAKE_SHARED_BUILD_LIB=YES \
 ..

$ cmake --build .
```

### Example convert text to Base64
```bash
$ echo 'Hello World!!!' | acu -from text -to base64
```

---

### Example convert Base64 to text
```bash
$ echo 'SGVsbG8gV29ybGQhISE=' | acu -from base64 -to text
```

---

### Example convert text to MD5
```bash
$ echo 'Hello World!!!' | acu -from text -to MD5
```

---

### Example convert text to HMAC SHA256
```bash
$ echo 'Hello World!!!' | acu -from text -to SHA256 -hmac 236bf30c70dc03f69175f030afbe38f3
```

---

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

---

### Converting numbers from decimal to binary
```bash
$ echo "3369" | acu -notation -from 10 -to 2
```

### Converting numbers from Arabic to Roman numerals
```bash
$ echo "3369" | acu -notation -from 10 -to 1
```

### Converting numbers from binary to decimal
```bash
$ echo "0000110100101001" | acu -notation -from 2 -to 10
```

### Convert text to binary
```bash
$ echo "Hello World" | acu -notation -from 0 -to 2
```

### Converting from binary to text
```bash
$ echo "0110010001101100011100100110111101010111001000000110111101101100011011000110010101001000" | acu -notation -from 2 -to 0
```

---

### Generate date from timestamp
```bash
$ echo "1740268539834" | acu -date -from timestamp -to date -formatDate "%a, %d %b %Y %H:%M:%S %Z"
```

### Generate timestamp from date
```bash
$ echo "Sun, 23 Feb 2025 02:55:39 MSK" | acu -date -to timestamp -from date -formatDate "%a, %d %b %Y %H:%M:%S %Z"
```

---

### Convert seconds to minutes
```bash
$ echo "120" | acu -seconds -from s -to m
```

### Convert seconds to hours
```bash
$ echo "16200" | acu -seconds -from s -to h
```

### Convert seconds to days
```bash
$ echo "1555200" | acu -seconds -from s -to d
```

### Convert Seconds to Weeks
```bash
$ echo "1555200" | acu -seconds -from s -to w
```

### Convert seconds to months
```bash
$ echo "7884000" | acu -seconds -from s -to M
```

### Convert seconds to years
```bash
$ echo "7884000" | acu -seconds -from s -to y
```

---

### Convert bytes to kilobytes
```bash
$ echo "4096" | acu -bytes -from b -to Kb
```

### Convert bytes to megabytes
```bash
$ echo "4096" | acu -bytes -from b -to Mb
```

### Convert bytes to gigabytes
```bash
$ echo "34896609280" | acu -bytes -from b -to Gb
```

### Convert bytes to terabytes
```bash
$ echo "3188583720550.4" | acu -bytes -from b -to Tb
```

---

## Creating installation packages

### Creating a tar.gz archive
```bash
$ ./make_tar_archive.sh
```

### Creating a deb package
```bash
$ ./linux_make_deb.sh
```

### Creating an rpm package
```bash
$ ./linux_make_rpm.sh
```

### Creating an Installation Package for MacOS X
```bash
$ ./make_installer_macos.sh
```
> Installation package will be located in the directory **./package/MacOS/target/pkg**

### Create exe package
```bash
$ ./windows_make_installer.sh
```
> To create an installation package, you need to open the **install.iss** file from the **win** directory in the application [Inno Setup](https://jrsoftware.org/isinfo.php)

## Installation from the installation package

### Installation from tar.gz
```bash
$ tar -xzvf ./acu_1.0.0_MacOSX_arm64.tar.gz -C /
```

### Installation from deb package
```bash
$ sudo dpkg -i ./acu_1.0.0-1~buster_amd64.deb
```

### Installation from rpm package
```bash
$ sudo rpm -i ./acu-1.0.0-1.amd64.rpm
```

---

## Basic launch parameters

### Getting the version application
```bash
$ acu -V

## Result:
# 
# ANYKS - conversion utility 1.0.4 (built: Oct 31 2024 21:22:56)
# awh: 4.2.6
# target: MacOS X
# installed dir: /opt/acu/bin
# 
# *
# * site:     https://acu.anyks.com
# * email:    info@anyks.com
# * telegram: @forman
# *
#
```
> There are several ways to get the version of an application, the short version is **"-V"** or the full version is **"--version"**.

### Getting reference information
```bash
$ acu -H

## Result:
# 
# usage: acu [-V | --version] [-H | --info] [<args>]
# 
# 
# [FLAGS]
# + Flag for generating headers when parsing CSV files: [-header | --header]
# 
# + Flag for generating a readable file format (XML or JSON): [-prettify | --prettify]
# 
# + Display application version: [-version | --version | -V]
# 
# + Display information about available application functions: [-info | --info | -H]
# 
# [ARGS]
# + CEF file parsing mode: [-cef <value> | --cef=<value>]
#   - (LOW | MEDIUM | STRONG)
# 
# + Format of the file from which reading is performed: [-from <value> | --from=<value>]
#   - (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | GROK | TEXT | BASE64)
# 
# + File format to which the writing is made: [-to <value> | --to=<value>]
#   - (XML | JSON | INI | YAML | CSV | CEF | SYSLOG | TEXT | BASE64 | MD5 | SHA1 | SHA224 | SHA256 | SHA384 | SHA512)
# 
# + Format date (if required): [-formatDate "<value>" | --formatDate="<value>"]
#   - ( %m/%d/%Y %H:%M:%S | %H:%M:%S %d.%m.%Y | ... )
# 
# + Logging level (if required): [-logLevel <value> | --logLevel=<value>]
#   - ( 0 = NONE | 1 = INFO | 2 = WARNING | 3 = CRITICAL | 4 = INFO and WARNING | 5 = INFO and CRITICAL | 6 = WARNING CRITICAL | 7 = ALL)
# 
# + Hash-based message authentication code: [-hmac <value> | --hmac=<value>]
# 
# + File address for writing logs (if required): [-log <value> | --log=<value>]
# 
# + File or directory address for saving converted files: [-dest <value> | --dest=<value>]
# 
# + Separator for parsing CSV files (default: ";"): [-delim <value> | --delim=<value>]
# 
# + Address of the file or directory with files to convert: [-src <value> | --src=<value>]
# 
# + Address of the file in JSON format with GROK templates: [-patterns <value> | --patterns=<value>]
# 
# + Address of the file with the regular expression in GROK format: [-express <value> | --express=<value>]
# 
```
> There are several ways to get help information, the short version is **"-H"** or the full version is **"--info"**.

### General concepts

It should be explained in advance that the launch parameters can be specified in any order; there is a full and short form for setting the launch parameters.

- **Full form:** --<NAME>=<VALUE>
- **Short form:** -<NAME> <VALUE>
> For example, the entries **--config=config.json** and **-config config.json** are equivalent.

To simplify work with **ACU Server**, there is a configuration file in **JSON** format, all startup parameters can be specified in this configuration file. By default, each operating system has its own set of paths by which **ACU Server** searches for the configuration file. The path by which **ACU Server** will search for the configuration file is specified in the **[PATHS]** section of the reference information. The address of the configuration file can be specified independently using the **config** parameter.

For **UNIX-like** operating systems, parameters can be specified using environment variables with the **ACU_** prefix.

---

## Description of the ACU Server configuration file

### Configuration file example
```json
{
	
	"ssl": {
		"key": "",
		"cert": "",
		"verify": false
	},
	"net": {
		"ipv": 4,
		"wait": 15,
		"sonet": "tcp",
		"host": "127.0.0.1",
		"port": 2222,
		"proto": "http2",
		"unixSocket": "",
		"total": 100,
		"ipv6": false,
		"filter": {
			"type": "mac",
			"black": [],
			"white": []
		},
		"compress": [
			"ZSTD",
			"BROTLI",
			"GZIP",
			"DEFLATE"
		],
		"authentication": {
			"enabled": false,
			"auth": "Basic",
			"digest": "MD5",
			"users": {"admin": "admin"}
		},
		"bandwidth": {
			"read": "20Mbps",
			"write": "20Mbps"
		}
	},
	"workers": 0,
	"user": "auto",
	"group": "auto",
	"boost": true,
	"maxRequests": 600,
	"root": "./html",
	"log": "./acu.log",
	"pidfile": "acu.pid",
	"index": "index.html",
	"favicon": "./icons/icon.ico",
	"origin": "http://127.0.0.1:2222",
	"formatDate": "%H:%M:%S %d.%m.%Y"
}
```

### Description of configuration file parameters
| **Parameter name**     | **Parameter Description** |
|------------------------|---------------------------|
| **ssl** | Object of parameters for **secure network SSL connection** to network services. |
| **ssl.verify** | Parameter for activating the **SSL certificate validity check** mode. |
| **ssl.key** | Key of the certificate file in the file system, for working in secure server mode. |
| **ssl.cert** | Certificate file in the file system, for working in secure server mode. |
| **server** | Server parameters |
| **server.ipv** | Supported Internet Protocol Type **IPv4** or **IPv6**. |
| **server.wait** | Time to wait for data to be received from the client in seconds. |
| **server.sone** | Internet connection protocol (TCP, TLS, SCTP). |
| **server.host** | Server host through which the connection is made. |
| **server.port** | Server port through which the connection is made. |
| **server.proto** | Default server application protocol (http or http2). |
| **server.unixSocket** | Name of the **Unix socket** in the file system, through which the connection to the server will be made. The Unix socket will be located in the **/tmp** directory. |
| **server.total** | Number of clients simultaneously connected to one server. |
| **server.ipv6** | Perform server communication only via **IPv6**. |
| **server.filter** | An object that filters the list of clients connecting to the server. |
| **server.filter.type** | Filter clients by **MAC** or **IP** address. |
| **server.filter.black** | List of clients that are prohibited from connecting to the server. |
| **server.filter.white** | List of clients that are only allowed to connect to the server. |
| **server.compres** | List of compressors supported by the server, in priority order (ZSTD, BROTLI, GZIP, DEFLATE, LZ4, LZMA, BZIP2). |
| **server.authentication** | Server authorization parameters object. |
| **server.authentication.enabled** | Parameter for activating the authorization mode on the server. |
| **server.authentication.auth** | Server authorization method **Basic** or **Digest**. |
| **server.authentication.digest** | Hash type for the **Digest** authentication method (**MD5**, **SHA1**, **SHA256**, **SHA512**). |
| **server.authentication.username** | Username for authorization on the server. |
| **server.authentication.password** | User password for authorization on the server. |
| **server.bandwidth** | Object of network connection speed limit parameters for the server. |
| **server.bandwidth.read** | Maximum data transfer rate in read mode. |
| **server.bandwidth.write** | Maximum data transfer rate in recording mode. |
| **workers** | Number of cluster processes, if the value is not set or set to -1, the cluster will not be activated. Set to 0, the cluster will automatically create the number of processes for your machine. |
| **user** | Name or user ID under which the application should be launched. The "auto" parameter sets the current user. |
| **group** | Name or identifier of the user group under which the application should be launched. The "auto" parameter sets the current user group. |
| **boost** | Flag for activating reconfiguration of the operating system kernel for maximum network protocol performance. |
| **maxRequests** | Maximum number of requests to the server per day available to one specific user. |
| **root** | Catalog with the site location in HTML format. |
| **log** | Address of the log file where the processes occurring on the server will be recorded. |
| **pidfile** | Name of the **PID file** located in the **/var/run** directory to record the identifier of the active process in a unix-like operating system. |
| **index** | An index HTML file that is opened by default by the server when a client's root request occurs. |
| **favicon** | Address to the site icon file. |
| **origin** | Website address that the client must send to the server in the **Origin** HTTP header. |
| **formatDate** | Format of the date and time displayed when logging processes on the server. |
