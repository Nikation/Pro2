# Pro2
项目2，游戏服务器
此项目实现游戏服务器，客户端，登陆器

开发环境：
          服务器端OS：Ubuntu 1804.1
          MySQL5.7
          Server version: 5.7.27-0ubuntu0.18.04.1 (Ubuntu)
          
          nginx
          nginx version: nginx/1.13.12
          built by gcc 7.4.0 (Ubuntu 7.4.0-1ubuntu1~18.04.1) 
          configure arguments: --with-pcre=../pcre-8.42 --with-zlib=../zlib-1.2.11 --with-openssl=../openssl-1.0.2o
          
          docker
          Docker version 19.03.4, build 9013bf583a
          
          FCgi
          spawn-fcgi v1.6.5 - spawns FastCGI processes
          
          
          客户端OS：Microsoft Windows [版本 10.0.18362.476]
          QT:Qt Creator 4.9.1  Based on Qt 5.12.3 (MSVC 2017, 32 bit)
          VS:VS2017社区版
          unity3d:2017.4.34f1 Personal
          
          
项目概述：这个项目使用unity3d作为游戏引擎，客户端分为登陆器和游戏客户端，客户端进行登陆，登陆器连接服务器进行验证当前的用户是否合法，
如果合法服务端开启一给游戏服务器容器，将容器的端口返回给客户端，登陆客户端通过端口好开启游戏客户端进入游戏。


使用nginx做游戏服务器端的代理服务器

使用FastCgi进行管理游戏服务器容器的创建


