# Ucraft
A minecraft server implementation written in C for machines with limited resources.
The current server supports **1.21.4** for the client.

NOTE: This is a weekend project so i do not recommend using this for production as this is guranteed to have bugs and the general source code quality lacks in consistency. 

### Building
The server was built and tested on a linux machine. To build on a Windows machine, MSVC is required.
```
$ sudo apt install git build-essential cmake make
$ git clone https://github.com/vimpop/UCraft/
$ cd ~/UCraft && mkdir build && cd build 
$ cmake ..
$ make
```
In the ```build/src``` folder an executable by the name of ```UCraft``` will be produced that can be ran.
```
$ ./src/UCraft
[INFO]: Listening on *:25565
[INFO]: UCraft server started!
```
### Benchmarks
Comparing this server to the vanilla server is unfair because it lacks most, if not all, features of the vanilla server. It only has the basic primitives for a simple minigame like [TNT run][1] which is included in the code. But if you really want them in a **amd64** environment:
- The server binary size is approximately 46K bytes without authentication and 90K bytes with the authentication library. Memory usage varies based on the number of active players. In the worst-case scenario with 10 players, heap usage will be around 70K bytes with authentication and 20K bytes without authentication.
- The maximum clients this server can handle is ```FD_SETSIZE``` which is 1024 clients in most machines but has not been tested.


### Credits
- Thanks to [Bixilon][2] who has helped with major parts of the minecraft protocol
- [wiki.vg][3] for documenting Minecraft's protocol

### Screenshot
| ![][4] | 
|:--:| 
| *three players playing the game on the server* |

[1]:https://hypixel.fandom.com/wiki/TNT_Run
[2]:https://bixilon.de/en
[3]:https://wiki.vg/Main_Page
[4]:arena.png
