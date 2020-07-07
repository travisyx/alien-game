# AI-lien (06/2020)

## By Travis Xiang, James Bowden, and Wei Foo

*Written from scratch as a final project for Caltech's CS3 (Software Design course). The Caltech Honor Code applies; please do not consult/use this repo if you are taking the course or will be taking the course in the future. Inspired in part by Alien Isolation.*

### Hide, save up your money, and avoid the AI-lien for long enough to escape the deserted town! But beware: it will track you down using A*! 

* How to run (works on Linux and MacOS, Windows will need VM):
	* Clone repository.
	* Install SDL 2. 
	* Install SDL TTF.
	* Install SDL IMG.
	* Make the files using 'make clean all'.
	* Run the game with 'make run’. You can keep using this command and do not need to remake.
	* You can change certain parameters: stamina, player velocity, # bullets, easy/med/hard stalk radius (demo/game.c); alien velocity, delay time (alien.c); prices, # coins and hiding spots spawned (map.c). If you change parameters, 'make clean all' again before you 'make run' again.

* Controls: 
	* Arrow keys to move
	* WASD to shoot

* Rules:
	* You have limited stamina. There is an alien hunting you. Your stamina will decrease while moving and increase if you stay still.
	* Hiding (in a hiding spot) will regenerate stamina faster. In addition, the alien cannot see you while you're hiding.
	* Hiding spots can be bought for a certain amount of money. Once you've bought a hiding spot, you can hide in it again at no additional cost.
	* Doors cost money. Save up enough money to open one of the doors and you can escape!
	* If you’re behind a wall or hiding spot, the alien’s path of vision will be blocked.
	* You have a few bullets that can be used to knock back the alien in emergency situations. Don't count on them though...

* What's inside:
	* Everything is written from scratch in C using only the included libraries and SDL (for graphics rendering).
	* All components were tested as they were developed. Unfortunately, many tests for earlier components have been removed, but you can see an example in ‘tests/test_suite_forces.c’.
	* Map is initialized as 100 by 100 grid with backing array that contains various "objects" in it. 
	* Objects, text, and images are rendered using SDL 2 (incl. TTF, IMG packages). 
	* Alien navigates the map using A*, which takes advantage of a priority queue struct.
	* Collisions are detected using a combination of a bounding box method to filter out possible hits and then a more expensive separating axis method to confirm and determine bounce angle.
	* Abstraction: Map built from scene (collection of bodies, built off of polygon, built off of vectors and lists), objects (abstraction on bodies to make more usable for game), and nodes (abstraction on objects to make suitable for pathfinding). Ailien is built off of map, objects, nodes, and sorted list (priority queue, used for A*). Forces built to work with scene, and collisions implemented as instantaneous impulses inside of the force set up.

