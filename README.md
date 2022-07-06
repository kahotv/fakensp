## 说明

- 实现gethostbyname、getaddrinfo返回虚拟IP

  （系统ping用的getaddrinfo）

- 暂时只支持32位程序

- 支持包括XP以上系统

## 编译

vs2015_xp



## gethostbyname



## getaddrinfo

解决了自己的NSP顺序靠后的问题，选择性跳过了系统的NSP调用。



## 参考

- mDNSResponder-878.70.2

  https://opensource.apple.com/tarballs/mDNSResponder/

- PaperAirplane

  https://github.com/liulilittle/PaperAirplane

- ReactOS

  https://github.com/mirror/reactos