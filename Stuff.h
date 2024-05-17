#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stack>
#include <string>
#include <queue>

#include "Definitions.cpp"

extern bool isSuper; // Use ID to differentiate between Pacman and Ghosts
extern sf::Clock superClock;
extern int foodID;
extern const int minionLimit;
extern bool startShock;

#define backCode 20
#define quitCode 30
#define nothingCode 42
#define resumeCode 50
#define toMainCode 60

#define mapWidth gridLength * gameCols
#define mapHeight gridLength * gameRows
#define mapXgap (gameWidth - mapWidth) / 2
#define mapYgap (gameHeight - mapHeight) / 2

struct Grid;

struct MovingEntity {
    int id;
    float x;
    float y;
    sf::Texture texture;
    sf::Sprite sprite;
    float speed;
    float threshold;
    char dir;
    char tempDir;
    bool hasChangedDir;
    sf::Clock teleportCooldown;
    bool pacmanAte;
    bool ghostLoose;

    MovingEntity();
    int gridX();
    int gridPosX();
    int gridY();
    int gridPosY();
    bool alignedToRow();
    bool alignedToCol();
    void moveEntityRandDir(Grid*** grid, int currentPart, float dT);
    void resetPos();
    ~MovingEntity() {};
};

struct Level;

struct Pacman : public MovingEntity {
    int score;
    int lives;
    int pelletsEaten;
    sf::CircleShape minionRadius;
    int frame;
    sf::Clock frameClock;
    int shockFrame;
    sf::Clock shockClock;
    sf::Texture shockTexture;
    sf::Sprite shockSprite;

    Pacman();
    void handleInput(Level* level, float dT);
    char deadEnd(Grid*** grid, int currentPart);
    void handleInput2(Level* level, float dT);
    bool checkCollision(MovingEntity* entity);
    void morb(MovingEntity* entity, int morbCount);
};

struct Minion : public MovingEntity {
    bool awake;
    Minion* next;

    Minion();
    void animate();
    bool checkRadius(Pacman* pacman);
    void moveMinion(float x, float y);
    ~Minion() {}
};

struct Ghost : public MovingEntity {
    sf::Color c;
    int currentFrame;
    Minion* firstMinion;
    sf::Vector2f* coordinateQueue;
    int coordinateQueueSize;
    int currentIndex;
    bool isMorbed;
    sf::Sound eatingGhost;
    sf::Clock frameClock;

    Ghost();
    void addMinion(Minion* minion);
    void setMinionColor(sf::Color c);
    char deadEnd(Grid*** grid, int currentPart);
    void moveGhost(Level* level, float dT);
    void moveGhost2(Level* level, float dT);
    void resetCoordinateQueue();
    int minionCount();
};

struct PelletBox : public MovingEntity {
    float spawnDelay;
    bool** visited;
    bool visitedInitialized;
    int pelletsProduced;

    PelletBox();
    void initializeVisited(Grid*** grid, int currentPart);
    void reset();
    char deadEnd(Grid*** grid, int currentPart);
    bool findValidTeleport(Grid*** grid, int currentPart);
    void moveEntityRandDir(Grid*** grid, int currentPart, float dT);
    void movePelletBox(Grid*** grid, int currentPart, float dT);
    ~PelletBox();
};

struct sfmlText {
    sf::Text text;
    sf::Font font;

    sfmlText()
    {
        font.loadFromFile("Font/SpaceMono-Bold.ttf");
        text.setFont(font);
        text.setCharacterSize(36);
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(2);
    }

};

struct MenuNode {
    MenuNode* up;
    MenuNode* down;
    MenuNode* left;
    MenuNode* right;
    sf::Texture nodeTexture;
    sf::Sprite nodeSprite;
    sf::RectangleShape ron;
    char transitionStatus;
    sf::Clock transitionClock;
    int executionID;

    MenuNode(std::string str, std::string textureAddress, int executionID);
    void transition();
};

struct Menu {
    MenuNode* root;
    MenuNode** startGame;
    int levelCount; // Indexed from 0 for Arcade Mode, so it's upto AND INCLUDING this value.
    MenuNode* current;
    MenuNode* gameOver;
    MenuNode* pauseRoot;
    MenuNode* levelComplete;
    MenuNode* resume;
    std::stack<MenuNode*> nodeList;
    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    sf::RectangleShape column;

    std::string type;

    Menu(std::string id);
    void setupNodeSprite(MenuNode* ptr, float mul, float xPos, float yPos);
    MenuNode* goUp(MenuNode* current);
    MenuNode* goDown(MenuNode* current);
    MenuNode* goLeft(MenuNode* current);
    MenuNode* goRight(MenuNode* current);
    void printMenu(sf::RenderWindow& layer);
    bool isStart();
    bool isResume();
    int handleInput(sf::RenderWindow& window);
    ~Menu();
};

struct Grid {
    int x;
    int y;
    int targetX;
    int targetY;
    float velocity;
    int value;
    sf::Sprite boxSprite;
    char transitionStatus;
    sf::Clock transitionClock;
    float delayTimer;
    sf::RectangleShape whiteBox;
    int teleportX;
    int teleportY;
    int currentFrame;
    sf::Clock rectClock;

    Grid();
};

struct MinionManager {
    std::vector<std::vector<Minion*>> minionArray;
    Pacman* pacman;
    Ghost* ghosts;
    int ghostCount;
    std::queue<int> turn;
    int currentPart;
    int totalParts;
    std::queue<int> morbTurn;
    std::stack<Minion*> stuffToYeet;

    MinionManager();
    void linkToEntities(Pacman* pacman, Ghost* ghosts, int ghostCount, int totalParts);
    void addMinion(Minion* minion, float x, float y, int part);
    int transferMinion(Minion* minion);
    void updateMinions();
    void resetPart(int partToObliterate);
    void prepareToYeet(Minion* minion);
    void yeet();
    void resetMorb();
};

struct Level {
    bool isIntro;
    MinionManager minionManager;
    Pacman* pacman;
    Ghost* ghosts;
    int ghostCount;
    PelletBox* pelletBox;
    sf::Texture walls;
    Grid*** grid;
    int stage;
    int totalParts;
    int currentPart;
    int pacmanGridX;
    int pacmanGridY;
    sf::Clock releaseClock;
    int* pelletTargets;
    bool targetReached;

    Level(int stage, Pacman* pacman, Ghost* ghosts, int ghostCount, PelletBox* pelletBox);
    void pickDirection(int directions[], int& xDir, int& yDir);
    void obliteratePart(int partToObliterate);
    void prepBox(Grid* currentBox, float delayTime);
    void transition1(int partToTransitionTo); // Left | Right
    void transition2(int partToTransitionTo); // Up - Down
    void transition3(int partToTransitionTo); // TopLeft / BottomRight
    void transition4(int partToTransitionTo); // BottomRight / Top Left
    void transition5(int partToTransitionTo); // Column by Column (Left to Right)
    void transition6(int partToTransitionTo); // Column by Column (Right to Left)
    void transition7(int partToTransitionTo); // Row by Row (Top to Bottom)
    void transition8(int partToTransitionTo); // Row by Row (Bottom to Top)
    void transition9(int partToTransitionTo); // Diagonal by Diagonal (Top Left / Bottom Right)
    void transition10(int partToTransitionTo); // Diagonal by Diagonal (Bottom Right / Top Left)
    void transition11(int partToTransitionTo); // Diagonal by Diagonal (Top Right \ Bottom Left)
    void transition12(int partToTransitionTo); // Diagonal by Diagonal (Bottom Left \ Top Right)
    void transitionRandom(int partToTransitionTo);
    void animate(Grid* currentBox, float dT);
    int rectToGet(int i, int j, int k);
    void printLevel(sf::RenderTexture& layer, float dT);
    void releaseGhost();
    ~Level();
};