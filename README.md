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
