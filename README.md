2048-multiplayer
================



在终端进行编译

> $ g++ -o 2048.out 2048multiplayer.cpp -std=c++11

游戏开始、单机和多用户
> $ 2048.out

To play in multiplayer mode, first start the *server* using the command:

> $ 2048.out server

Now to play the game, start a client using the command

> $ 2048.out client

注册模块

This command will ask for a *username*, and hence you are required to enter a valid username.

Next, it will ask the *Server IP/Host* to connect, and you are required to enter a host you want to connect to and start playing the game.
You can get to know your host by running command
> $ ifconfig
Alternatively, you can enter 127.0.0.1 to play on localhost.
