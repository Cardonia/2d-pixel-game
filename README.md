# 2D Zombie Wave Game

A simple 2D top-down pixel game made in C++ using SFML.

## Game Info
- Square map
- Zombies spawn randomly
- Zombie count increases with wave level
- Player can move with:
  - WASD + Space to attack
  - PS4 controller
- Player uses a sword to fight
- Simple typing text dialog
- Basic cutscenes

## Tech
- C++
- SFML

## Repo Content
- Source code
- Precompiled builds:
  - Windows
  - Linux

### Gameplay Video
[https://youtube.com/your-video](https://youtu.be/ou0TuQiz7oY?si=s8l9TOYpB6xmpMkN)

## How to Compile (Linux)
- Download SFML 2.6.2 source code from their website 
- Compile SFML on your hardware
Make a new folder in the source code folder and open it 
open terminal and run this 2 commnad `cmake ..` and `make`
this will compile sfml and create important 2 folder (lib) , (include)
- Copy these folders into the game folder:
  - lib
  - include
- Compile the game using this commands `g++ main.cpp -o Game -I./include -L./lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio`
