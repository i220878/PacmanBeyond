#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

#include "Stuff.h"

MovingEntity::MovingEntity() {
    ghostLoose = false;
    dir = 'n';
    tempDir = 'n';
    hasChangedDir = false;
    id = 0;
    pacmanAte = false;
    speed = 0;
    threshold = 0;
    x = 0;
    y = 0;
}
int MovingEntity::gridX() {
    return int(x / gridLength);
}
int MovingEntity::gridPosX() {
    return gridX() * gridLength + gridLength / 2;
}
int MovingEntity::gridY() {
    return int(y / gridLength);
}
int MovingEntity::gridPosY() {
    return gridY() * gridLength + gridLength / 2;
}
bool MovingEntity::alignedToRow() {
    return abs(gridPosY() - y) < threshold;
}
bool MovingEntity::alignedToCol() {
    return abs(gridPosX() - x) < threshold;
}
void MovingEntity::moveEntityRandDir(Grid*** grid, int currentPart, float dT) {

    threshold = ((float)speed / 200) * 8;

    int gridX = this->gridX();
    int gridY = this->gridY();
    bool rowAligned = this->alignedToRow();
    bool colAligned = this->alignedToCol();

    if ((!rowAligned && colAligned) || (rowAligned && !colAligned)) {
        hasChangedDir = false;
    }

    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (rowAligned && colAligned && grid[currentPart][gridY][gridX].value == teleBox && teleportCooldown.getElapsedTime().asSeconds() > 0.5) {
            teleportCooldown.restart();
            x = grid[currentPart][gridY][gridX].teleportX;
            y = grid[currentPart][gridY][gridX].teleportY;
            return;
        }
    }
    int blockU = freeBox;
    int blockD = freeBox;
    int blockL = freeBox;
    int blockR = freeBox;
    // This next check became necessary because of seg fault on trying to change
    // direction on -1. The ghosts did that, not Pac Man.
    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (gridY > 0)
            blockU = grid[currentPart][gridY - 1][gridX].value;
        if (gridY < gameRows - 1)
            blockD = grid[currentPart][gridY + 1][gridX].value;
        if (gridX > 0)
            blockL = grid[currentPart][gridY][gridX - 1].value;
        if (gridX < gameCols - 1)
            blockR = grid[currentPart][gridY][gridX + 1].value;
    }
    if (tempDir != dir) {
        if (rowAligned && colAligned) {
            bool canChangeDir = true;
            switch (tempDir) {
            case 'u':
                if (blockU == wallBox || blockU == doorBox)
                    canChangeDir = false;
                break;
            case 'd':
                if (blockD == wallBox || blockD == doorBox)
                    canChangeDir = false;
                break;
            case 'l':
                if (blockL == wallBox || blockL == doorBox)
                    canChangeDir = false;
                break;
            case 'r':
                if (blockR == wallBox || blockR == doorBox)
                    canChangeDir = false;
                break;
            }
            if (canChangeDir) {
                dir = tempDir;
                x = gridPosX();
                y = gridPosY();
                hasChangedDir = true;
            }
        }
        else if ((rowAligned && (tempDir == 'l' || tempDir == 'r'))
            || (colAligned && (tempDir == 'u' || tempDir == 'd'))) {
            dir = tempDir;
        }
    }

    float dTMultiplier = 4;
    float distanceToMove = speed * dT * dTMultiplier;
    switch (dir) {
    case 'u':
        if (rowAligned && (blockU == wallBox || blockU == doorBox)) {
            break;
        }
        y -= distanceToMove;
        if (y < 0)
            y = mapHeight;
        break;
    case 'd':
        if (rowAligned && (blockD == wallBox || blockD == doorBox)) {
            break;
        }
        y += distanceToMove;
        if (y > mapHeight)
            y = 0;
        break;
    case 'l':
        if (colAligned && (blockL == wallBox || blockL == doorBox)) {
            break;
        }
        x -= distanceToMove;
        if (x < 0)
            x = mapWidth;
        break;
    case 'r':
        if (colAligned && (blockR == wallBox || blockR == doorBox)) {
            break;
        }
        x += distanceToMove;
        if (x > mapWidth)
            x = 0;
        break;
    }
    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (id == 0 && rowAligned && colAligned) {
            if (grid[currentPart][gridY][gridX].value == foodBox) {
                grid[currentPart][gridY][gridX].value = freeBox;
                pacmanAte = true;
                foodID = foodBox;
            }
            else if (grid[currentPart][gridY][gridX].value == frutBox) {
                grid[currentPart][gridY][gridX].value = freeBox;
                pacmanAte = true;
                foodID = frutBox;
            }
            else if(isSuper == false && grid[currentPart][gridY][gridX].value == suprBox) {
                std::cout << "Going super!\n";
                grid[currentPart][gridY][gridX].value = freeBox;
                pacmanAte = true;
                isSuper = true;
                startShock = true;
                foodID = suprBox;
                superClock.restart();
            }
        }
    }
    sprite.setPosition(x + mapXgap, y + mapYgap);
}

Pacman::Pacman() {
    this->shockFrame = 0;
    this->shockTexture.loadFromFile("Textures/shockAnim.png");
    this->shockSprite.setTexture(shockTexture);

    frame = 0;
    score = 0;
    lives = 3;
    pelletsEaten = 0;
    minionRadius.setRadius(gridLength * 3 + gridLength / 2);
    minionRadius.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 100));
    id = pacID;
    texture.loadFromFile("Textures/pacSprites.png");
    sprite.setTexture(texture);
    sprite.setOrigin(20, 20);
    speed = 100;
    threshold = 8;
    tempDir = 'n';
    dir = 'n';
    x = mapWidth / 2;
    y = (mapHeight * 3) / 4 + gridLength / 2;
    y = gridPosY();
    hasChangedDir = false;
}
void Pacman::handleInput(Level* level, float dT) {
    if (frameClock.getElapsedTime().asSeconds() > 0.0333) {
        frameClock.restart();
        frame = (frame + 1) % 3;
        sprite.setTextureRect(sf::IntRect(frame * 40, 0, 40, 40));
    }
    bool u = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool l = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    bool r = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

    if (u && !d && !l && !r)
        tempDir = 'u';
    if (!u && d && !l && !r)
        tempDir = 'd';
    if (!u && !d && l && !r)
        tempDir = 'l';
    if (!u && !d && !l && r)
        tempDir = 'r';

    this->moveEntityRandDir(level->grid, level->currentPart, dT);
    if (dir == 'u') {
        sprite.setRotation(270);
    }
    else if (dir == 'd') {
        sprite.setRotation(90);
    }
    else if (dir == 'l') {
        sprite.setRotation(180);
    }
    else if (dir == 'r') {
        sprite.setRotation(0);
    }
    level->pacmanGridX = this->gridX();
    level->pacmanGridY = this->gridY();
    if (pacmanAte == true) {
        pacmanAte = false;
        switch (foodID) {
        case foodBox:
        {
            pelletsEaten++;
            score += 10;
            break;
        }
        case frutBox:
        {
            score += 1000;
            break;
        }
        case suprBox:
        {
            score += 100;
            break;
        }
        }
    }
    minionRadius.setPosition(this->x - gridLength * 2, this->y - gridLength / 2);
}

char Pacman::deadEnd(Grid*** grid, int currentPart) {
    int gridX = this->gridX();
    int gridY = this->gridY();
    bool rowAligned = this->alignedToRow();
    bool colAligned = this->alignedToCol();
    if (!(rowAligned && colAligned))
        return '_';

    int blockU = freeBox;
    int blockD = freeBox;
    int blockL = freeBox;
    int blockR = freeBox;
    // This next check became necessary because of seg fault on trying to change
    // direction on -1. The ghosts did that, not Pac Man.
    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (gridY > 0)
            blockU = grid[currentPart][gridY - 1][gridX].value;
        if (gridY < gameRows - 1)
            blockD = grid[currentPart][gridY + 1][gridX].value;
        if (gridX > 0)
            blockL = grid[currentPart][gridY][gridX - 1].value;
        if (gridX < gameCols - 1)
            blockR = grid[currentPart][gridY][gridX + 1].value;
    }
    switch (dir) {
    case 'u':
    {
        if ((blockU & blockL & blockR) == wallBox)
            return 'd';
        break;
    }
    case 'd':
    {
        if ((blockD & blockL & blockR) == wallBox)
            return 'u';
        break;
    }
    case 'l':
    {
        if ((blockL & blockU & blockD) == wallBox)
            return 'r';
        break;
    }
    case 'r':
    {
        if ((blockR & blockU & blockD) == wallBox)
            return 'l';
        break;
    }
    }
    return '_';
}
void Pacman::handleInput2(Level* level, float dT) {
    if (frameClock.getElapsedTime().asSeconds() > 0.0333) {
        frameClock.restart();
        frame = (frame + 1) % 3;
        sprite.setTextureRect(sf::IntRect(frame * 40, 0, 40, 40));
    }
    bool u = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    bool d = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    bool l = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    bool r = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

    char deadEndCheck = deadEnd(level->grid, level->currentPart);
    if (deadEndCheck == '_') {
        if (!hasChangedDir) {
            do {
                int dirRand = rand() % 4;
                switch (dirRand) {
                case 0:
                    tempDir = 'u';
                    break;
                case 1:
                    tempDir = 'd';
                    break;
                case 2:
                    tempDir = 'l';
                    break;
                case 3:
                    tempDir = 'r';
                    break;
                }
            } while ((tempDir == 'u' && dir == 'd') ||
                (tempDir == 'd' && dir == 'u') ||
                (tempDir == 'l' && dir == 'r') ||
                (tempDir == 'r' && dir == 'l'));
        }
    }
    else {
        tempDir = deadEndCheck;
    }
    this->moveEntityRandDir(level->grid, level->currentPart, dT);

    if (dir == 'u') {
        sprite.setRotation(270);
    }
    else if (dir == 'd') {
        sprite.setRotation(90);
    }
    else if (dir == 'l') {
        sprite.setRotation(180);
    }
    else if (dir == 'r') {
        sprite.setRotation(0);
    }
    level->pacmanGridX = this->gridX();
    level->pacmanGridY = this->gridY();
    if (pacmanAte == true) {
        pacmanAte = false;
        switch (foodID) {
        case foodBox:
        {
            pelletsEaten++;
            score += 10;
            break;
        }
        case frutBox:
        {
            score += 1000;
            break;
        }
        case suprBox:
        {
            score += 100;
            break;
        }
        }
    }
    minionRadius.setPosition(this->x - gridLength * 2, this->y - gridLength / 2);
}
bool Pacman::checkCollision(MovingEntity* entity) {
    if (pow(pow(this->x - entity->x, 2) + pow(this->y - entity->y, 2), 0.5) < (float)gridLength / 3)
        return true;
    else
        return false;
}
void Pacman::morb(MovingEntity* entity, int morbCount) {
    if (entity == nullptr)
        return;

    float angle = 90 + atan(float(entity->y - this->y) / float(entity->x - this->x)) * (180 / 3.1416);
    if (entity->x - this->x < 0) {
        angle += 180;
    }
    sprite.setRotation(angle - 90);
    this->x = entity->x;
    this->y = entity->y;
    if (morbCount == 0)
        score += 2500;
    score += morbCount * 50;
    sprite.setPosition(x + mapXgap, y + mapYgap);
}

Minion::Minion() {
    awake = false;
    next = nullptr;
    id = minionID;
    texture.loadFromFile("Textures/minion.png");
    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    sprite.setScale(sf::Vector2f(gridLength / (float)texture.getSize().x * 0.8, gridLength / (float)texture.getSize().y * 0.8));
    speed = 0;
    threshold = 8;
    tempDir = 'n';
    dir = 'n';
    x = mapWidth / 2;
    y = mapHeight / 4;
    y = gridPosY();
}

void Minion::animate() {
    if (awake) {

    }
    else {

    }
}

bool Minion::checkRadius(Pacman* pacman) {
    if (pow(pow(this->x - pacman->x, 2) + pow(this->y - pacman->y, 2), 0.5) < (float)gridLength * 3.1)
        return true;
    else
        return false;
}

void Minion::moveMinion(float x, float y) {
    this->x = x;
    this->y = y;
    this->sprite.setPosition(this->x + mapXgap, this->y + mapYgap);
}

Ghost::Ghost() {
    currentFrame = 0;
    isMorbed = false;
    coordinateQueueSize = 10000;
    coordinateQueue = new sf::Vector2f[coordinateQueueSize]{};
    currentIndex = 0;
    id = ghostID;
    texture.loadFromFile("Textures/ghostSprites.png");
    sprite.setTexture(texture);
    sprite.setOrigin(20, 20);
    speed = 50;
    threshold = 8;
    tempDir = 'u';
    dir = 'u';
    x = mapWidth / 2;
    y = mapHeight / 2;
    y = gridPosY();
    hasChangedDir = false;
    firstMinion = nullptr;
}

void Ghost::addMinion(Minion* minion) {
    minion->next = firstMinion;
    firstMinion = minion;
    sf::Color c2 = this->c;
    c2.a = 150;
    minion->sprite.setColor(c2);
    Minion* ptr = firstMinion;
    coordinateQueue[currentIndex].x = this->x;
    coordinateQueue[currentIndex].y = this->y;
    currentIndex++;
    currentIndex %= coordinateQueueSize;

    int i = 1;
    int location = currentIndex;

    while (ptr) {
        ptr->speed = this->speed;
        location = currentIndex - (i * 5);
        if (location < 0)
            location += coordinateQueueSize;
        ptr->moveMinion(coordinateQueue[location].x, coordinateQueue[location].y);
        ptr = ptr->next;
        i++;
    }
}

void Ghost::setMinionColor(sf::Color c) {
    Minion* traverse = firstMinion;
    while (traverse) {
        traverse->sprite.setColor(c);
        traverse = traverse->next;
    }
}

char Ghost::deadEnd(Grid*** grid, int currentPart) {
    int gridX = this->gridX();
    int gridY = this->gridY();
    bool rowAligned = this->alignedToRow();
    bool colAligned = this->alignedToCol();
    if (!(rowAligned && colAligned))
        return '_';

    int blockU = freeBox;
    int blockD = freeBox;
    int blockL = freeBox;
    int blockR = freeBox;
    // This next check became necessary because of seg fault on trying to change
    // direction on -1. The ghosts did that, not Pac Man.
    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (gridY > 0)
            blockU = grid[currentPart][gridY - 1][gridX].value;
        if (gridY < gameRows - 1)
            blockD = grid[currentPart][gridY + 1][gridX].value;
        if (gridX > 0)
            blockL = grid[currentPart][gridY][gridX - 1].value;
        if (gridX < gameCols - 1)
            blockR = grid[currentPart][gridY][gridX + 1].value;
    }
    switch (dir) {
    case 'u':
    {
        if ((blockU & blockL & blockR) == wallBox)
            return 'd';
        break;
    }
    case 'd':
    {
        if ((blockD & blockL & blockR) == wallBox)
            return 'u';
        break;
    }
    case 'l':
    {
        if ((blockL & blockU & blockD) == wallBox)
            return 'r';
        break;
    }
    case 'r':
    {
        if ((blockR & blockU & blockD) == wallBox)
            return 'l';
        break;
    }
    }
    return '_';
}
void Ghost::moveGhost(Level* level, float dT) {
    char deadEndCheck = deadEnd(level->grid, level->currentPart);
    if (deadEndCheck == '_') {
        if (!hasChangedDir) {
            do {
                int dirRand = rand() % 4;
                switch (dirRand) {
                case 0:
                    tempDir = 'u';
                    break;
                case 1:
                    tempDir = 'd';
                    break;
                case 2:
                    tempDir = 'l';
                    break;
                case 3:
                    tempDir = 'r';
                    break;
                }
            } while ((tempDir == 'u' && dir == 'd') ||
                (tempDir == 'd' && dir == 'u') ||
                (tempDir == 'l' && dir == 'r') ||
                (tempDir == 'r' && dir == 'l'));
        }
    }
    else {
        tempDir = deadEndCheck;
    }
    this->moveEntityRandDir(level->grid, level->currentPart, dT);
    Minion* ptr = firstMinion;
    coordinateQueue[currentIndex].x = this->x;
    coordinateQueue[currentIndex].y = this->y;
    currentIndex++;
    currentIndex %= coordinateQueueSize;

    int i = 1;
    int location = currentIndex;

    while (ptr) {
        ptr->speed = this->speed;
        location = currentIndex - (i * 20);
        if (location < 0)
            location += coordinateQueueSize;
        ptr->moveMinion(coordinateQueue[location].x, coordinateQueue[location].y);
        ptr = ptr->next;
        i++;
    }
}


void Ghost::moveGhost2(Level* level, float dT) {
    char deadEndCheck = deadEnd(level->grid, level->currentPart);
    if (deadEndCheck == '_') {
        if (!hasChangedDir) {
            // Implementing BFS for ghost movement
            std::queue <sf::Vector2i> q;
            q.push(sf::Vector2i(this->gridX(), this->gridY()));
            bool visited[gameRows][gameCols] = {};
            visited[this->gridY()][this->gridX()] = true;

            sf::Vector2i parents[gameRows][gameCols] = {};

            sf::Vector2i nextCell;

            while (!q.empty()) {
                sf::Vector2i current = q.front();
                q.pop();

                if (current == sf::Vector2i(level->pacmanGridX, level->pacmanGridY)) {
                    while (parents[current.y][current.x] != sf::Vector2i(this->gridX(), this->gridY()) && (current.x != 0 && current.y != 0)) {
                        current = parents[current.y][current.x];
                    }
                    nextCell = current;
                }
                if (current.y > 0 && !visited[current.y - 1][current.x] && level->grid[level->currentPart][current.y - 1][current.x].value != wallBox) {
                    q.push(sf::Vector2i(current.x, current.y - 1));
                    visited[current.y - 1][current.x] = true;
                    parents[current.y - 1][current.x] = current;
                }
                if (current.y < gameRows - 1 && !visited[current.y + 1][current.x] && level->grid[level->currentPart][current.y + 1][current.x].value != wallBox) {

                    q.push(sf::Vector2i(current.x, current.y + 1));
                    visited[current.y + 1][current.x] = true;
                    parents[current.y + 1][current.x] = current;
                }
                if (current.x > 0 && !visited[current.y][current.x - 1] && level->grid[level->currentPart][current.y][current.x - 1].value != wallBox) {
                    q.push(sf::Vector2i(current.x - 1, current.y));
                    visited[current.y][current.x - 1] = true;
                    parents[current.y][current.x - 1] = current;
                }
                if (current.x < gameCols - 1 && !visited[current.y][current.x + 1] && level->grid[level->currentPart][current.y][current.x + 1].value != wallBox) {
                    q.push(sf::Vector2i(current.x + 1, current.y));
                    visited[current.y][current.x + 1] = true;
                    parents[current.y][current.x + 1] = current;
                }
            }

            if (nextCell.x == this->gridX() - 1 && nextCell.y == this->gridY()) {
                tempDir = 'l';
            }
            else if (nextCell.x == this->gridX() + 1 && nextCell.y == this->gridY()) {
                tempDir = 'r';
            }
            else if (nextCell.y == this->gridY() - 1 && nextCell.x == this->gridX()) {
                tempDir = 'u';
            }
            else if (nextCell.y == this->gridY() + 1 && nextCell.x == this->gridX()) {
                tempDir = 'd';
            }
        }
    }
    else {
        tempDir = deadEndCheck;
    }

    this->moveEntityRandDir(level->grid, level->currentPart, dT);
    Minion* ptr = firstMinion;
    coordinateQueue[currentIndex].x = this->x;
    coordinateQueue[currentIndex].y = this->y;
    currentIndex++;
    currentIndex %= coordinateQueueSize;

    int i = 1;
    int location = currentIndex;

    while (ptr) {
        ptr->speed = this->speed;
        location = currentIndex - (i * 20);
        if (location < 0)
            location += coordinateQueueSize;
        ptr->moveMinion(coordinateQueue[location].x, coordinateQueue[location].y);
        ptr = ptr->next;
        i++;
    }
}

void Ghost::resetCoordinateQueue() {
    for (int i = 0; i < coordinateQueueSize; i++) {
        coordinateQueue[i].x = this->x;
        coordinateQueue[i].y = this->y;
    }
}

int Ghost::minionCount() {
    int count = 0;
    Minion* ptr = firstMinion;
    while (ptr) {
        count++;
        ptr = ptr->next;
    }
    return count;
}

PelletBox::PelletBox() {
    visitedInitialized = false;
    spawnDelay = 0;
    pelletsProduced = 0;
    visited = new bool* [gameRows];
    for (int i = 0; i < gameRows; i++)
        visited[i] = new bool[gameCols] {};
    visited[17][22] = true;

    id = pBoxID;
    texture.loadFromFile("Textures/pelletBox.png");
    sprite.setTexture(texture);
    sprite.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2);
    sprite.setScale(sf::Vector2f(gridLength / (float)texture.getSize().x, gridLength / (float)texture.getSize().y));
    speed = 200;
    threshold = 8;
    tempDir = 'n';
    dir = 'n';
    x = mapWidth / 2;
    y = mapHeight / 4;
    y = gridPosY();
    hasChangedDir = false;
}

void PelletBox::initializeVisited(Grid*** grid, int currentPart) {
    for (int j = 0; j < gameRows; j++) {
        for (int k = 0; k < gameCols; k++) {
            if (grid[currentPart][j][k].value != freeBox) {
                visited[j][k] = true;
            }
        }
    }
}

void PelletBox::reset() {
    spawnDelay = 0;
    pelletsProduced = 0;
    visitedInitialized = false;
    for (int i = 0; i < gameRows; i++) {
        for (int j = 0; j < gameCols; j++) {
            visited[i][j] = false;
        }
    }
    visited[17][22] = true;
    x = 22 * gridLength + gridLength / 2;
    y = 7 * gridLength + gridLength / 2;
    sprite.setPosition(-100, -100);
}

char PelletBox::deadEnd(Grid*** grid, int currentPart) {
    int gridX = this->gridX();
    int gridY = this->gridY();
    bool rowAligned = this->alignedToRow();
    bool colAligned = this->alignedToCol();
    if (!(rowAligned && colAligned))
        return '_';

    int blockU = freeBox;
    int blockD = freeBox;
    int blockL = freeBox;
    int blockR = freeBox;
    // This next check became necessary because of seg fault on trying to change
    // direction on -1. The ghosts did that, not Pac Man.
    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (gridY > 0)
            blockU = grid[currentPart][gridY - 1][gridX].value;
        if (gridY < gameRows - 1)
            blockD = grid[currentPart][gridY + 1][gridX].value;
        if (gridX > 0)
            blockL = grid[currentPart][gridY][gridX - 1].value;
        if (gridX < gameCols - 1)
            blockR = grid[currentPart][gridY][gridX + 1].value;
    }
    bool visitedU = false;
    bool visitedD = false;
    bool visitedL = false;
    bool visitedR = false;

    if (gridX > 0 && gridX < gameCols - 1 && gridY > 0 && gridY < gameRows - 1) {
        visitedU = visited[gridY - 1][gridX];
        visitedD = visited[gridY + 1][gridX];
        visitedL = visited[gridY][gridX - 1];
        visitedR = visited[gridY][gridX + 1];
    }

    switch (dir) {
        case 'u':
        {
            int num = blockU | blockL | blockR;
            if ((num & freeBox) == 0)
                return 'd';
            if (visitedU && visitedL && visitedR)
                return 'd';
            break;
        }
        case 'd':
        {
            int num = blockD | blockL | blockR;
            if ((num & freeBox) == 0)
                return 'u';
            if (visitedD && visitedL && visitedR)
                return 'u';
            break;
        }
        case 'l':
        {
            int num = blockL | blockU | blockD;
            if ((num & freeBox) == 0)
                return 'r';
            if (visitedL && visitedU && visitedD)
                return 'r';
            break;
        }
        case 'r':
        {
            int num = blockR | blockU | blockD;
            if ((num & freeBox) == 0)
                return 'l';
            if (visitedR && visitedU && visitedD)
                return 'l';
            break;
        }
    }
    int num = blockU | blockD | blockL | blockR;
    if ((num & freeBox) == 0)
        return 'a';
    return '_';
}

bool PelletBox::findValidTeleport(Grid*** grid, int currentPart) {
    for (int i = 1; i < gameRows - 1; i++) {
        for (int j = 1; j < gameCols - 1; j++) {
            if (grid[currentPart][i][j].value == freeBox and visited[i][j] == false) {
                this->y = i * gridLength + gridLength / 2;
                this->x = j * gridLength + gridLength / 2;
                this->tempDir = 'n';
                this->dir = 'n';
                visited[i][j] = true;
                grid[currentPart][i][j].value = foodBox;
                grid[currentPart][i][j].currentFrame = 0;
                pelletsProduced++;
                if (deadEnd(grid, currentPart) != '_') {
                    findValidTeleport(grid, currentPart);
                }
                return true;
            }
        }
    }
    return false;
}

void PelletBox::moveEntityRandDir(Grid*** grid, int currentPart, float dT) {

    threshold = ((float)speed / 200) * 8;

    int gridX = this->gridX();
    int gridY = this->gridY();
    bool rowAligned = this->alignedToRow();
    bool colAligned = this->alignedToCol();

    if ((!rowAligned && colAligned) || (rowAligned && !colAligned)) {
        hasChangedDir = false;
    }

    int blockU = freeBox;
    int blockD = freeBox;
    int blockL = freeBox;
    int blockR = freeBox;

    if (gridX >= 0 && gridX < gameCols && gridY >= 0 && gridY < gameRows) {
        if (gridY > 0)
            blockU = grid[currentPart][gridY - 1][gridX].value;
        if (gridY < gameRows - 1)
            blockD = grid[currentPart][gridY + 1][gridX].value;
        if (gridX > 0)
            blockL = grid[currentPart][gridY][gridX - 1].value;
        if (gridX < gameCols - 1)
            blockR = grid[currentPart][gridY][gridX + 1].value;
    }

    if (tempDir != dir) {
        if (rowAligned && colAligned) {
            bool canChangeDir = true;
            switch (tempDir) {
            case 'u':
                if (blockU == wallBox)
                    canChangeDir = false;
                break;
            case 'd':
                if (blockD == wallBox)
                    canChangeDir = false;
                break;
            case 'l':
                if (blockL == wallBox)
                    canChangeDir = false;
                break;
            case 'r':
                if (blockR == wallBox)
                    canChangeDir = false;
                break;
            }
            if (canChangeDir) {
                dir = tempDir;
                x = gridPosX();
                y = gridPosY();
                hasChangedDir = true;
            }
        }
        else if ((rowAligned && (tempDir == 'l' || tempDir == 'r'))
            || (colAligned && (tempDir == 'u' || tempDir == 'd'))) {
            dir = tempDir;
        }
    }
    float dTMultiplier = 4;
    float distanceToMove = speed * dT * dTMultiplier;
    switch (dir) {
    case 'u':
        if (rowAligned && gridY > 0 && visited[gridY - 1][gridX] == true) {
            break;
        }
        if (rowAligned && blockU == wallBox) {
            break;
        }
        y -= distanceToMove;
        if (y < 0)
            y = mapHeight;
        break;
    case 'd':
        if (rowAligned && gridY < gameRows - 1 && visited[gridY + 1][gridX] == true) {
            break;
        }
        if (rowAligned && blockD == wallBox) {
            break;
        }
        y += distanceToMove;
        if (y > mapHeight)
            y = 0;
        break;
    case 'l':
        if (colAligned && gridX > 0 && visited[gridY][gridX - 1] == true) {
            break;
        }
        if (colAligned && blockL == wallBox) {
            break;
        }
        x -= distanceToMove;
        if (x < 0)
            x = mapWidth;
        break;
    case 'r':
        if (colAligned && gridX < gameCols && visited[gridY][gridX + 1] == true) {
            break;
        }
        if (colAligned && blockR == wallBox) {
            break;
        }
        x += distanceToMove;
        if (x > mapWidth)
            x = 0;
        break;
    }
    sprite.setPosition(x + mapXgap, y + mapYgap);
}

void PelletBox::movePelletBox(Grid*** grid, int currentPart, float dT) {
    if (spawnDelay < 2) {
        spawnDelay += dT;
        if (!visitedInitialized) {
            this->initializeVisited(grid, currentPart);
            visitedInitialized = true;
        }
        return;
    }
    int gX = this->gridX();
    int gY = this->gridY();
    char deadEndCheck = deadEnd(grid, currentPart);
    if (deadEndCheck == '_') {
        if (!hasChangedDir) {
            if (gX > 0 && gX < gameCols - 1 && gY > 0 && gY < gameRows - 1) {
                if (grid[currentPart][gY - 1][gX].value == freeBox && visited[gY - 1][gX] == false) {
                    tempDir = 'u';
                }
                else if (grid[currentPart][gY][gX + 1].value == freeBox && visited[gY][gX + 1] == false) {
                    tempDir = 'r';
                }
                else if (grid[currentPart][gY + 1][gX].value == freeBox && visited[gY + 1][gX] == false) {
                    tempDir = 'd';
                }
                else if (grid[currentPart][gY][gX - 1].value == freeBox && visited[gY][gX - 1] == false) {
                    tempDir = 'l';
                }
            }
        }
    }
    else {
        findValidTeleport(grid, currentPart);
    }
    PelletBox::moveEntityRandDir(grid, currentPart, dT);
    if (gX >= 0 && gX < gameCols && gY >= 0 && gY < gameRows) {
        if (this->alignedToRow() && this->alignedToCol() && visited[gY][gX] == false && grid[currentPart][gY][gX].value == freeBox) {
            visited[gY][gX] = true;
            pelletsProduced++;
            grid[currentPart][gY][gX].value = foodBox;
            grid[currentPart][gY][gX].currentFrame = 0;
        }
    }
}

PelletBox::~PelletBox() {
    for (int i = 0; i < gameRows; i++) {
        delete[] visited[i];
    }
    delete[] visited;
}

MenuNode::MenuNode(std::string str, std::string textureAddress, int executionID = -1) {
    up = nullptr;
    down = nullptr;
    left = nullptr;
    right = nullptr;
    nodeTexture.loadFromFile(textureAddress);
    nodeSprite.setTexture(nodeTexture);
    ron.setSize(sf::Vector2f(400, 50));
    ron.setFillColor(sf::Color::Transparent);
    ron.setOutlineColor(sf::Color::White);
    ron.setOutlineThickness(3);
    ron.setPosition(300, 100);
    transitionStatus = 'n';
    this->executionID = executionID;
}

void MenuNode::transition() {
    if (transitionClock.getElapsedTime().asSeconds() < 0.004)
        return;
    if (transitionStatus == 't') {
        transitionClock.restart();
        sf::Color c = ron.getFillColor();
        if (c == sf::Color::White) {
            transitionStatus = 'n';
            return;
        }
        c.r += 5;
        c.g += 5;
        c.b += 5;
        c.a += 5;
        ron.setFillColor(c);
    }
    else if (transitionStatus == 'f') {
        transitionClock.restart();
        sf::Color c = ron.getFillColor();
        if (c == sf::Color::Transparent) {
            transitionStatus = 'n';
            return;
        }
        c.r -= 5;
        c.g -= 5;
        c.b -= 5;
        c.a -= 5;
        ron.setFillColor(c);
    }
}

Menu::Menu(std::string id) {
    logoTexture.loadFromFile("Textures/MenuBoxes/logo.png");
    logoSprite.setTexture(logoTexture);
    column.setSize(sf::Vector2f(500, gameHeight));
    column.setPosition(170, 0);
    column.setFillColor(sf::Color(0x00, 0x00, 0x00, 220));

    MenuNode* playMain = new MenuNode("Play", "Textures/MenuBoxes/play.png");
    MenuNode* options = new MenuNode("Options", "Textures/MenuBoxes/options.png");
    MenuNode* quit = new MenuNode("Quit", "Textures/MenuBoxes/quit.png");

    MenuNode* story = new MenuNode("Story Mode", "Textures/MenuBoxes/storymode.png");
    MenuNode* arcade = new MenuNode("Arcade Mode", "Textures/MenuBoxes/arcademode.png");
    MenuNode* back1 = new MenuNode("Back","Textures/MenuBoxes/back.png", backCode);

    MenuNode* musicVol = new MenuNode("Music Volume", "Textures/MenuBoxes/musicvol.png");
    MenuNode* soundVol = new MenuNode("Sound Volume", "Textures/MenuBoxes/soundvol.png");
    MenuNode* back2 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);

    MenuNode* back3 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Settings in Arcade Mode
    MenuNode* start1 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 10);

    MenuNode* newgame = new MenuNode("New Game", "Textures/MenuBoxes/newgame.png");
    MenuNode* loadgame = new MenuNode("Load Game", "Textures/MenuBoxes/loadgame.png");
    MenuNode* back4 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);

    // ToDo: List Of Saved Games
    MenuNode* back5 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);

    MenuNode* stage0 = new MenuNode("Tutorial", "Textures/MenuBoxes/tutorial.png");
    MenuNode* stage1 = new MenuNode("Stage 1", "Textures/MenuBoxes/stage1.png");
    MenuNode* stage2 = new MenuNode("Stage 2", "Textures/MenuBoxes/stage2.png");
    MenuNode* stage3 = new MenuNode("Stage 3", "Textures/MenuBoxes/stage3.png");
    MenuNode* stage4 = new MenuNode("Stage 4", "Textures/MenuBoxes/stage4.png");
    MenuNode* stage5 = new MenuNode("Boss 1", "Textures/MenuBoxes/bossstage1.png");
    MenuNode* stage6 = new MenuNode("Boss 2", "Textures/MenuBoxes/bossstage2.png");
    MenuNode* stage7 = new MenuNode("Boss 3", "Textures/MenuBoxes/bossstage3.png");
    MenuNode* stage8 = new MenuNode("Boss 4", "Textures/MenuBoxes/bossstage4.png");
    MenuNode* stage9 = new MenuNode("Super Boss", "Textures/MenuBoxes/superboss.png");
    MenuNode* back6 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);

    MenuNode* backS0 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS0 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 0);

    MenuNode* backS1 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS1 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 1);

    MenuNode* backS2 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS2 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 2);

    MenuNode* backS3 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS3 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 3);

    MenuNode* backS4 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS4 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 4);

    MenuNode* backS5 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS5 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 5);

    MenuNode* backS6 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS6 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 6);

    MenuNode* backS7 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS7 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 7);

    MenuNode* backS8 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS8 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 8);

    MenuNode* backS9 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);
    // ToDo: Level Showcase Screen
    MenuNode* startS9 = new MenuNode("Start", "Textures/MenuBoxes/start.png", 9);

    MenuNode* yes = new MenuNode("Yes", "Textures/MenuBoxes/yes.png", quitCode);
    MenuNode* no = new MenuNode("No", "Textures/MenuBoxes/no.png", backCode);

    gameOver = new MenuNode("Game Over.", "Textures/MenuBoxes/gameover.png");
    gameOver->down = new MenuNode("Main Menu", "Textures/MenuBoxes/mainmenu.png");
    gameOver->down->up = gameOver;
    gameOver->down->right = playMain;

    levelComplete = new MenuNode("Level Complete!", "Textures/MenuBoxes/levelcomplete.png");
    levelComplete->down = new MenuNode("Main Menu", "Textures/MenuBoxes/mainmenu.png");
    levelComplete->down->up = levelComplete;
    levelComplete->down->right = playMain;

    playMain->down = options;
    playMain->right = story;
    options->up = playMain;
    options->down = quit;
    options->right = musicVol;
    quit->up = options;
    quit->right = no;

    no->down = yes;
    no->left = quit;
    yes->up = no;
    yes->left = quit;

    story->down = arcade;
    story->left = playMain;
    story->right = newgame;
    arcade->up = story;
    arcade->down = back1;
    arcade->left = playMain;
    arcade->right = back3;
    back1->up = arcade;
    back1->left = playMain;

    musicVol->down = soundVol;
    musicVol->left = options;
    soundVol->up = musicVol;
    soundVol->down = back2;
    soundVol->left = options;
    back2->up = soundVol;
    back2->left = options;

    startGame = new MenuNode * [11] {};

    // ToDo: Level Settings in Arcade Mode
    back3->down = start1;
    back3->left = arcade;
    start1->up = back3;
    start1->left = arcade;
    startGame[10] = start1;

    newgame->down = loadgame;
    newgame->left = story;
    newgame->right = stage0;
    loadgame->up = newgame;
    loadgame->down = back4;
    loadgame->left = story;
    // ToDo: List of saved games
    loadgame->right = back5;
    back4->up = loadgame;
    back4->left = story;

    back5->left = loadgame;

    MenuNode* arr[] = {
        stage0,
        stage1,
        stage2,
        stage3,
        stage4,
        stage5,
        stage6,
        stage7,
        stage8,
        stage9,
        back6 };
    MenuNode* arr2[] = {
        backS0,
        backS1,
        backS2,
        backS3,
        backS4,
        backS5,
        backS6,
        backS7,
        backS8,
        backS9 };
    MenuNode* arr3[] = {
        startS0,
        startS1,
        startS2,
        startS3,
        startS4,
        startS5,
        startS6,
        startS7,
        startS8,
        startS9 };

    levelCount = 9;

    for (int i = 0; i <= levelCount; i++) {
        MenuNode* ptr = arr[i];
        if (ptr == nullptr)
            continue;

        if (i != 0)
            ptr->up = arr[i - 1];
        ptr->down = arr[i + 1];
        ptr->left = newgame;
        ptr->right = arr2[i];
        arr2[i]->left = ptr;
        arr2[i]->down = arr3[i];
        arr3[i]->up = arr2[i];
        arr3[i]->left = ptr;
        startGame[i] = arr3[i];
    }
    back6->up = stage9;
    back6->left = newgame;

    root = playMain;
    root->transitionStatus = 't';
    current = root;

    MenuNode* arr4[] = {
        playMain->down,
        story->down,
        musicVol->down,
        back3->down,
        newgame->down,
        stage0->down,
        backS0->down,
        backS1->down,
        backS2->down,
        backS3->down,
        backS4->down,
        backS5->down,
        backS6->down,
        backS7->down,
        backS8->down,
        backS9->down,
        no->down,
        gameOver->down,
        levelComplete->down };

    nodeList.push(playMain);
    nodeList.push(options);
    nodeList.push(quit);
    nodeList.push(story);
    nodeList.push(arcade);
    nodeList.push(back1);
    nodeList.push(musicVol);
    nodeList.push(soundVol);
    nodeList.push(back2);
    nodeList.push(back3);
    nodeList.push(start1);
    nodeList.push(newgame);
    nodeList.push(loadgame);
    nodeList.push(back4);
    nodeList.push(back5);
    nodeList.push(stage0);
    nodeList.push(stage1);
    nodeList.push(stage2);
    nodeList.push(stage3);
    nodeList.push(stage4);
    nodeList.push(stage5);
    nodeList.push(stage6);
    nodeList.push(stage7);
    nodeList.push(stage8);
    nodeList.push(stage9);
    nodeList.push(back6);
    nodeList.push(backS0);
    nodeList.push(backS1);
    nodeList.push(backS2);
    nodeList.push(backS3);
    nodeList.push(backS4);
    nodeList.push(backS5);
    nodeList.push(backS6);
    nodeList.push(backS7);
    nodeList.push(backS8);
    nodeList.push(backS9);
    nodeList.push(startS0);
    nodeList.push(startS1);
    nodeList.push(startS2);
    nodeList.push(startS3);
    nodeList.push(startS4);
    nodeList.push(startS5);
    nodeList.push(startS6);
    nodeList.push(startS7);
    nodeList.push(startS8);
    nodeList.push(startS9);
    nodeList.push(yes);
    nodeList.push(no);
    nodeList.push(gameOver);
    nodeList.push(levelComplete);
    nodeList.push(gameOver->down);
    nodeList.push(levelComplete->down);

    float xVal = 420;
    MenuNode* ptr = nullptr;
    float mul = 1.4;
    ptr = playMain;
    setupNodeSprite(ptr, mul, xVal, 380);
    ptr = options;
    setupNodeSprite(ptr, mul, xVal, 530);
    ptr = quit;
    setupNodeSprite(ptr, mul, xVal, 660);

    ptr = story;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = arcade;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = back1;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = musicVol;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = soundVol;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = back2;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = back3;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = start1;
    setupNodeSprite(ptr, mul, xVal, 500);

    ptr = newgame;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = loadgame;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = back4;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = back5;
    setupNodeSprite(ptr, mul, xVal, 500);

    ptr = stage0;
    setupNodeSprite(ptr, mul, xVal, 300);
    ptr = stage1;
    setupNodeSprite(ptr, mul, xVal, 360);
    ptr = stage2;
    setupNodeSprite(ptr, mul, xVal, 420);
    ptr = stage3;
    setupNodeSprite(ptr, mul, xVal, 480);
    ptr = stage4;
    setupNodeSprite(ptr, mul, xVal, 540);
    ptr = stage5;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = stage6;
    setupNodeSprite(ptr, mul, xVal, 660);
    ptr = stage7;
    setupNodeSprite(ptr, mul, xVal, 720);
    ptr = stage8;
    setupNodeSprite(ptr, mul, xVal, 780);
    ptr = stage9;
    setupNodeSprite(ptr, mul, xVal, 840);
    ptr = back6;
    setupNodeSprite(ptr, mul, xVal, 960);

    ptr = backS0;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS0;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS1;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS1;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS2;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS2;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS3;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS3;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS4;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS4;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS5;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS5;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS6;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS6;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS7;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS7;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS8;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS8;
    setupNodeSprite(ptr, mul, xVal, 600);
    ptr = backS9;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = startS9;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = yes;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = no;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = gameOver;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = gameOver->down;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = levelComplete;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = levelComplete->down;
    setupNodeSprite(ptr, mul, xVal, 700);



    MenuNode* PresumeGame = new MenuNode("Resume", "Textures/MenuBoxes/resume.png", resumeCode);
    MenuNode* Poptions = new MenuNode("Options", "Textures/MenuBoxes/optionsP.png");
    MenuNode* PreturnToMain = new MenuNode("Main Menu", "Textures/MenuBoxes/mainmenu.png", toMainCode);
    MenuNode* PquitGame = new MenuNode("Quit", "Textures/MenuBoxes/quitP.png");

    MenuNode* PmusicVol = new MenuNode("Music Volume", "Textures/MenuBoxes/musicvol.png");
    MenuNode* PsoundVol = new MenuNode("Sound Volume", "Textures/MenuBoxes/soundvol.png");
    MenuNode* Pback1 = new MenuNode("Back", "Textures/MenuBoxes/back.png", backCode);

    MenuNode* Pyes = new MenuNode("Yes", "Textures/MenuBoxes/yes.png", quitCode);
    MenuNode* Pno = new MenuNode("No", "Textures/MenuBoxes/no.png", backCode);

    pauseRoot = PresumeGame;
    PresumeGame->down = Poptions;
    Poptions->up = PresumeGame;
    Poptions->down = PreturnToMain;
    Poptions->right = PmusicVol;
    PreturnToMain->up = Poptions;
    PreturnToMain->down = PquitGame;
    PreturnToMain->right = root;
    PquitGame->up = PreturnToMain;
    PquitGame->right = Pno;

    PmusicVol->down = PsoundVol;
    PmusicVol->left = Poptions;
    PsoundVol->up = PmusicVol;
    PsoundVol->down = Pback1;
    PsoundVol->left = Poptions;
    Pback1->up = PsoundVol;
    Pback1->left = Poptions;

    Pno->down = Pyes;
    Pno->left = PquitGame;
    Pyes->up = Pno;
    Pyes->left = PquitGame;

    MenuNode* arr5[] = {
        PresumeGame->down,
        PmusicVol->down,
        Pno->down
    };

    ptr = PresumeGame;
    setupNodeSprite(ptr, mul, xVal, 300);
    ptr = Poptions;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = PreturnToMain;
    setupNodeSprite(ptr, mul, xVal, 500);
    ptr = PquitGame;
    setupNodeSprite(ptr, mul, xVal, 600);

    ptr = PmusicVol;
    setupNodeSprite(ptr, mul, xVal, 300);
    ptr = PsoundVol;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = Pback1;
    setupNodeSprite(ptr, mul, xVal, 500);
    
    ptr = Pyes;
    setupNodeSprite(ptr, mul, xVal, 400);
    ptr = Pno;
    setupNodeSprite(ptr, mul, xVal, 500);

    nodeList.push(PresumeGame);
    nodeList.push(Poptions);
    nodeList.push(PreturnToMain);
    nodeList.push(PquitGame);
    nodeList.push(PmusicVol);
    nodeList.push(PsoundVol);
    nodeList.push(Pback1);
    nodeList.push(Pyes);
    nodeList.push(Pno);
}

void Menu::setupNodeSprite(MenuNode* ptr, float mul, float xPos, float yPos) {
    if (ptr == nullptr)
        return;

    ptr->nodeSprite.setOrigin(ptr->nodeTexture.getSize().x / 2, ptr->nodeTexture.getSize().y / 2);
    ptr->ron.setSize(sf::Vector2f(334, ptr->nodeTexture.getSize().y));
    ptr->ron.setScale(mul, mul);
    ptr->ron.setOrigin(sf::Vector2f(334 / 2, ptr->nodeSprite.getOrigin().y));

    ptr->nodeSprite.setPosition(xPos, yPos);
    ptr->ron.setPosition(ptr->nodeSprite.getPosition());
}

MenuNode* Menu::goUp(MenuNode* current) {
    current->transitionStatus = 'f';
    if (current->up) {
        current->up->transitionStatus = 't';
        return current->up;
    }
    else {
        while (current->down)
            current = current->down;
        current->transitionStatus = 't';
        return current;
    }
}
MenuNode* Menu::goDown(MenuNode* current) {
    current->transitionStatus = 'f';
    if (current->down) {
        current->down->transitionStatus = 't';
        return current->down;
    }
    else {
        while (current->up)
            current = current->up;
        current->transitionStatus = 't';
        return current;
    }
}
MenuNode* Menu::goLeft(MenuNode* current) {
    if (current->left) {
        current->left->transitionStatus = 't';
        MenuNode* ptr = current;
        while (ptr->up)
            ptr = ptr->up;
        while (ptr) {
            ptr->ron.setFillColor(sf::Color::Transparent);
            ptr->transitionStatus = 'n';
            ptr = ptr->down;
        }
        return current->left;
    }
    else {
        return current;
    }
}
MenuNode* Menu::goRight(MenuNode* current) {
    if (current->right) {
        MenuNode* ptr = current;
        while (ptr->up)
            ptr = ptr->up;
        while (ptr) {
            ptr->ron.setFillColor(sf::Color::Transparent);
            ptr->transitionStatus = 'n';
            ptr = ptr->down;
        }
        current->right->transitionStatus = 't';
        return current->right;
    }
    else {
        if (current->executionID >= 0 && current->executionID <= 10) { // Start Game
            return startGame[current->executionID];
        }
        else if (current->executionID == backCode) { // Back
            return goLeft(current);
        }
        else if (current->executionID == quitCode) { // Quit
            return nullptr;
        }
        else {
            return current;
        }
    }
}

bool Menu::isStart() {
    if (current == nullptr)
        return false;
    if (current->executionID == 10)
        return true;
    for (int i = 0; i <= levelCount; i++) {
        if (current == startGame[i])
            return true;
    }
    return false;
}

bool Menu::isResume() {
    if (current == nullptr)
        return false;
    if (current->executionID == resumeCode)
        return true;
    else
        return false;
}

int Menu::handleInput(sf::RenderWindow& window) {
    sf::Event e;
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Key::Up || e.key.code == sf::Keyboard::W) {
                current = goUp(current);
            }
            if (e.key.code == sf::Keyboard::Key::Down || e.key.code == sf::Keyboard::S) {
                current = goDown(current);
            }
            if (e.key.code == sf::Keyboard::Key::Left || e.key.code == sf::Keyboard::Key::Escape || e.key.code == sf::Keyboard::A) {
                current = goLeft(current);
            }
            if (e.key.code == sf::Keyboard::Key::Right || e.key.code == sf::Keyboard::Key::Enter || e.key.code == sf::Keyboard::D) {
                current = goRight(current);
                if (isStart() || isResume()) { // return stageCode
                    return current->executionID;
                }
            }
        }
        if (e.type == sf::Event::Closed || current == nullptr) {
            return quitCode;
        }
    }
    return nothingCode;
}

Menu::~Menu() {
    MenuNode* ptr = nullptr;
    while (!nodeList.empty()) {
        ptr = nodeList.top();
        nodeList.pop();
        delete ptr;
    }
}

void Menu::printMenu(sf::RenderWindow& layer) {
    layer.draw(this->column);
    layer.draw(this->logoSprite);
    MenuNode* ptr = current;
    while (ptr->up)
        ptr = ptr->up;
    while (ptr) {
        ptr->transition();
        if(current == ptr)
            layer.draw(ptr->ron);
        // layer.draw(begin->option.text);
        layer.draw(ptr->nodeSprite);
        ptr = ptr->down;
    }
}

Grid::Grid() {
    currentFrame = 0;
    delayTimer = 0;
    targetX = 0;
    targetY = 0;
    teleportX = 0;
    teleportY = 0;
    transitionStatus = 'i';
    value = 0;
    velocity = 0;
    x = 0;
    y = 0;
}

MinionManager::MinionManager() {
    currentPart = 0;
    ghostCount = 0;
    pacman = nullptr;
    totalParts = 0;
    ghosts = nullptr;
}

void MinionManager::linkToEntities(Pacman* pacman, Ghost* ghosts, int ghostCount, int totalParts) {
    this->pacman = pacman;
    this->ghosts = ghosts;
    this->ghostCount = ghostCount;
    for (int i = 0; i < ghostCount; i++) {
        turn.push(i);
    }
    minionArray.resize(totalParts); // Resizes for ALL PARTS not CURRENT PART
    this->totalParts = totalParts;
}

void MinionManager::addMinion(Minion* minion, float x, float y, int part) {
    minion->x = x + gridLength / 2;
    minion->y = y + gridLength / 2;
    minion->sprite.setPosition(minion->x + mapXgap, minion->y + mapYgap);
    minionArray[part].push_back(minion);
}

void MinionManager::updateMinions() {
    for (int i = 0; i < minionArray[currentPart].size(); i++) {
        if (minionArray[currentPart][i] == nullptr)
            continue;
        if (minionArray[currentPart][i]->checkRadius(this->pacman)) {
            std::cout << "Checking radius.\n";
            bool allGhostsMorbed = true;
            for (int i = 0; i < ghostCount; i++) {
                if (ghosts[i].isMorbed == false) {
                    allGhostsMorbed = false;
                    break;
                }
            }
            if (allGhostsMorbed) {
                std::cout << "all ghosts morbed. resetting...\n";
                for (int i = 0; i < ghostCount; i++) {
                    ghosts[i].isMorbed = false;
                    turn.push(i);
                }
            }
            if (!turn.empty()) {
                while (ghosts[turn.front()].isMorbed == true)
                    turn.pop();
            }
            std::cout << "All good chief.\n";
            int totalCount = 0;
            for (int i = 0; i < ghostCount; i++) {
                totalCount += ghosts[i].minionCount();
            }
            if (totalCount < minionLimit) {
                ghosts[turn.front()].addMinion(minionArray[currentPart][i]);
                minionArray[currentPart][i] = nullptr;
                turn.push(turn.front());
                turn.pop();
            }
            else {
                delete minionArray[currentPart][i];
                minionArray[currentPart][i] = nullptr;
            }
        }
    }
    // Sprite sheet stuff.
}

int MinionManager::transferMinion(Minion* minion) {
    for (int i = 0; i < minionArray[currentPart].size(); i++) {
        if (minionArray[currentPart][i] == minion) {
            minionArray[currentPart][i] = nullptr;
            break;
        }
    }
    if (morbTurn.empty()) {
        for (int i = 0; i < ghostCount; i++) {
            if (!ghosts[i].isMorbed)
                morbTurn.push(i);
        }
    }
    if (morbTurn.empty()) {
        for (int i = 0; i < ghostCount; i++) {
            ghosts[i].isMorbed = false;
            morbTurn.push(i);
        }
    }
    ghosts[morbTurn.front()].addMinion(minion);
    morbTurn.push(morbTurn.front());
    morbTurn.pop();
    return 0;
}

void MinionManager::resetPart(int partToObliterate) {
    minionArray[partToObliterate].clear();
    yeet();
}

void MinionManager::prepareToYeet(Minion* minion) {
    for (int i = 0; i < minionArray[currentPart].size(); i++) {
        if (minionArray[currentPart][i] == minion) {
            minionArray[currentPart][i] = nullptr;
            break;
        }
    }
    stuffToYeet.push(minion);
}

void MinionManager::yeet() {
    while (!stuffToYeet.empty()) {
        delete stuffToYeet.top();
        stuffToYeet.pop();
    }
    // std::cout << "complete!\n";
}

void MinionManager::resetMorb() {
    while (!morbTurn.empty())
        morbTurn.pop();
    yeet();
}

int mazeColorLookup(const sf::Color& c) {
    if (c.r == 0x00 && c.g == 0x00 && c.b == 0x00)
        return freeBox;
    else if (c.r == 0xFF && c.g == 0xFF && c.b == 0xFF)
        return wallBox;
    else if (c.r == 0x00 && c.g == 0xFF && c.b == 0xFF)
        return barrBox;
    else if (c.r == 0x12 && c.g == 0x34 && c.b == 0x56)
        return spwnBox;
    else if (c.r == 0x12 && c.g == 0x45 && c.b == 0x78)
        return doorBox;
    else if (c.r == 0x13 && c.g == 0x46 && c.b == 0x79)
        return mnonBox;
    else if (c.r == 0xAA && c.g == 0xFF && c.b == 0xFF)
        return suprBox;
    else
        return teleBox;
}

Level::Level(int stage, Pacman* pacman, Ghost* ghosts, int ghostCount, PelletBox* pelletBox) {
    isIntro = (stage == 11);
    this->stage = stage;
    this->pacman = pacman;
    this->ghosts = ghosts;
    this->ghostCount = ghostCount;
    this->pelletBox = pelletBox;
    this->pacmanGridX = 0;
    this->pacmanGridY = 0;
    this->targetReached = false;

    if (isIntro) {
        stage = 10;
        this->stage = 10;
    }

    std::cout << "Initializing level.\n";

    int parts[] = { 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 45};
    int stageCount = 0;
    int partCount = 0;
    totalParts = parts[stage];
    this->pelletTargets = new int[totalParts] {};
    this->minionManager.linkToEntities(pacman, ghosts, ghostCount, totalParts);
    grid = new Grid * *[totalParts] {};
    walls.loadFromFile("Mazes/dynamicWalls4.png");
    for (int i = 0; i < totalParts; i++) {
        std::cout << "Initializing part " << i << std::endl;
        sf::Image image;
        if (stage == 10) {
            if (partCount >= parts[stageCount]) {
                stageCount++;
                partCount = 0;
            }
        }
        if (stage == 10) {
            image.loadFromFile("Mazes/" + std::to_string(stageCount) + "_" + std::to_string(partCount) + ".png");
            partCount++;
        }
        else {
            image.loadFromFile("Mazes/" + std::to_string(stage) + "_" + std::to_string(i) + ".png");
        }
        grid[i] = new Grid * [gameRows] {};
        for (int j = 0; j < gameRows; j++) {
            grid[i][j] = new Grid[gameCols]{};
            for (int k = 0; k < gameCols; k++) {
                sf::Color color = image.getPixel(k, j);
                Grid* currentBox = &grid[i][j][k];
                currentBox->value = mazeColorLookup(color);
                if (currentBox->value == mnonBox) {
                    minionManager.addMinion(new Minion, k * gridLength, j * gridLength, i);
                    currentBox->value = freeBox;
                }
                else if (currentBox->value == teleBox) {
                    currentBox->teleportX = color.r * gridLength + gridLength / 2;
                    currentBox->teleportY = color.g * gridLength + gridLength / 2;
                }
                else if (currentBox->value == freeBox) {
                    pelletTargets[i]++;
                    currentBox->teleportX = -1;
                    currentBox->teleportY = -1;
                }
                else {
                    currentBox->teleportX = -1;
                    currentBox->teleportY = -1;
                }
                currentBox->boxSprite.setTexture(walls);
                currentBox->transitionStatus = 'i';
                currentBox->x = -100;
                currentBox->y = -100;
                currentBox->targetX = k * gridLength;
                currentBox->targetY = j * gridLength;
                currentBox->delayTimer = -1;
                currentBox->currentFrame = 0;
            }
        }
    }
    for (int i = 0; i < totalParts; i++) {
        pelletTargets[i] *= float(50 + i) / 100;
        std::cout << "Target " << i << ": " << pelletTargets[i] << '\n';
    }
    if (isIntro)
        currentPart = 9;
    else
        currentPart = 0;
    std::cout << "Done!\n";
    minionManager.currentPart = 0;
}

void Level::obliteratePart(int partToObliterate) {
    minionManager.resetPart(partToObliterate);
    Grid* currentBox = nullptr;
    int parts[] = { 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 45 };
    int stageCount = 0;
    int partCount = 0;
    totalParts = parts[stage];
    sf::Image image;
    if (stage == 10) {
        int num = partToObliterate;
        int i = 0;
        for (i = 0; i < 10 && num > 0; i++)
            num -= parts[i];
        if (num < 0) {
            num += parts[i - 1];
            std::cout << "Resetting Mazes/" + std::to_string(i - 1) + "_" + std::to_string(num) + ".png\n";
            image.loadFromFile("Mazes/" + std::to_string(i - 1) + "_" + std::to_string(num) + ".png");
        }
        else {
            std::cout << "Resetting Mazes/" + std::to_string(i) + "_" + std::to_string(num) + ".png\n";
            image.loadFromFile("Mazes/" + std::to_string(i) + "_" + std::to_string(num) + ".png");
        }
    }
    else {
        std::cout << "Resetting Mazes/" + std::to_string(stage) + "_" + std::to_string(partToObliterate) + ".png\n";
        image.loadFromFile("Mazes/" + std::to_string(stage) + "_" + std::to_string(partToObliterate) + ".png");
    }
    int i = partToObliterate;
    for (int j = 0; j < gameRows; j++) {
        for (int k = 0; k < gameCols; k++) {
            sf::Color color = image.getPixel(k, j);
            Grid* currentBox = &grid[i][j][k];
            currentBox->value = mazeColorLookup(color);
            if (currentBox->value == mnonBox) {
                minionManager.addMinion(new Minion, k * gridLength, j * gridLength, i);
                currentBox->value = freeBox;
            }
            else if (currentBox->value == teleBox) {
                currentBox->teleportX = color.r * gridLength + gridLength / 2;
                currentBox->teleportY = color.g * gridLength + gridLength / 2;
            }
            else {
                currentBox->teleportX = -1;
                currentBox->teleportY = -1;
            }
            currentBox->x = -100;
            currentBox->y = -100;
            currentBox->delayTimer = -1;
            currentBox->transitionStatus = 'i';
            currentBox->currentFrame = 0;
        }
    }
    std::cout << "Maze Reset.\n";
}

void Level::pickDirection(int directions[], int& xDir, int& yDir) {
    int dir = rand() % 8;
    xDir = 0;
    yDir = 0;
    while (directions[dir] == 0)
        dir = rand() % 8;
    switch (dir) {
    case 0: // 0
        yDir = -gameHeight / 2;
        break;
    case 1: // 45
        xDir = gameWidth / 2;
        yDir = -gameHeight / 2;
        break;
    case 2: // 90
        xDir = gameWidth / 2;
        break;
    case 3: // 135
        xDir = gameWidth / 2;
        yDir = gameHeight / 2;
        break;
    case 4: // 180
        yDir = gameHeight / 2;
        break;
    case 5: // 225
        xDir = -gameWidth / 2;
        yDir = gameHeight / 2;
        break;
    case 6: // 270
        xDir = -gameWidth / 2;
        break;
    case 7: // 315
        xDir = -gameWidth / 2;
        yDir = -gameHeight / 2;
        break;
    }
}

void Level::prepBox(Grid* currentBox, float delayTime) {
    currentBox->x = currentBox->targetX;
    currentBox->y = currentBox->targetY;
    currentBox->transitionStatus = 'P';
    currentBox->delayTimer = delayTime;
    currentBox->whiteBox.setSize(sf::Vector2f(gridLength * 0.95, gridLength * 0.95));
    currentBox->whiteBox.setScale(0.01, 0.01);
    currentBox->whiteBox.setFillColor(sf::Color::White);
    currentBox->whiteBox.setOrigin((gridLength * 0.95) / 2, (gridLength * 0.95) / 2);
    currentBox->whiteBox.setPosition(currentBox->x + mapXgap + gridLength / 2, currentBox->y + mapYgap + gridLength / 2);
}

void Level::transition1(int partToTransitionTo) { // Left | Right
    int i = partToTransitionTo;
    int xDir = 0;
    int yDir = 0;
    int directions[] = { 1, 0, 0, 0, 1, 1, 1, 1 };
    pickDirection(directions, xDir, yDir);
    for (int j = 0; j < gameRows; j++) {
        for (int k = 0; k < gameCols / 2; k++) {
            grid[i][j][k].x = grid[i][j][k].targetX + xDir;
            grid[i][j][k].y = grid[i][j][k].targetY + yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
        if (gameCols % 2 != 0) {
            prepBox(&grid[i][j][gameCols / 2], j * 0.02 + 0.70);
        }
        for (int k = gameCols / 2 + 1; k < gameCols; k++) {
            grid[i][j][k].x = grid[i][j][k].targetX - xDir;
            grid[i][j][k].y = grid[i][j][k].targetY - yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
    }
}

void Level::transition2(int partToTransitionTo) { // Up - Down
    int i = partToTransitionTo;
    int xDir = 0;
    int yDir = 0;
    int directions[] = { 1, 1, 1, 0, 0, 0, 1, 1 };
    pickDirection(directions, xDir, yDir);
    for (int k = 0; k < gameCols; k++) {
        for (int j = 0; j < gameRows / 2; j++) {
            grid[i][j][k].x = grid[i][j][k].targetX + xDir;
            grid[i][j][k].y = grid[i][j][k].targetY + yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
        if (gameRows % 2 != 0) {
            prepBox(&grid[i][gameRows / 2][k], k * 0.02 + 0.70);
        }
        for (int j = gameRows / 2 + 1; j < gameRows; j++) {
            grid[i][j][k].x = grid[i][j][k].targetX - xDir;
            grid[i][j][k].y = grid[i][j][k].targetY - yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
    }
}

void Level::transition3(int partToTransitionTo) { // TopLeft / BottomRight
    int i = partToTransitionTo;
    int xDir = 0;
    int yDir = 0;
    int directions[] = { 1, 0, 0, 0, 0, 0, 1, 1 };
    pickDirection(directions, xDir, yDir);
    for (int j = 0; j < gameRows; j++) {
        for (int k = 0; k < int((float(gameRows - j) / gameRows) * gameCols); k++) {
            grid[i][j][k].x = grid[i][j][k].targetX + xDir;
            grid[i][j][k].y = grid[i][j][k].targetY + yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
        for (int k = int((float(gameRows - j) / gameRows) * gameCols); k < gameCols; k++) {
            grid[i][j][k].x = grid[i][j][k].targetX - xDir;
            grid[i][j][k].y = grid[i][j][k].targetY - yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
    }
}

void Level::transition4(int partToTransitionTo) { // TopRight \ BottomLeft
    int i = partToTransitionTo;
    int xDir = 0;
    int yDir = 0;
    int directions[] = { 0, 0, 0, 0, 1, 1, 1, 0 };
    pickDirection(directions, xDir, yDir);
    for (int j = 0; j < gameRows; j++) {
        for (int k = 0; k < int((float(j) / gameRows) * gameCols); k++) {
            grid[i][j][k].x = grid[i][j][k].targetX + xDir;
            grid[i][j][k].y = grid[i][j][k].targetY + yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
        for (int k = int((float(j) / gameRows) * gameCols); k < gameCols; k++) {
            grid[i][j][k].x = grid[i][j][k].targetX - xDir;
            grid[i][j][k].y = grid[i][j][k].targetY - yDir;
            grid[i][j][k].transitionStatus = 'm';
            grid[i][j][k].whiteBox.setScale(0, 0);
            grid[i][j][k].velocity = 1;
        }
    }

}

void Level::transition5(int partToTransitionTo) { // Column by Column (Left to Right)
    int i = partToTransitionTo;
    for (int k = 0; k < gameCols; k++) {
        float delayTime = k * 0.02 + 0.40;
        for (int j = 0; j < gameRows; j++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void Level::transition6(int partToTransitionTo) { // Column by Column (Right to Left)
    int i = partToTransitionTo;
    for (int k = 0; k < gameCols; k++) {
        float delayTime = (gameCols - 1 - k) * 0.02 + 0.40;
        for (int j = 0; j < gameRows; j++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void Level::transition7(int partToTransitionTo) { // Row by Row (Top to Bottom)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = j * 0.02 + 0.40;
        for (int k = 0; k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void Level::transition8(int partToTransitionTo) { // Row by Row (Bottom to Top)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = (gameRows - 1 - j) * 0.02 + 0.40;
        for (int k = 0; k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void Level::transition9(int partToTransitionTo) { // Diagonal by Diagonal (Top Left / Bottom Right)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = j * 0.02 + 0.40;
        for (int k = 0; k < int((float(gameRows - j) / gameRows) * gameCols); k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
        delayTime = (gameRows - 1 - j) * 0.02 + 0.40;
        for (int k = int((float(gameRows - j) / gameRows) * gameCols); k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}
void Level::transition10(int partToTransitionTo) { // Diagonal by Diagonal (Bottom Right / Top Left)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = j * 0.02 + 0.40;
        for (int k = 0; k < int((float(j) / gameRows) * gameCols); k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
        delayTime = (gameRows - 1 - j) * 0.02 + 0.40;
        for (int k = int((float(j) / gameRows) * gameCols); k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void Level::transition11(int partToTransitionTo) { // Diagonal by Diagonal (Top Left / Bottom Right)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = (gameRows - 1 - j) * 0.02 + 0.40;
        for (int k = 0; k < int((float(gameRows - j) / gameRows) * gameCols); k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
        delayTime = j * 0.02 + 0.40;
        for (int k = int((float(gameRows - j) / gameRows) * gameCols); k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}
void Level::transition12(int partToTransitionTo) { // Diagonal by Diagonal (Bottom Right / Top Left)
    int i = partToTransitionTo;
    for (int j = 0; j < gameRows; j++) {
        float delayTime = (gameRows - 1 - j) * 0.02 + 0.40;
        for (int k = 0; k < int((float(j) / gameRows) * gameCols); k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
        delayTime = j * 0.02 + 0.40;
        for (int k = int((float(j) / gameRows) * gameCols); k < gameCols; k++) {
            prepBox(&grid[i][j][k], delayTime);
        }
    }
}

void MovingEntity::resetPos() {
    switch (id) {
    case 0:
        x = mapWidth / 2;
        y = (mapHeight * 3) / 4 + gridLength / 2;
        y = gridPosY();
        tempDir = 'n';
        dir = 'n';
        break;
    case 1:
        x = mapWidth / 2;
        y = mapHeight / 2;
        x = gridPosX();
        y = gridPosY();
        tempDir = 'n';
        dir = 'n';
        hasChangedDir = false;
        ghostLoose = false;
        break;
    }
}

void Level::transitionRandom(int partToTransitionTo) {
    targetReached = false;
    pacman->resetPos();
    while (!minionManager.turn.empty())
        minionManager.turn.pop();
    for (int i = 0; i < ghostCount; i++) {
        ghosts[i].resetPos();
        ghosts[i].resetCoordinateQueue();
        ghosts[i].isMorbed = false;
        minionManager.turn.push(i);
    }


    ghosts[0].setMinionColor(ghosts[0].c);
    ghosts[1].setMinionColor(ghosts[1].c);
    ghosts[2].setMinionColor(ghosts[2].c);
    ghosts[3].setMinionColor(ghosts[3].c);
    for (int i = 0; i < ghostCount; i++) {
        ghosts[i].sprite.setColor(sf::Color::White);
    }
    isSuper = false;
    minionManager.resetMorb();

    pelletBox->reset();
    minionManager.currentPart = partToTransitionTo;
    pacman->pelletsEaten = 0;
    int choice = rand() % 12 + 1;
    switch (choice) {
    case 1:
        transition1(partToTransitionTo);
        break;
    case 2:
        transition2(partToTransitionTo);
        break;
    case 3:
        transition3(partToTransitionTo);
        break;
    case 4:
        transition4(partToTransitionTo);
        break;
    case 5:
        transition5(partToTransitionTo);
        break;
    case 6:
        transition6(partToTransitionTo);
        break;
    case 7:
        transition7(partToTransitionTo);
        break;
    case 8:
        transition8(partToTransitionTo);
        break;
    case 9:
        transition9(partToTransitionTo);
        break;
    case 10:
        transition10(partToTransitionTo);
        break;
    case 11:
        transition11(partToTransitionTo);
        break;
    case 12:
        transition12(partToTransitionTo);
        break;
    }
    if (choice >= 1 && choice <= 4) {
        for (int j = 0; j < gameRows; j++) {
            for (int k = 0; k < gameCols; k++) {
                if (grid[partToTransitionTo][j][k].value == teleBox)
                    prepBox(&grid[partToTransitionTo][j][k], 2);
            }
        }
    }
    for (int i = 0; i < totalParts; i++)
        grid[i][17][22].value = frutBox;
}

void Level::animate(Grid* currentBox, float dT) {
    if (currentBox->transitionStatus == 's') // Stable
        return;

    float dTMultiplier = 1000;

    float elapsedTime = currentBox->transitionClock.getElapsedTime().asSeconds();
    if (elapsedTime > 0.01666) {
        currentBox->transitionClock.restart();
        if (elapsedTime > 0.02)
            elapsedTime = 0.01666;
        // currentBox->transitionTotal += currentBox->transitionClock.getElapsedTime().asSeconds();
        switch (currentBox->transitionStatus) {
        case 'm': // Moving In
        {
            float angle = 90 + atan(float(currentBox->targetY - currentBox->y) / float(currentBox->targetX - currentBox->x)) * (180 / 3.1416);
            if (currentBox->targetX - currentBox->x < 0) {
                angle += 180;
            }
            float dispV = -cos(angle * (3.1416 / 180));
            float dispH = sin(angle * (3.1416 / 180));
            currentBox->x += dispH * currentBox->velocity;
            currentBox->y += dispV * currentBox->velocity;

            float newAngle = 90 + atan(float(currentBox->targetY - currentBox->y) / float(currentBox->targetX - currentBox->x)) * (180 / 3.1416);
            if (currentBox->targetX - currentBox->x < 0) {
                newAngle += 180;
            }

            currentBox->velocity *= 1.15;

            if (std::abs(newAngle - angle) > 1) {
                currentBox->transitionStatus = 's';
                currentBox->x = currentBox->targetX;
                currentBox->y = currentBox->targetY;
                currentBox->velocity = 0;
            }

            break;
        }
        case 'M': // Moving Out
        {
            currentBox->y += 5;
            break;
        }
        case 'f': // Fading In
        {
            sf::Color c = currentBox->boxSprite.getColor();
            c.a += 10;
            currentBox->boxSprite.setColor(c);
            if (c.a >= 250) {
                c.a = 255;
                currentBox->boxSprite.setColor(c);
                currentBox->transitionStatus = 's';
            }
            break;
        }
        case 'F': // Fading Out
        {
            sf::Color c = currentBox->boxSprite.getColor();
            c.a -= 15;
            currentBox->boxSprite.setColor(c);
            if (c.a == 0) {
                currentBox->transitionStatus = 'i';
            }
            break;
        }
        case 'P': // Popping into white box
        {
            sf::Vector2f currentScale = currentBox->whiteBox.getScale();
            currentScale.x += 0.2;
            currentScale.y += 0.2;
            if (currentScale.x >= 1 || currentScale.y >= 1) {
                currentScale.x = 1;
                currentScale.y = 1;
                currentBox->transitionStatus = 'p';
            }
            currentBox->whiteBox.setScale(currentScale);
        }
        case 'p': // Popping out of white box
        {
            if (currentBox->delayTimer > 0) {
                currentBox->delayTimer -= elapsedTime;
                currentBox->boxSprite.setPosition(currentBox->x + mapXgap, currentBox->y + mapYgap);
                return;
            }
            sf::Vector2f currentScale = currentBox->whiteBox.getScale();
            currentScale.x -= 0.04;
            currentScale.y -= 0.04;
            if (currentScale.x < 0.01 || currentScale.y < 0.01) {
                currentScale.x = 0;
                currentScale.y = 0;
                currentBox->transitionStatus = 's';
                currentBox->delayTimer = -1;
            }
            currentBox->whiteBox.setScale(currentScale);
        }
        }
    }
    currentBox->boxSprite.setPosition(currentBox->x + mapXgap, currentBox->y + mapYgap);
}

int Level::rectToGet(int i, int j, int k) {
    int calc = 0;
    int val = grid[i][j][k].value;
    if (val != wallBox) {
        return 0;
    }
    if (j == 0 || (j > 0 && grid[i][j - 1][k].value == wallBox)) {
        calc |= 1;
    }
    if (k == gameCols - 1 || (k < gameCols - 1 && grid[i][j][k + 1].value == wallBox)) {
        calc |= 2;
    }
    if (j == gameRows - 1 || (j < gameRows - 1 && grid[i][j + 1][k].value == wallBox)) {
        calc |= 4;
    }
    if (k == 0 || (k > 0 && grid[i][j][k - 1].value == wallBox)) {
        calc |= 8;
    }
    return calc;
}

void Level::printLevel(sf::RenderTexture& layer, float dT) {
    Grid* currentBox = nullptr;
    for (int i = 0; i < totalParts; i++) {
        for (int j = 0; j < gameRows; j++) {
            for (int k = 0; k < gameCols; k++) {
                currentBox = &grid[i][j][k];
                if (currentBox->transitionStatus == 'i')
                    continue;
                switch (currentBox->value) {
                    case freeBox:
                    case barrBox:
                    case spwnBox:
                    case doorBox:
                    case mnonBox:
                    case wallBox:
                    {
                        int rectToDisplace = rectToGet(i, j, k);
                        currentBox->boxSprite.setTextureRect(sf::IntRect(0, 40 * rectToDisplace, 40, 40));
                        if (j > 0 && j < gameRows - 1 && k > 0 && k < gameCols - 1) {
                            bool special = false;
                            switch (rectToDisplace) {
                                case 3:
                                {
                                    if (grid[i][j - 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 6:
                                {
                                    if (grid[i][j + 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 7:
                                {
                                    if (grid[i][j - 1][k + 1].value != wallBox && grid[i][j + 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 9:
                                {
                                    if (grid[i][j - 1][k - 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 11:
                                {
                                    if (grid[i][j - 1][k - 1].value != wallBox && grid[i][j - 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 12:
                                {
                                    if (grid[i][j + 1][k - 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 13:
                                {
                                    if (grid[i][j - 1][k - 1].value != wallBox && grid[i][j + 1][k - 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 14:
                                {
                                    if (grid[i][j + 1][k - 1].value != wallBox && grid[i][j + 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                                case 15:
                                {
                                    if (grid[i][j - 1][k - 1].value != wallBox && grid[i][j - 1][k + 1].value != wallBox && grid[i][j + 1][k - 1].value != wallBox && grid[i][j + 1][k + 1].value != wallBox)
                                        special = true;
                                    break;
                                }
                            }
                            if(special)
                                currentBox->boxSprite.setTextureRect(sf::IntRect(40, 40 * rectToDisplace, 40, 40));
                        }
                        if (rectToDisplace == 0 && currentBox->value != wallBox)
                            currentBox->boxSprite.setTextureRect(sf::IntRect(40, 0, 40, 40));
                        animate(currentBox, dT);
                        if (currentBox->transitionStatus != 'P' && currentBox->delayTimer < 0) {
                            if (stage >= 7 && stage <= 9) {
                                sf::Color c = currentBox->boxSprite.getColor();
                                c.r = rand() % 256;
                                c.g = c.r;
                                c.b = c.r;
                                currentBox->boxSprite.setColor(c);
                            }
                            else {
                                currentBox->boxSprite.setColor(sf::Color::White);
                            }
                            layer.draw(currentBox->boxSprite);
                        }
                        break;
                    }
                    case teleBox:
                    {
                        animate(currentBox, dT);
                        currentBox->boxSprite.setTextureRect(sf::IntRect(80, 40 * currentBox->currentFrame, 40, 40));
                        if (currentBox->rectClock.getElapsedTime().asSeconds() > 0.01666) {
                            currentBox->rectClock.restart();
                            currentBox->currentFrame++;
                            if (currentBox->currentFrame > 15)
                                currentBox->currentFrame = 0;
                        }
                        currentBox->boxSprite.setPosition(currentBox->x + mapXgap, currentBox->y + mapYgap);
                        layer.draw(currentBox->boxSprite);
                        break;
                    }
                    case foodBox:
                    {
                        currentBox->boxSprite.setTextureRect(sf::IntRect(120, 40 * currentBox->currentFrame, 40, 40));
                        if (currentBox->rectClock.getElapsedTime().asSeconds() > 0.01666) {
                            currentBox->rectClock.restart();
                            currentBox->currentFrame++;
                            if (currentBox->currentFrame > 15)
                                currentBox->currentFrame = 0;
                        }
                        currentBox->boxSprite.setPosition(currentBox->x + mapXgap + 7,  currentBox->y + mapYgap + 7);
                        currentBox->boxSprite.setScale(0.65, 0.65);
                        if(!targetReached)
                            layer.draw(currentBox->boxSprite);
                        break;
                    }
                    case frutBox:
                    {
                        animate(currentBox, dT);
                        currentBox->boxSprite.setTextureRect(sf::IntRect(200, 40 * currentBox->currentFrame, 40, 40));
                        if(targetReached)
                            layer.draw(currentBox->boxSprite);
                        break;
                    }
                    case suprBox:
                    {
                        animate(currentBox, dT);
                        currentBox->boxSprite.setTextureRect(sf::IntRect(160, 40 * currentBox->currentFrame, 40, 40));
                        if (currentBox->rectClock.getElapsedTime().asSeconds() > 0.1) {
                            currentBox->rectClock.restart();
                            currentBox->currentFrame++;
                            if (currentBox->currentFrame > 15)
                                currentBox->currentFrame = 0;
                        }
                        layer.draw(currentBox->boxSprite);
                        break;
                    }
                }
                layer.draw(currentBox->whiteBox);
            }
        }
    }
}

void Level::releaseGhost() {
    if (releaseClock.getElapsedTime().asSeconds() < 3)
        return;
    releaseClock.restart();
    int ghostToRelease = -1;
    for (int i = 0; i < ghostCount; i++) {
        if (ghosts[i].ghostLoose == false) {
            ghostToRelease = i;
            break;
        }
    }
    if (ghostToRelease == -1)
        return;
    ghosts[ghostToRelease].x = 22 * gridLength + gridLength / 2;
    ghosts[ghostToRelease].y = 7 * gridLength + gridLength / 2;
    ghosts[ghostToRelease].ghostLoose = true;
    int direction = rand() % 2;
    if (direction == 0) {
        ghosts[ghostToRelease].dir = 'r';
        ghosts[ghostToRelease].tempDir = 'r';
    }
    else {
        ghosts[ghostToRelease].dir = 'l';
        ghosts[ghostToRelease].tempDir = 'l';
    }
    ghosts[ghostToRelease].resetCoordinateQueue();
}

Level::~Level() {
    for (int i = 0; i < totalParts; i++) {
        obliteratePart(i);
    }
    for (int i = 0; i < ghostCount; i++) {
        Minion* ptr = ghosts[i].firstMinion;
        while (ptr) {
            Minion* toDelete = ptr;
            ptr = ptr->next;
            delete toDelete;
        }
        ghosts[i].firstMinion = nullptr;
    }
    for (int i = 0; i < totalParts; i++) {
        for (int j = 0; j < gameRows; j++) {
            delete[] grid[i][j];
        }
        delete[] grid[i];
    }
    delete[] grid;
}