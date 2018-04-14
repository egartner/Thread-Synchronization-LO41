# Thread-Synchronization-LO41

The goal of this project was to experiment with thread synchronization. 
Here, we implement a railway: each train represent a thread and they must synchronize in order to avoid collision and deadlock. We consider different types of train as well as train stations, tunnels (where only one train can pass at a time) and standard parallels tracks.

# Instructions
The program can be compiled using the Makefile (typing "Make" with the right privileges). The program then expect one argument at execution which is the number of trains.
