#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstring>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Stuff.h"

bool isSuper = false; // Use ID to differentiate between Pacman and Ghosts
sf::Clock superClock;
int foodID = 0;
const int ghostCount = 4;
const int minionLimit = 100;
bool startShock = false;

int main() {
    srand(time(0));

    // [0] and [1] are intro and main menu
    // [2] to [10] are random chosen for Stages 1 to 4, 5 (Boss 1), 6 (Boss 2)
    // [11] is for Stage 7 (Boss 3)
    // [12] is for Stage 8 (Boss 4)
    // [13] is for Stage 9 (Super Boss)
    std::string trackNames[] = {
        "MDK - Press Start Mashup (Original Mix Vip Mix 10 Remixes)",
        "MDK - Press Start Mashup (Original Mix Vip Mix 10 Remixes)",
        "[Electro] - Nitro Fun Hyper Potions - Checkpoint [Monstercat Release]",
        "[Electro] - Rogue - Adventure Time [Monstercat Release]",
        "F-777 - Airborne Robots",
        "MDK ft. Travis Montgomery - Shockwave",
        "MDK - Infinite",
        "MDK meganeko - Supercharge",
        "Nitro Fun - Hidden Level [Monstercat Release]",
        "Teminite & MDK - Space Invaders",
        "Teminite Panda Eyes - Highscore",
        "LISA The Painful OST - Mens Hair Club",
        "Teminite - A New Dawn",
        "OMORI-OST-014-Acrophobia"
    };
    std::string trackList[] = {
        "Music/0_1.ogg",
        "Music/0_2.ogg",
        "Music/1_1.ogg",
        "Music/1_2.ogg",
        "Music/1_3.ogg",
        "Music/1_4.ogg",
        "Music/1_5.ogg",
        "Music/1_6.ogg",
        "Music/1_7.ogg",
        "Music/1_8.ogg",
        "Music/1_9.ogg",
        "Music/2_1.ogg",
        "Music/2_2.ogg",
        "Music/3_1.ogg"
    };

    sf::Music music;
    int track = rand() % 13;
    track = 1;
    music.openFromFile(trackList[track]);
    std::cout << "Now Playing: " << trackNames[track] << std::endl;

    sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight), "PacManOverdrive", sf::Style::Close | sf::Style::Titlebar);

    sf::Clock statusClock;
    statusClock.restart();
    sf::Clock dT;
    dT.restart();
    float dTvalue = 0;

    Pacman pacman;
    Ghost* ghosts = new Ghost[ghostCount]{};
    int alpha = 150;
    ghosts[0].c = sf::Color::Red;
    ghosts[1].c = sf::Color(0xff, 0xb7, 0xff);
    ghosts[2].c = sf::Color::Cyan;
    ghosts[3].c = sf::Color(0xff, 0xb7, 0x51);

    PelletBox pelletBox;

    sf::RectangleShape outline(sf::Vector2f(gridLength, gridLength));
    outline.setOrigin(sf::Vector2f(gridLength / 2, gridLength / 2));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::White);
    outline.setOutlineThickness(gridLength / 20);

    Menu mainMenu("main");
    int state = 0;
    // 0 is main menu, 1 is paused, 2 is playing, 3 is watching cutscene
    int stage = -1;
    int currentTrack = -1;

    Level* currentLevel = nullptr;

    bool morbinTime = false;
    Ghost* ghostToMorb = nullptr;
    Ghost* ghostToMorb2 = nullptr;
    Minion* minionToMorb = nullptr;
    float morbDelays[] = { 0.5, 0.25, 0.125, 0.0625, 0.03125, 0.015625 };
    int morbCount = 0;
    float morbDelay = morbDelays[0];
    sf::Clock morbClock;
    int currentMorbed = 0;

    sf::Texture livesTexture;
    sf::Sprite lives;
    livesTexture.loadFromFile("Textures/bullet.png");
    lives.setTexture(livesTexture);
    sfmlText scoreText;
    scoreText.text.setPosition(sf::Vector2f(gridLength * 2, (float)gameHeight * 9 / 10));

    int pitchTracker = 0;
    int zoomTracker = 0;
    sf::View morbinZoom(sf::FloatRect(gameWidth / 2, gameHeight / 2, gameWidth, gameHeight));
    sf::SoundBuffer eatingGhost;
    eatingGhost.loadFromFile("SFX/eatingGhost2.wav");
    sf::Sound eatingSoundList[minionLimit + 1];
    for (int i = 0; i < minionLimit; i++) {
        eatingSoundList[i].setBuffer(eatingGhost);
        eatingSoundList[i].setPitch(1 + (float)i * 2 / 100);
    }
    for (int i = 0; i < ghostCount; i++) {
        ghosts[i].eatingGhost.setBuffer(eatingGhost);
    }

    sf::RectangleShape fruitBarExterior;
    fruitBarExterior.setSize(sf::Vector2f(gridLength * 10, gridLength));
    fruitBarExterior.setOrigin(sf::Vector2f(fruitBarExterior.getSize().x / 2, fruitBarExterior.getSize().y / 2));
    fruitBarExterior.setOutlineColor(sf::Color::White);
    fruitBarExterior.setFillColor(sf::Color::Transparent);
    fruitBarExterior.setOutlineThickness(gridLength / 8);
    fruitBarExterior.setPosition(sf::Vector2f(gameWidth / 2, float(gameHeight * 9.5) / 10));
    sf::RectangleShape fruitBar;
    fruitBar.setSize(sf::Vector2f(gridLength * 10 - gridLength / 2, gridLength * 0.5));
    fruitBar.setOrigin(sf::Vector2f(fruitBar.getSize().x / 2, fruitBar.getSize().y / 2));
    fruitBar.setFillColor(sf::Color::White);
    fruitBar.setPosition(fruitBarExterior.getPosition());

    sf::RectangleShape bingus;
    bingus.setSize(sf::Vector2f(gameWidth, gameHeight));
    bingus.setFillColor(sf::Color::Transparent);

    sf::RenderTexture target;
    target.create(gameWidth, gameHeight);
    bool shockwave = false;
    sf::Shader shockwaveShader;
    shockwaveShader.loadFromFile("Textures/shockwave.glsl", sf::Shader::Fragment);
    sf::Clock shockwaveClock;
    sf::View shockwaveZoom;
    shockwaveZoom.setSize(500, 500);

    currentLevel = new Level(11, &pacman, ghosts, ghostCount, &pelletBox);
    currentLevel->transitionRandom(currentLevel->currentPart);
    dT.restart();
    pacman.score = 0;
    pacman.lives = 1000;
    state = 0;
    mainMenu.current->ron.setFillColor(sf::Color::Black);
    mainMenu.current->transitionStatus = 'n';
    mainMenu.current = mainMenu.root;
    currentLevel->releaseClock.restart();
    pacman.x = 40 * gridLength + gridLength / 2;
    pacman.y = 1 * gridLength + gridLength / 2;
    pacman.tempDir = 'r';
    pacman.dir = 'r';

    while (window.isOpen()) {
        target.clear();

        if(currentLevel != nullptr)
        if(!currentLevel->targetReached)
            scoreText.text.setString(std::to_string(pacman.score));
        if (music.getStatus() == sf::Music::Status::Stopped) {
            if (stage >= 0 && stage <= 6 || stage == 10) {
                int track = rand() % 9 + 2; // 2 to 10
                while (currentTrack == track)
                    track = rand() % 9 + 2;
                currentTrack = track;
                music.openFromFile(trackList[track]);
                music.play();
                // music.setPlayingOffset(sf::seconds(music.getDuration().asSeconds() * 0.98));
                std::cout << "Now Playing: " << trackNames[track] << std::endl;
            }
        }
        if (morbinTime) {
            if (morbClock.getElapsedTime().asSeconds() > morbDelay * 2) {
                pacman.frame = (pacman.frame + 1) % 3;
                pacman.sprite.setTextureRect(sf::IntRect(pacman.frame * 40, 0, 40, 40));
                morbClock.restart();
                zoomTracker++;
                if (ghostToMorb != nullptr) {
                    ghostToMorb2 = ghostToMorb;
                    pacman.morb(ghostToMorb, morbCount);
                    minionToMorb = ghostToMorb->firstMinion;
                    ghostToMorb->eatingGhost.play();
                    ghostToMorb->resetPos();
                    ghostToMorb = nullptr;
                    if (state == 2) {
                        morbinZoom.setCenter(pacman.x + mapXgap, pacman.y + mapYgap);
                        morbinZoom.setSize(gameWidth * float(128 - zoomTracker) / 256, gameHeight * float(128 - zoomTracker) / 256);
                    }
                    else {
                        morbinZoom.setCenter(gameWidth / 2, gameHeight / 2);
                        morbinZoom.setSize(gameWidth, gameHeight);
                    }
                }
                else if (minionToMorb != nullptr) {
                    pacman.morb(minionToMorb, morbCount);
                    eatingSoundList[pitchTracker].play();
                    Minion* nextTemp = minionToMorb->next;
                    if (currentMorbed == ghostCount) {
                        currentLevel->minionManager.prepareToYeet(minionToMorb);
                        minionToMorb->next = nullptr;
                    }
                    else {
                        currentLevel->minionManager.transferMinion(minionToMorb);
                    }
                    minionToMorb = nextTemp;
                    if (state == 2) {
                        morbinZoom.setCenter(pacman.x + mapXgap, pacman.y + mapYgap);
                        morbinZoom.setSize(gameWidth * float(128 - zoomTracker) / 256, gameHeight * float(128 - zoomTracker) / 256);
                    }
                    else {
                        morbinZoom.setCenter(gameWidth / 2, gameHeight / 2);
                        morbinZoom.setSize(gameWidth, gameHeight);
                    }
                }
                else {
                    ghostToMorb2->firstMinion = nullptr;
                    dT.restart();
                    morbinTime = false;
                    ghostToMorb = nullptr;
                    minionToMorb = nullptr;
                    morbCount = 0;
                    pitchTracker = 0;
                    zoomTracker = 0;
                    morbDelay = morbDelays[0];
                    currentLevel->minionManager.resetMorb();
                    morbClock.restart();
                    pacman.sprite.setRotation(0);
                    pacman.x = pacman.gridPosX();
                    pacman.y = pacman.gridPosY();
                    morbinZoom.setCenter(gameWidth / 2, gameHeight / 2);
                    morbinZoom.setSize(gameWidth, gameHeight);
                    window.setView(morbinZoom);
                }
                if (morbinTime) {
                    morbCount++;
                    pitchTracker++;
                    if (morbCount < 6) {
                        morbDelay = morbDelays[morbCount];
                    }
                    else {
                        morbDelay *= 0.99;
                    }
                }
            }
        }
        else {
            if (startShock) {
                shockwave = true;
                shockwaveClock.restart();
                startShock = false;
            }
            if (isSuper) {
                for (int j = 0; j < ghostCount; j++) {
                    ghosts[j].sprite.setColor(sf::Color(0x29, 0x86, 0xcc));
                    ghosts[j].setMinionColor(sf::Color(0x29, 0x86, 0xcc, alpha));
                }
            }
            if (isSuper && superClock.getElapsedTime().asSeconds() > 10) {
                ghosts[0].setMinionColor(ghosts[0].c);
                ghosts[1].setMinionColor(ghosts[1].c);
                ghosts[2].setMinionColor(ghosts[2].c);
                ghosts[3].setMinionColor(ghosts[3].c);
                for (int i = 0; i < ghostCount; i++) {
                    ghosts[i].sprite.setColor(sf::Color::White);
                }
                isSuper = false;
                morbinTime = false;
                currentLevel->minionManager.resetMorb();
            }

            for (int i = 0; i < ghostCount; i++) {
                if (pacman.checkCollision(&ghosts[i])) {
                    if (isSuper && ghosts[i].isMorbed == false) {
                        morbinTime = true;
                        ghosts[i].isMorbed = true;
                        while (!currentLevel->minionManager.morbTurn.empty())
                            currentLevel->minionManager.morbTurn.pop();
                        for (int j = 0; j < ghostCount; j++) {
                            if (ghosts[j].isMorbed)
                                continue;
                            currentLevel->minionManager.morbTurn.push(j);
                        }
                        Minion* ptr = ghosts[i].firstMinion;
                        ghostToMorb = &ghosts[i];
                        currentMorbed++;
                        break;
                    }
                    else if (isSuper && ghosts[i].isMorbed == true) {

                    }
                    else {
                        pacman.lives--;
                        pacman.resetPos();
                        for (int j = 0; j < ghostCount; j++) {
                            ghosts[j].resetPos();
                            ghosts[j].resetCoordinateQueue();
                        }
                    }
                }
                Minion* ptr = ghosts[i].firstMinion;
                while (ptr) {
                    if (pacman.checkCollision(ptr)) {
                        if (isSuper) {

                        }
                        else {
                            pacman.lives--;
                            pacman.resetPos();
                            for (int j = 0; j < ghostCount; j++) {
                                ghosts[j].resetPos();
                                ghosts[j].resetCoordinateQueue();
                            }
                        }
                    }
                    ptr = ptr->next;
                }
            }

            if (morbinTime == true)
                continue;

            if (state != 1) {
                if(!currentLevel->isIntro)
                    pelletBox.movePelletBox(currentLevel->grid, currentLevel->currentPart, dTvalue);
                if (state == 2)
                    pacman.handleInput(currentLevel, dTvalue);
                else
                    pacman.handleInput2(currentLevel, dTvalue);

                currentLevel->minionManager.updateMinions();
                for (int i = 0; i < ghostCount; i++) {
                    if (ghosts[i].ghostLoose)
                        ghosts[i].moveGhost2(currentLevel, dTvalue);
                    else
                        ghosts[i].moveGhost(currentLevel, dTvalue);
                }
                currentLevel->releaseGhost();
            }

            dTvalue = dT.getElapsedTime().asSeconds();
            dT.restart();

            sf::Event e;
            while (window.pollEvent(e)) {
                if (e.type == sf::Event::KeyPressed) {
                    if (e.key.code == sf::Keyboard::Key::Left && pacman.speed > 5) {
                        pacman.speed -= 5;
                        pacman.threshold = (float)pacman.speed / 25;
                        std::cout << "Speed: " << pacman.speed << "\n";
                        /*
                        * Used during Debugging for checking Transitions. Can effectively double the stages in a level.
                        if (currentLevel->currentPart > 0) {
                            currentLevel->obliteratePart(currentLevel->currentPart);
                            currentLevel->currentPart--;
                            currentLevel->transitionRandom(currentLevel->currentPart);
                        }*/
                    }
                    if (e.key.code == sf::Keyboard::Key::Right && pacman.speed < 400) {
                        pacman.speed += 5;
                        pacman.threshold = (float)pacman.speed / 25;
                        std::cout << "Speed: " << pacman.speed << "\n";
                    }
                    if (e.key.code == sf::Keyboard::Key::P) {
                        state = 1;
                        mainMenu.current = mainMenu.pauseRoot;
                        mainMenu.current->transitionStatus = 't';
                    }
                    if (e.key.code == sf::Keyboard::Key::O) {
                        currentLevel->isIntro = false;
                        currentLevel->obliteratePart(currentLevel->currentPart);
                        currentLevel->currentPart++;
                        if (currentLevel->currentPart < currentLevel->totalParts) {
                            currentLevel->transitionRandom(currentLevel->currentPart);
                            currentMorbed = 0;
                        }
                        else {
                            state = 0;
                            delete currentLevel;
                            currentLevel = nullptr;

                            currentLevel = new Level(10, &pacman, ghosts, ghostCount, &pelletBox);
                            currentLevel->transitionRandom(currentLevel->currentPart);
                            dT.restart();
                            pacman.score = 0;
                            pacman.lives = 1000;
                            mainMenu.current->ron.setFillColor(sf::Color::Black);
                            mainMenu.current->transitionStatus = 'n';
                            mainMenu.current = mainMenu.root;
                            currentLevel->releaseClock.restart();

                            mainMenu.current = mainMenu.levelComplete;
                            mainMenu.current->transitionStatus = 't';
                        }
                    }
                }
                if (e.type == sf::Event::Closed) {
                    return 0;
                }
            }

            if (state == 2 && pacman.lives == 0) {
                state = 0;
                delete currentLevel;
                currentLevel = nullptr;
                mainMenu.current = mainMenu.gameOver;
                mainMenu.current->transitionStatus = 't';

            }
            else if (pacman.pelletsEaten == currentLevel->pelletTargets[currentLevel->currentPart]) {
                currentLevel->targetReached = true;
                currentLevel->grid[currentLevel->currentPart][17][22].value = frutBox;
                currentLevel->grid[currentLevel->currentPart][17][22].currentFrame = rand() % 6;
            }
        }

        if (currentLevel != nullptr && currentLevel->targetReached == true && currentLevel->grid[currentLevel->currentPart][17][22].value != frutBox) {
            currentLevel->isIntro = false;
            currentLevel->obliteratePart(currentLevel->currentPart);
            currentLevel->currentPart++;
            for (int i = 0; i < ghostCount; i++) {
                ghosts[i].speed += 2;
            }
            pacman.speed += 3;
            if (currentLevel->currentPart < currentLevel->totalParts) {
                currentLevel->transitionRandom(currentLevel->currentPart);
                currentMorbed = 0;
            }
            else {
                state = 0;
                delete currentLevel;
                currentLevel = nullptr;
                mainMenu.current = mainMenu.levelComplete;
                mainMenu.current->transitionStatus = 't';
            }
        }

        if(currentLevel != nullptr)
            currentLevel->printLevel(target, dTvalue);

        target.display();

        if (shockwave) {

            if (pacman.shockClock.getElapsedTime().asSeconds() > 0.02) {
                pacman.shockClock.restart();
                if (pacman.shockFrame == 0)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(440, 656, 150, 150));
                }
                else if (pacman.shockFrame == 1)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(644, 464, 168, 168));
                }
                else if (pacman.shockFrame == 2)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(450, 460, 192, 192));
                }
                else if (pacman.shockFrame == 3)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(230, 464, 216, 216));
                }
                else if (pacman.shockFrame == 4)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(444, 234, 220, 220));
                }
                else if (pacman.shockFrame == 5)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(244, 0, 240, 240));
                }
                else if (pacman.shockFrame == 6)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(0, 0, 240, 240));
                }
                else if (pacman.shockFrame == 7)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(0, 480, 240, 240));
                }
                else if (pacman.shockFrame == 8)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(0, 240, 240, 240));
                }
                else if (pacman.shockFrame == 9)
                {
                    pacman.shockSprite.setTextureRect(sf::IntRect(480, 0, 240, 240));
                }
                pacman.shockSprite.setOrigin(120, 120);
                pacman.shockSprite.setPosition(pacman.sprite.getPosition());
                pacman.shockSprite.setScale(3, 3);
                pacman.shockFrame++;
            }

            sf::Vector2f shockwaveCoords;
            shockwaveCoords.x = (float)(pacman.x + mapXgap) / gameWidth;
            shockwaveCoords.y = (float)(pacman.y + 0 * mapYgap) / gameHeight;
            shockwaveCoords.y = 1.0 - shockwaveCoords.y;
            shockwaveCoords.y -= 0.1;
            shockwaveShader.setUniform("iMouse", shockwaveCoords);
            shockwaveShader.setUniform("buffer", sf::Shader::CurrentTexture);
            shockwaveShader.setUniform("iTime", shockwaveClock.getElapsedTime().asSeconds());
            window.draw(sf::Sprite(target.getTexture()), &shockwaveShader);
            if (shockwaveClock.getElapsedTime().asSeconds() > 1) {
                shockwave = false;
            }
            window.draw(pacman.shockSprite);
        }
        else {
            pacman.shockFrame = 0;
            window.draw(sf::Sprite(target.getTexture()));
        }


        for (int i = 0; i < ghostCount; i++) {
            Minion* ptr = ghosts[i].firstMinion;
            while (ptr) {
                window.draw(ptr->sprite);
                ptr = ptr->next;
            }
            if (!(morbinTime && &ghosts[i] == ghostToMorb2)) {
                if (ghosts[i].frameClock.getElapsedTime().asSeconds() > 0.0166) {
                    ghosts[i].currentFrame = (ghosts[i].currentFrame + 1) % 2;
                    ghosts[i].frameClock.restart();
                }
                if (ghosts[i].dir == 'u') {
                    ghosts[i].sprite.setTextureRect(sf::IntRect(160 + ghosts[i].currentFrame * 40, i * 40, 40, 40));
                }
                else if (ghosts[i].dir == 'd') {
                    ghosts[i].sprite.setTextureRect(sf::IntRect(240 + ghosts[i].currentFrame * 40, i * 40, 40, 40));
                }
                else if (ghosts[i].dir == 'l') {
                    ghosts[i].sprite.setTextureRect(sf::IntRect(80 + ghosts[i].currentFrame * 40, i * 40, 40, 40));
                }
                else if (ghosts[i].dir == 'r') {
                    ghosts[i].sprite.setTextureRect(sf::IntRect(0 + ghosts[i].currentFrame * 40, i * 40, 40, 40));
                }
                window.draw(ghosts[i].sprite);
            }
        }
        if (currentLevel != nullptr) {
            for (int i = 0; i < currentLevel->minionManager.minionArray[currentLevel->currentPart].size(); i++) {
                if (currentLevel->minionManager.minionArray[currentLevel->currentPart][i] == nullptr)
                    continue;
                window.draw(currentLevel->minionManager.minionArray[currentLevel->currentPart][i]->sprite);
            }
        }
        if (morbinTime) {
            window.setView(morbinZoom);
            Minion* ptr = minionToMorb;
            while (ptr) {
                window.draw(ptr->sprite);
                ptr = ptr->next;
            }
        }
        if (state == 2) {
            window.draw(scoreText.text);
            for (int i = 0; i < pacman.lives; i++) {
                lives.setPosition(scoreText.text.getPosition().x + i * 10 - 12, scoreText.text.getPosition().y + 40);
                window.draw(lives);
            }
        }
        if(currentLevel != nullptr)
        if (!currentLevel->targetReached)
            fruitBar.setScale(sf::Vector2f((float)pacman.pelletsEaten / currentLevel->pelletTargets[currentLevel->currentPart], 1));
        else
            fruitBar.setScale(sf::Vector2f(1, 1));
        window.draw(fruitBarExterior);
        window.draw(fruitBar);
        if (!morbinTime)
            window.draw(pacman.minionRadius);
        window.draw(pelletBox.sprite);
        window.draw(pacman.sprite);


        if (state == 0) { // Main Menu
            int code = mainMenu.handleInput(window);
            if (code == quitCode) {
                return 0;
            }
            else if (code >= 0 && code <= 10) {
                stage = code;

                if (currentLevel != nullptr)
                    delete currentLevel;
                currentLevel = new Level(stage, &pacman, ghosts, ghostCount, &pelletBox);
                currentLevel->transitionRandom(currentLevel->currentPart);
                dT.restart();
                pacman.score = 0;
                pacman.lives = currentLevel->totalParts;
                state = 2;
                mainMenu.current->ron.setFillColor(sf::Color::Black);
                mainMenu.current->transitionStatus = 'n';
                mainMenu.current = mainMenu.root;
                currentLevel->releaseClock.restart();

                music.stop();
                if (code >= 0 && code <= 6 || code == 10) {
                    int track = rand() % 9 + 2; // 2 to 10
                    // track = 2;
                    currentTrack = track;
                    music.openFromFile(trackList[track]);
                    music.play();
                    // music.setPlayingOffset(sf::seconds(music.getDuration().asSeconds() * 0.98));
                    std::cout << "Now Playing: " << trackNames[track] << std::endl;
                }
                else {
                    music.openFromFile(trackList[code + 4]);
                    music.play();
                    std::cout << "Now Playing: " << trackNames[code + 4] << std::endl;
                }
            }
            else if (code == nothingCode) {
                mainMenu.printMenu(window);
            }
            if (music.getStatus() != sf::Music::Status::Playing) {
                music.play();
            }
        }
        else if (state == 1) { // Paused
            int code = mainMenu.handleInput(window);
            if (code == quitCode) {
                return 0;
            }
            else if (mainMenu.current == mainMenu.root) {
                delete currentLevel;
                state = 0;

                currentLevel = new Level(10, &pacman, ghosts, ghostCount, &pelletBox);
                currentLevel->transitionRandom(currentLevel->currentPart);
                dT.restart();
                pacman.score = 0;
                pacman.lives = 1000;
                mainMenu.current->ron.setFillColor(sf::Color::Black);
                mainMenu.current->transitionStatus = 'n';
                mainMenu.current = mainMenu.root;
                currentLevel->releaseClock.restart();

            }
            else if (code == resumeCode) {
                state = 2;
                dT.restart();
            }
            else if (code == nothingCode) {
                mainMenu.printMenu(window);
            }
        }

        window.display();
        window.clear();
    }
}