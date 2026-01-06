#include "MyGame.h"
#include <iostream>
#include "../engine/Projectile.h"
#include <vector>
#include <memory>

//constructor
MyGame::MyGame() : AbstractGame()
{
    score = 0;
    currentScene = SceneState::MENU;
    initialised = false;
}

//game resource loading
void MyGame::initGame()
{
	//debug output for init
    std::cout << "initGame called. gfx=" << gfx.get() << "\n";

	//load textures
    bgTex = ResourceManager::loadTexture("res/images/bg.png", SDL_COLOR_GRAY);

	//load player (FSM) and enemy textures
    SDL_Texture* pIdle = ResourceManager::loadTexture("res/images/fin_idle.png", SDL_COLOR_WHITE);
    SDL_Texture* pDmg = ResourceManager::loadTexture("res/images/fin_damaged.png", SDL_COLOR_WHITE);
    SDL_Texture* pSht = ResourceManager::loadTexture("res/images/fin_shoot.png", SDL_COLOR_WHITE);
    SDL_Texture* eTex = ResourceManager::loadTexture("res/images/Circle_Red.png", SDL_COLOR_WHITE);

	//load sounds
    ResourceManager::loadSound("res/sounds/collect.wav");
    ResourceManager::loadSound("res/sounds/hurt.wav");
    ResourceManager::loadSound("res/sounds/shoot.wav");
    ResourceManager::loadSound("res/sounds/beep.wav");
    ResourceManager::loadMP3("res/sounds/DDLoop1.wav");

	//setup audio layers
    mySystem->CreateLayer("sfx");
    mySystem->CreateLayer("music");
	//set layer volumes
    mySystem->SetLayerVolume("sfx", 0.8f);
    mySystem->SetLayerVolume("music", 0.5f);

	//play background music
    mySystem->PlayMusic("res/sounds/DDLoop1.wav", 1.0f, true);

	//assign textures to entities
    player.setTextures(pIdle, pDmg, pSht, pIdle);
    enemy.setTexture(eTex);

	//load font for UI
    uiFont = ResourceManager::loadFont("res/fonts/arial.ttf", 24);
	//set font for graphics engine
    gfx->useFont(uiFont);

	//setup background rectangles
    bgDest = { 0, 0, 800, 600 };

	//setup player rectangles
    restartGame();

	//register physics objects
    physics->registerObject(player.getPhysics());
    physics->registerObject(enemy.getPhysics());

	//mark as initialised and finished
    initialised = true;
}

//reset game state
void MyGame::restartGame()
{
	//setting both inital positions for enemy and player
    player.setPosition({ 100, 100 });
    enemy.setPosition({ 600, 450 });

	//reset entity states
    player.getDamage().reset();
    player.setIdle();

    //resetting score and scene 
    score = 0;
    currentScene = SceneState::MENU;

	//reset respawn timer and collision state
    gameKeys.clear();
    for (int i = 0; i < 5; i++) {
		//spawn collectibles at random positions
        Point2 randomPos(rand() % 700 + 50, rand() % 500 + 50);
        gameKeys.push_back(std::make_shared<GameKey>(randomPos));
    }
}

//main update entry point
void MyGame::update()
{
	//initialize game resources if not done already
    if (!initialised)
    {
        initGame();
        return;
    }
	//scene management
    if (currentScene == SceneState::GAME)
    {
        updateGame();
    }
}

//core gameplay logic
void MyGame::updateGame()
{
	//player input handling (mouse + keyboard)
	Point2 move(0, 0); //movement vector (mouse)
	if (eventSystem->isPressed(Key::W)) move.y = -5; //up
	if (eventSystem->isPressed(Key::S)) move.y = 5; //down
	if (eventSystem->isPressed(Key::A)) move.x = -5; //left
	if (eventSystem->isPressed(Key::D)) move.x = 5; //right

	player.applyInput(move); //apply movement input to player

	//using mouse position for player rotation
	Point2 mousePos = eventSystem->getMousePos(); //get current mouse position
	player.rotateTowards(mousePos, 0.016f);//rotate player towards mouse with delta time
	player.update(0.016f);//update player state
	enemy.update(0.016f, player.getPosition()); //update enemy with player position

	respawnTimer += 0.016f; //increment respawn timer for enemy and collectibles

	bool isColliding = player.getPhysics()->isColliding(*enemy.getPhysics()); //check collision between player and enemy

    //collision enter logic for player damage
    if (isColliding && !wasCollidingWithEnemy)
    {
		if (enemy.isAlive()) //only apply damage if enemy is alive
        {
			if (player.getDamage().applyDamage()) //apply damage to player
            {
				player.setDamaged(); //set player visual state to damaged
				mySystem->Play("sfx", "res/sounds/hurt.wav"); //play hurt sound
            }

			if (player.getDamage().isDead()) //check if player is dead
            {
				player.setDead();//set player visual state to dead
                currentScene = SceneState::GAMEOVER;
            }
        }
    }
	wasCollidingWithEnemy = isColliding; //update collision state for next frame

    //spawn projectile with offset to avoid self collision
    if (eventSystem->isPressed(Mouse::BTN_LEFT) && player.canShoot())
    {
		SDL_Rect r = player.getRect(); //get player rectangle by reference
		Point2 center{ r.x + r.w / 2, r.y + r.h / 2 }; //calculate player center point by reference
		Point2 mouse = eventSystem->getMousePos(); //get current mouse position for aiming

		//calculate direction vector from player to mouse by casting to float for precision
		Vector2f dir(static_cast<float>(mouse.x - center.x), static_cast<float>(mouse.y - center.y)); //direction vector from player to mouse
		Vector2f unitDir = normalise(dir); //normalize direction vector for consistent speed and direction values

        //offset spawn point by 50 units to clear player hitbox
        Point2 fireOrigin{ (int)(center.x + unitDir.x * 50.0f), (int)(center.y + unitDir.y * 50.0f) };

		//create and add new projectile to the list using shared pointer for memory management
        projectiles.push_back(std::make_shared<Projectile>(fireOrigin, unitDir));

		//play shooting sound and update player state
        mySystem->Play("sfx", "res/sounds/shoot.wav");
		//request shooting state change
        player.onShoot();
    }

    //collectible pickup detection
    for (auto& k : gameKeys)
    {
		//skip if collectible is not alive
        if (!k->isAlive) continue;

		//check collision between player and collectible
        if (player.getPhysics()->isColliding(*k->physics))
        {
			//collect the key
            k->isAlive = false;
			//increase score and play sounds
            score += 100;
            mySystem->Play("sfx", "res/sounds/beep.wav");
            mySystem->Play("sfx", "res/sounds/collect.wav");
        }
    }

    //projectile movement and enemy collision
    for (auto& p : projectiles) {
		//skip dead projectiles
        if (!p->isAlive()) continue;
		//update projectile position
        p->update(0.016f);

		//check collision with enemy because player projectiles don't hit player
        if (enemy.isAlive() && p->getPhysics()->isColliding(*enemy.getPhysics())) {
			//apply damage to enemy
            if (enemy.getDamage().applyDamage()) {
				//also increase score on hit
                score += 50;
                mySystem->Play("sfx", "res/sounds/beep.wav");
                mySystem->Play("sfx", "res/sounds/hurt.wav");
				//check if enemy is dead after damage
                if (enemy.getDamage().isDead()) 
                    enemy.kill();
            }
			p->kill(); //destroy projectile on hit
        }
    }

    //remove dead projectile entities
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::shared_ptr<Projectile>& p) { return !p->isAlive(); }),
        projectiles.end()
    );

    //enemy respawn timer check
	if (!enemy.isAlive() && respawnTimer >= RESPAWN_DELAY) //respawn enemy after RESPAWN_DELAY
    {
		enemy.revive(Point2(rand() % 700 + 50, rand() % 500 + 50)); //respawn at random position
    }

    //collectible respawn logic with multiplier
    for (auto& k : gameKeys)
    {
		//respawn collectibles faster than enemy because of multiplier
        if (!k->isAlive && (respawnTimer * COLLECTIBLE_MULTIPLIER >= RESPAWN_DELAY))
        {
			//respawn collectible at random position
            k->isAlive = true;
            k->physics->setCenter(Point2(rand() % 700 + 50, rand() % 500 + 50));
        }
    }

    //global respawn timer reset
    bool allAlive = enemy.isAlive();
	//check if all collectibles are alive
    for (auto& k : gameKeys) if (!k->isAlive) allAlive = false;
	//reset timer if everything is alive
    if (allAlive) respawnTimer = 0.0f;

	//win condition check
    if (score >= 3000)
    {
        currentScene = SceneState::WIN;
		//stop music on win
        mySystem->StopMusic();
    }
}

//main render pipeline
void MyGame::render()
{
	//safety check for initialisation
    if (!initialised) return;

	//clear screen before rendering because double buffering
    gfx->clearScreen();

	//scene rendering with different UI elements
    if (currentScene == SceneState::MENU)
    {
        gfx->useFont(uiFont);
        gfx->setDrawColor(SDL_COLOR_WHITE);
        gfx->drawText("MY GAME", 350, 250);
        gfx->drawText("PRESS SPACE TO START", 280, 300);
    }
	//the game state rendering with all entities, UI and background
    else if (currentScene == SceneState::GAME)
    {
        gfx->drawTexture(bgTex, &bgDest);

        gfx->setDrawColor(SDL_COLOR_YELLOW);
        for (auto& k : gameKeys)
        {
            if (!k->isAlive) continue;
            gfx->drawCircle(k->physics->getCenter(), 10);
        }

        player.render(gfx.get());

        if (enemy.isAlive()) enemy.render(gfx.get());

        gfx->setDrawColor(SDL_COLOR_WHITE);
        for (auto& p : projectiles)
        {
            if (!p->isAlive()) continue;
            p->render(gfx.get());
        }

        gfx->setDrawColor(SDL_COLOR_WHITE);
        gfx->drawText("Score: " + std::to_string(score), 20, 20);
    }
	//game over scene rendering with retry prompt
    else if (currentScene == SceneState::GAMEOVER)
    {
        gfx->useFont(uiFont);
        gfx->setDrawColor(SDL_COLOR_RED);
        gfx->drawText("GAME OVER", 300, 250);
        gfx->drawText("Press SPACE to try again", 250, 320);
    }
	//win scene rendering with final score display
    else if (currentScene == SceneState::WIN)
    {
        gfx->useFont(uiFont);
        gfx->setDrawColor(SDL_COLOR_GREEN);
        gfx->drawText("VICTORY!", 350, 200);
        gfx->setDrawColor(SDL_COLOR_WHITE);
        gfx->drawText("Final Score: " + std::to_string(score), 320, 260);
        gfx->drawText("You saved the X-CUBE world!", 250, 320);
        gfx->drawText("Press SPACE to Play Again", 270, 400);
    }

    //render debug overlays on top of scene
    renderAudioDebug();


	//present rendered frame to screen
    gfx->showScreen();
}

//input event dispatcher
void MyGame::handleKeyEvents()
{
	//scene transition handling between menu, game, and end states
    if ((currentScene == SceneState::MENU || currentScene == SceneState::WIN || currentScene == SceneState::GAMEOVER) &&
		eventSystem->isPressed(Key::SPACE)) //start or restart game
    {
        restartGame();
        currentScene = SceneState::GAME;
    }
	//if in game, allow returning to menu with ESC
    else if (currentScene == SceneState::GAME && eventSystem->isPressed(Key::ESC))
    {
        currentScene = SceneState::MENU;
    }

    if(eventSystem->isPressed(Key::F)) //toggle debug menu with F key
    {
        showDebugMenu = !showDebugMenu;
	}
}

void MyGame::renderUI() {}

//audio debug overlay rendering
void MyGame::renderAudioDebug()
{
	//only render if debug menu is enabled
    if (!showDebugMenu) return;

	//draw background box
	SDL_Rect debugBox = { 510, 20, 285, 220 }; //position and size
	gfx->setDrawColor(SDL_COLOR_GRAY); //gray background
	gfx->fillRect(&debugBox); //fill rectangle

	gfx->setDrawColor(SDL_COLOR_WHITE); //white text
	gfx->drawText("AUDIO DEBUG (F)", 520, 30); //title

	int yOffset = 60; //initial vertical offset for layers
	std::vector<std::string> layerNames = { "music", "sfx", "ui", "foley" }; //example layers

	for (const auto& name : layerNames) //loop through each layer
    {
		gfx->drawText(name, 520, yOffset); //draw layer name

		SDL_Rect barBorder = { 580, yOffset + 5, 120, 15 }; //volume bar border
		gfx->setDrawColor(SDL_COLOR_BLACK); //black border
		gfx->drawRect(&barBorder); //draw border

		float vol = (name == "music") ? 0.5f : 0.8f; //get volume for layer (example values)
		SDL_Rect barFill = { 580, yOffset + 5, static_cast<int>(120 * vol), 15 }; //filled volume bar

		gfx->setDrawColor(SDL_COLOR_GREEN); //green fill
		gfx->fillRect(&barFill); //draw filled bar

		gfx->setDrawColor(SDL_COLOR_WHITE); //white text for percentage
		gfx->drawText(std::to_string(static_cast<int>(vol * 100)) + "%", 710, yOffset); //draw volume percentage

        yOffset += 30;
    }

	gfx->setDrawColor(SDL_COLOR_BLUE); //blue text for track info
	gfx->drawText("Track: DDLoop1.wav", 520, yOffset + 5); //current track
	gfx->drawText("Loop: ON", 520, yOffset + 25);//loop status
}

//destructor
MyGame::~MyGame() {}