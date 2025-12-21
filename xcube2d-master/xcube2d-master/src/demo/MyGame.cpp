#include "MyGame.h"
#include "DamageSystem.h"

MyGame::MyGame() : AbstractGame(), score(0), numKeys(5), gameWon(false), box(5, 5, 128, 128) {

	TTF_Font* font = ResourceManager::loadFont("res/fonts/arial.ttf", 24);
	pickupSFX = ResourceManager::loadSound("res/sounds/collect.wav");
	deathSFX = ResourceManager::loadSound("res/sounds/hurt.wav");

	//get instance of custom engine system
	audioSystem = XEngine::getInstance()->getMyEngineSystem();
	//audioSystem->Init(); //initialize it

	//load background music
	ResourceManager::loadMP3("res/sounds/DDLoop1.wav");


	playerIdleTex = ResourceManager::loadTexture(
		"res/images/fin_idle.png", SDL_COLOR_GRAY);

	playerDamagedTex = ResourceManager::loadTexture(
		"res/images/fin_damaged.png", SDL_COLOR_GRAY);

	playerDeathTex = ResourceManager::loadTexture(
		"res/images/fin_death.png", SDL_COLOR_GRAY);

	playerSrc = { 0, 0, 128, 128 };
	playerDest = { box.x, box.y, 128, 128 };

	bgTex = ResourceManager::loadTexture(
		"res/images/bg.png", SDL_COLOR_GRAY
	);

	//full image
	bgSrc = { 0, 0, 800, 600 };

	//fill the screen
	bgDest = { 0, 0, 800, 600 };

	currentPlayerTex = playerIdleTex;

	gfx->useFont(font);
	gfx->setVerticalSync(true);

	for (int i = 0; i < numKeys; i++) { //spawn keys
		auto k = std::make_shared<GameKey>();
		k->isAlive = true;
		k->pos = Point2(rand() % 800, rand() % 600); //random position within screen bounds
		gameKeys.push_back(k); //add to vector
	}

	for (int i = 0; i < 5; i++)
		collectibles.push_back(Point2(rand() % 800, rand() % 600));

	damage = DamageSystem(3, 30); //3 lives, 30 frame cooldown
	damage.setDamageSound(deathSFX);  

}

MyGame::~MyGame() {

}

void MyGame::handleKeyEvents() {
	int speed = 3;

	if (eventSystem->isPressed(Key::W)) {
		velocity.y = -speed;
	}

	if (eventSystem->isPressed(Key::S)) {
		velocity.y = speed;
	}

	if (eventSystem->isPressed(Key::A)) {
		velocity.x = -speed;
	}

	if (eventSystem->isPressed(Key::D)) {
		velocity.x = speed;
	}
}

void MyGame::update() {
	//menu scene
	if (currentScene == SceneState::MENU)
	{
		if (eventSystem->isPressed(Key::SPACE))
		{
			currentScene = SceneState::GAME;

			audioSystem->PlayMusic("res/sounds/DDLoop1.wav", 0.6f, true);
		}
		return;
	}

	//game over scene
	if (currentScene == SceneState::GAMEOVER) {
		if (eventSystem->isPressed(Key::R) || eventSystem->isPressed(Key::ESC)) {
			restartGame();
		}
		return;
	}

	Point2 mouseWorld = eventSystem->getMousePos(); //get mouse position

	// move player
	box.x += velocity.x;
	box.y += velocity.y;

	if (eventSystem->isPressed(Key::SPACE)) {
		box.x += velocity.x * 2;
		box.y += velocity.y * 2;
	}


	//update player position
	playerDest.x = box.x;
	playerDest.y = box.y;

	//gamekey collection
	for (auto& key : gameKeys) {
		if (key->isAlive && box.contains(key->pos)) {

			//play key sound
			audioSystem->Play("sfx", "res/sounds/collect.wav");

			//update score counters
			score += 200;
			numKeys--;
			//respawn key randomly
			key->pos.x = rand() % 800;
			key->pos.y = rand() % 600;

			key->isAlive = false;
		}
	}

	//collectible collection
	for (auto& c : collectibles) {
		// check collision
		if (box.contains(c)) {

			//play collectible sound
			audioSystem->Play("sfx", "res/sounds/collect.wav");

			score += 50;

			//respawn collectible randomly
			c.x = rand() % 800;
			c.y = rand() % 600;
		}
	}

	//center of player box
	float playerCX = box.x + box.w * 0.5f;
	float playerCY = box.y + box.h * 0.5f;

	//direction to mouse
	float mdx = mouseWorld.x - playerCX;
	float mdy = mouseWorld.y - playerCY;

	//angle in radians
	playerAngle = atan2(mdy, mdx);


	//direction vector
	float dx = playerCX - enemyPos.x;
	float dy = playerCY - enemyPos.y;

	//distance to player
	float length = sqrt(dx * dx + dy * dy);

	//prevent divide by zero crash when enemy overlaps player
	if (length > 0.01f) {

		//normalized direction
		float nx = dx / length;
		float ny = dy / length;

		float chaseSpeed = 2.0f;   //increase speed here

		enemyPos.x += nx * chaseSpeed;
		enemyPos.y += ny * chaseSpeed;
	}

	// collision check with cooldown
	if (damage.tryApplyDamage(box, enemyPos)) {
		audioSystem->Play("sfx", "res/sounds/hurt.wav");

		shakeTimer = 10;
		enemyFlash = true;
		enemyFlashTimer = 5;
	}


	//screen wrap
	if (box.x < 0) box.x = 800;
	if (box.x > 800) box.x = 0;
	if (box.y < 0) box.y = 600;
	if (box.y > 600) box.y = 0;

	// reset velocity
	velocity.x = 0;
    velocity.y = 0;

	// update player visual state
	if (damage.isDead()) {
		playerState = PlayerVisualState::Dead;
	}
	else if (!damage.canTakeDamage()) {
		playerState = PlayerVisualState::Damaged;
	}
	else {
		playerState = PlayerVisualState::Idle;
	}

	//game over check
	if (damage.isDead()) {
		currentScene = SceneState::GAMEOVER;
	}
	//game win check
	if (gameWon) {
		currentScene = SceneState::GAMEOVER;
	}
}

void MyGame::render() {
	if (currentScene != SceneState::GAME)
		return;

	// draw background
	gfx->drawTexture(bgTex, &bgSrc, &bgDest);

	//generate shake offset
	if (shakeTimer > 0) {
		shakeX = rand() % 10 - 5; //random value between -5 and +5
		shakeY = rand() % 10 - 5;
		shakeTimer--;
	}
	else {
		shakeX = 0;
		shakeY = 0;
	}

	switch (playerState) { //select texture based on state (basic FSM)
	case PlayerVisualState::Idle:
		currentPlayerTex = playerIdleTex;
		break;

	case PlayerVisualState::Damaged:
		currentPlayerTex = playerDamagedTex;
		break;

	case PlayerVisualState::Dead:
		currentPlayerTex = playerDeathTex;
		break;
	}


	//PLAYER BOX
	SDL_Point center = {
	playerDest.w / 2,
	playerDest.h / 2
	};

	gfx->drawTexture(
		currentPlayerTex,
		&playerSrc,
		&playerDest,
		playerAngle * 180.0 / M_PI, // radians to degrees (ADD OFFSET IF NEEDED)
		&center,
		SDL_FLIP_NONE
	);



	//KEYS
	gfx->setDrawColor(SDL_COLOR_YELLOW);
	for (auto& key : gameKeys)
		if (key->isAlive)
			gfx->drawCircle(Point2(key->pos.x + shakeX, key->pos.y + shakeY), 5);

	//COLLECTIBLES
	gfx->setDrawColor(SDL_COLOR_GREEN);
	for (auto c : collectibles)
		gfx->drawCircle(Point2(c.x + shakeX, c.y + shakeY), 4);

	//ENEMY FLASH EFFECT
	if (enemyFlash) {
		gfx->setDrawColor(SDL_COLOR_WHITE);
		enemyFlashTimer--;
		if (enemyFlashTimer <= 0)
			enemyFlash = false;
	}
	else {
		gfx->setDrawColor(SDL_COLOR_RED);
	}

	gfx->drawCircle(Point2(enemyPos.x + shakeX, enemyPos.y + shakeY), 10);

}

void MyGame::renderUI() {

	if (currentScene == SceneState::MENU) {
		gfx->setDrawColor(SDL_COLOR_WHITE);
		gfx->drawText("XCUBE2D DEMO", 180, 200);
		gfx->drawText("PRESS SPACE TO START", 140, 280);
		return;
	}

	if (currentScene == SceneState::GAME) {
		gfx->setDrawColor(SDL_COLOR_AQUA);

		//score (top left corner)
		std::string scoreStr = "Score: " + std::to_string(score);
		gfx->drawText(scoreStr, 20, 20);

		//lives (directly under score)
		std::string livesStr = "Lives: " + std::to_string(damage.getLives());
		gfx->drawText(livesStr, 20, 60);

		return;
	}


	if (currentScene == SceneState::GAMEOVER) {
		gfx->setDrawColor(SDL_COLOR_RED);
		gfx->drawText("GAME OVER", 250, 250);
		gfx->drawText("PRESS R TO RESTART", 180, 320);
		return;
	}
}

void MyGame::restartGame() {
	//reset player box & stats
	box = Rect(5, 5, 128, 128);
	score = 0;
	damage.reset();
	numKeys = 5;
	gameWon = false;

	//clear old keys
	gameKeys.clear();
	collectibles.clear();

	//respawn
	for (int i = 0; i < numKeys; i++) {
		auto k = std::make_shared<GameKey>();
		k->isAlive = true;
		k->pos = Point2(rand() % 800, rand() % 600);
		gameKeys.push_back(k);
	}

	//respawn collectibles
	for (int i = 0; i < 5; i++)
		collectibles.push_back(Point2(rand() % 800, rand() % 600));

	enemyPos = Point2(400, 300); //reset enemy position

	currentScene = SceneState::MENU; //back to menu
}
