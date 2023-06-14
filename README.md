# Ucraft
A minecraft server implementation written in C for embedded devices
### Building
The server was built and tested on a linux machine. Windows is not (yet) supported
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
### Credits
- Thanks to [Bixilon][1] who has helped with major parts of the minecraft protocol
- [wiki.vg][2] for documenting Minecraft's protocol

[1]:https://bixilon.de/en
[2]:https://wiki.vg/Main_Page
