## 说明

- 实现gethostbyname、getaddrinfo返回虚拟IP
- 当前版本只支持32位系统
- 支持XP以上系统

## 编译

vs2015_xp



## gethostbyname



## getaddrinfo

解决了自己的NSP顺序靠后的问题，选择性跳过了系统的NSP调用。