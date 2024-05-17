#include <SFML/Graphics.hpp>

#define gameRows 21
#define gameCols 45
#define gridLength 40
#define gameWidth 1920
#define gameHeight 1080
#define acceleration 8

// Pacman spawns at predefined location grid[part][16][22]

// Static
#define freeBox 1 // 00 00 00
#define wallBox 2 // FF FF FF
#define barrBox 32 // 00 FF FF
#define spwnBox 64 // 12 34 56
#define doorBox 128 // 12 45 78
#define mnonBox 256 // 13 46 79

// For pathfinding, not shown
#define pcmnBox 1024 // 0000 0000 0000 0000 0000 0100 0000 0000
// 0x00000400

// Animated
#define teleBox 4 // TargetX TargetY FF
#define foodBox 8 // No color on maze, Spawns in-level
#define frutBox 16 // No color on maze, Spawns in-level at predefined location grid[part][17][22]
#define suprBox 512 // AA FF FF

#define pacID 0
#define ghostID 1
#define pBoxID 2
#define minionID 3