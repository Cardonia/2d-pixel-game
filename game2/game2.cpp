#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <SFML/Graphics.hpp>
using namespace sf;
using namespace std;

// Function to load a map from a file and return it as a 2D vector
//you can use this multiple times to load different maps all to different vectors
vector<vector<int>> loadMap(const string& filename) {
    ifstream file(filename);
    vector<vector<int>> map;
    string line;
    if (!file.is_open()) {
        cerr << "Error opening file map: " << filename << endl;
        return map; // Return empty map if file cannot be opened
    }
    while (getline(file, line)) {
        stringstream ss(line);
        int val;
        vector<int> row;

        while (ss >> val)
            row.push_back(val);

        map.push_back(row);
    }
    return map;
}


//draw the visible tiles based on the camera position ( background )
void drawVisibleTiles(RenderTexture& renderTexture, const vector<vector<int>>& tileMap,Sprite& tileSprite, const Texture& tileset,int tileSize, int cameraX, int cameraY) {
    int firstTileX = cameraX / tileSize - 1;
    int firstTileY = cameraY / tileSize - 1;
    int tilesInViewX = 22 + cameraX / tileSize;
    int tilesInViewY = 17 + cameraY / tileSize;

    for (int y = firstTileY; y < tilesInViewY; y++) {
        for (int x = firstTileX; x < tilesInViewX; x++) {
            if (y < 0 || y >= tileMap.size() || x < 0 || x >= tileMap[y].size()) continue;

            int tileNum = tileMap[y][x] - 1;

            int tu = tileNum % (tileset.getSize().x / tileSize);
            int tv = tileNum / (tileset.getSize().x / tileSize);

            tileSprite.setTextureRect(IntRect(tu * tileSize, tv * tileSize, tileSize, tileSize));
            tileSprite.setPosition(x * tileSize - cameraX, y * tileSize - cameraY);
            renderTexture.draw(tileSprite);
        }
    }
}


void playerAnimation(Sprite& sprite, float deltaTime, bool isWalking, int row) {
    static int frame = 0;
    static float animationTimer = 0;
    const int frameCount = 4;
    const int frameWidth = 16;
    const int frameHeight = 32;
    const float animationSpeed = 0.2f;

    if (isWalking) {
        animationTimer += deltaTime;

        if (animationTimer >= animationSpeed) {
            animationTimer = 0;
            frame++;
            if (frame >= frameCount)
                frame = 0;
        }
    }
    else {
        frame = 0;
    }

    sprite.setTextureRect(IntRect(frame * frameWidth, row * frameHeight, frameWidth, frameHeight));
}




class PlayerState {
public:
    RectangleShape Shape;
    
    
    bool isAttacking = false;
    int health = 100;
    int damage = 200;;
    //cooldown time if zombie took a damage
    float damageTakenCooldown = 0.0f;
    Vector2f knockback = Vector2f(0, 0);

};
//creating MainPlayer from playerState class.
PlayerState MainPlayer;



//zombie class
class Zombie {
public:
    static Texture zombieTex;
    Sprite zombieSprite;
    Vector2f XY;
    float speed = 20.0f;
	int health = 100, damage = 5;
    int direction = 0; // 0 = down, 1 = right, 2 = up, 3 = left
	bool  zombieKnockback = false;

	int frame = 0;
    int maxFrame = 3;
	float frameTime = 0.25f;
	float timePassed = 0.0f;
	int directionRow = 0; // 0 = down, 1 = right, 2 = up, 3 = left


    Zombie() {
        
        XY.x = rand()%600;
        XY.y= rand()%600;
        zombieSprite.setTexture(zombieTex);
        
        zombieSprite.setPosition(XY);
       zombieSprite.setTextureRect(IntRect(0,0,16,32));

    }
    void updateLocationByPlayer(float x, float y,float deltaTime,int row) {
        
        XY.x += x;
        XY.y += y;
        zombieSprite.setPosition(XY);

      

    }

    void updateZombieMove(int width, int height, float dt) {
        Vector2f direction = Vector2f(width / 2 - 8, height / 2 - 16) - XY;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length != 0) direction /= length;
        XY += direction * speed * dt;


        


        if (abs(direction.x) > abs(direction.y)) {
            if (direction.x > 0) directionRow = 1;//left to right
            else directionRow = 3;//right to left
        }
        else {
            if (direction.y > 0) directionRow = 0;//top to down
            else directionRow = 2;//down to top
        }

        timePassed += dt;
        if (timePassed >= frameTime) {
            frame++;
            timePassed = 0.0f;
            if (frame >= maxFrame) {
                frame = 0;
            }
            zombieSprite.setTextureRect(IntRect(frame * 16, directionRow * 32, 16, 32));
            zombieSprite.setPosition(XY.x, XY.y);


        }
    }
    
    void zombieDamage(int width, int height,Vector2f playerLocation,Vector2f zombieLocation) {
        
        Vector2f direction = Vector2f(width / 2 - 8, height / 2 - 16) - XY;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 20 && MainPlayer.damageTakenCooldown <= 0) {
            MainPlayer.health -= 5;
            MainPlayer.damageTakenCooldown = 0.5f;

            Vector2f knockDir = playerLocation - zombieLocation;
            float knockLength = sqrt(knockDir.x * knockDir.x + knockDir.y * knockDir.y);
            if (knockLength != 0)
                knockDir /= knockLength;

            
            MainPlayer.knockback = knockDir * 200.0f;

        }
    }

  

};

Texture Zombie::zombieTex;


void playerSwordAnimation(Sprite& sprite,int direction) {
  
}


int main() {
    srand(time(0));

    RenderTexture renderTexture;
    renderTexture.create(320, 240);
    int width = 320, height = 240;


    //window size and frame rate limit
    RenderWindow window(VideoMode(width * 2, height * 2), "Second Game");
    window.setFramerateLimit(60);
    //window.setVerticalSyncEnabled(true);
    int tileSize = 16;

    vector<vector<int>> tileMap = loadMap("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/map.txt");
    vector<Zombie> zombie;
    Texture tileSet, playerTex,swordTex;
    if (
        !tileSet.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/tiles.png") ||
        !playerTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/playerTex.png") ||
        !swordTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/directionAttacks.png") ||
        !Zombie::zombieTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/playerTex.png")) {
        cerr << "Error loading Textures\n";
    }
    Font font;
    if (!font.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/LibertinusSerif-Regular.ttf")) {
        // Error loading font
        cerr << "Error loading font\n";
    }

    Sprite tileSprite, player,playerSwordAnimation;
    player.setTexture(playerTex);
    player.setTextureRect(IntRect(0, 0, 16, 32));
    player.setPosition(width / 2 - player.getGlobalBounds().width / 2, height / 2 - player.getGlobalBounds().height / 2);
    tileSprite.setTexture(tileSet);
    playerSwordAnimation.setTexture(swordTex);
    //creating a rectangle shape for health text background
    RectangleShape healthBG;
    healthBG.setSize(Vector2f(70, 15));
    healthBG.setFillColor(Color::White);
    healthBG.setPosition(5, 5);




    //creating health text on screen
    Text healthText;
    healthText.setFont(font);
    healthText.setCharacterSize(12);
    healthText.setFillColor(Color::Red);
    healthText.setPosition(Vector2f(8, 5));





    //for delta time
    Clock clock;
    bool zombieSpawn = true;
    //player speed and camera position
    float speed = 80;
    float cameraX = 0, cameraY = 0;
    bool playerAttackKey = false;

    float playerAttackTime = 0;
    // Game loop
    while (window.isOpen()) {

        Event event;
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed)
                window.close();
        }
        //show white background
        renderTexture.clear(Color::White);

        //calculate delta time
        float dt = clock.restart().asSeconds();
        //if (dt > 0.018f) dt = 0.017f;
        playerAttackTime += dt;






        //show animation if the player is moving also the player will animate in the direction they are moving
        bool isPlayerMoving = false;
        int playerAnimate = 0;

        //store the zombie movement will be same with different framerate
        float zombieMovementX = 0, zombieMovementY = 0;
        //camera movement
        if (Keyboard::isKeyPressed(Keyboard::W)) { cameraY -= speed * dt; isPlayerMoving = true; playerAnimate = 2; zombieMovementY += speed * dt; }
        if (Keyboard::isKeyPressed(Keyboard::S)) { cameraY += speed * dt; isPlayerMoving = true; playerAnimate = 0; zombieMovementY -= speed * dt; }
        if (Keyboard::isKeyPressed(Keyboard::A)) { cameraX -= speed * dt; isPlayerMoving = true; playerAnimate = 3; zombieMovementX += speed * dt; }
        if (Keyboard::isKeyPressed(Keyboard::D)) { cameraX += speed * dt; isPlayerMoving = true; playerAnimate = 1; zombieMovementX -= speed * dt; }

        if (Keyboard::isKeyPressed(Keyboard::W) && Keyboard::isKeyPressed(Keyboard::D)) {
            cameraY += speed * dt * 0.293;
            cameraX -= speed * dt * 0.293;
            zombieMovementY -= speed * dt * 0.293;
            zombieMovementX += speed * dt * 0.293;
        }
        if (Keyboard::isKeyPressed(Keyboard::W) && Keyboard::isKeyPressed(Keyboard::A)) {
            cameraY += speed * dt * 0.293;
            cameraX += speed * dt * 0.293;
            zombieMovementY -= speed * dt * 0.293;
            zombieMovementX -= speed * dt * 0.293;
        }
        if (Keyboard::isKeyPressed(Keyboard::S) && Keyboard::isKeyPressed(Keyboard::D)) {
            cameraY -= speed * dt * 0.293;
            cameraX -= speed * dt * 0.293;
            zombieMovementY += speed * dt * 0.293;
            zombieMovementX += speed * dt * 0.293;
        }
        if (Keyboard::isKeyPressed(Keyboard::S) && Keyboard::isKeyPressed(Keyboard::A)) {
            cameraY -= speed * dt * 0.293;
            cameraX += speed * dt * 0.293;
            zombieMovementY += speed * dt * 0.293;
            zombieMovementX -= speed * dt * 0.293;
        }

        if (Keyboard::isKeyPressed(Keyboard::Space) && playerAttackTime >= 2) {
            playerAttackKey = true;
            playerAttackTime = 0;
        }
        cout << playerAttackTime << endl;

        // cerr<<"x "<<cameraX + 160 << " --- " <<"y " << cameraY + 130 << endl;

         //loop for spawning zombies by a value
        if (zombieSpawn) {
            for (int i = 0; i < 10; i++) {
                zombie.push_back(Zombie());
            }
            zombieSpawn = false;
        }

        if (MainPlayer.damageTakenCooldown > 0) MainPlayer.damageTakenCooldown -= dt;


        //draw background 
        drawVisibleTiles(renderTexture, tileMap, tileSprite, tileSet, tileSize, cameraX, cameraY);

        //draw player
        playerAnimation(player, dt, isPlayerMoving, playerAnimate);
        renderTexture.draw(player);


        //update health number every frame
        //to_string convert number to string
        healthText.setString("Health: " + to_string(MainPlayer.health));

        //draw health text with background
        renderTexture.draw(healthBG);
        renderTexture.draw(healthText);


        Vector2f getPlayerPosition = player.getPosition();
        if (playerAttackKey) {

            for (auto& z : zombie) {
                switch (playerAnimate) {
                case 0:
                    if (z.XY.x > getPlayerPosition.x - 20 && z.XY.x < getPlayerPosition.x + 20 && z.XY.y > getPlayerPosition.y && z.XY.y < getPlayerPosition.y + 40) {
                        z.health -= MainPlayer.damage;
                        z.XY.x = 0; z.XY.y = 0;
                        cout << z.health << endl;
						//z.zombieKnockback = true;
                    }
                    break;
                case 1:
                    if (z.XY.x > getPlayerPosition.x && z.XY.x < getPlayerPosition.x + 40 && z.XY.y > getPlayerPosition.y - 20 && z.XY.y < getPlayerPosition.y + 20) {
                        z.health -= MainPlayer.damage;
                        z.XY.x = 0; z.XY.y = 0;
                        cout << z.health << endl;
                    }
                    break;
                case 2:
                    if (z.XY.x > getPlayerPosition.x - 20 && z.XY.x < getPlayerPosition.x + 20 && z.XY.y > getPlayerPosition.y - 40 && z.XY.y < getPlayerPosition.y) {
                        z.health -= MainPlayer.damage;
                        z.XY.x = 0; z.XY.y = 0;
                        cout << z.health << endl;
                    }
                    break;
                case 3:
                    if (z.XY.x > getPlayerPosition.x - 40 && z.XY.x < getPlayerPosition.x && z.XY.y > getPlayerPosition.y - 20 && z.XY.y < getPlayerPosition.y + 20) {
                        z.health -= MainPlayer.damage;
                        z.XY.x = 0; z.XY.y = 0;
                        cout << z.health << endl;
                    }
                    break;
                }

            }

            playerAttackKey = false;



        }

        //draw zombies
        //loop through the zombies and update their positions. continues as many as there are zombies in the vector
        for (auto& z : zombie) {

            z.updateLocationByPlayer(zombieMovementX, zombieMovementY, dt, playerAnimate);
            z.updateZombieMove(width, height, dt);
            //if (z.zombieDamage(width, height))MainPlayer.health -= 5;
            z.zombieDamage(width, height, player.getPosition(), z.zombieSprite.getPosition());




            renderTexture.draw(z.zombieSprite);

        }
        //playerSwordAnimation(playerSwordAnimation,playerAnimate);

        for (int i = 0; i < zombie.size(); i++) {
            if (zombie[i].health <= 0) {
                zombie.erase(zombie.begin() + i);
                i--; // go back one step because the list is now smaller
            }
        }




        if (MainPlayer.knockback.x != 0 || MainPlayer.knockback.y != 0) {
            cameraX += MainPlayer.knockback.x * dt;
            cameraY += MainPlayer.knockback.y * dt;

            for (auto& z : zombie) {
                z.XY -= MainPlayer.knockback * dt;
            }

            // reduce knockback over time
            MainPlayer.knockback *= 0.9f; // 90% every frame

            // stop if very small
            if (abs(MainPlayer.knockback.x) < 1 && abs(MainPlayer.knockback.y) < 1)
                MainPlayer.knockback = Vector2f(0, 0);
        }

        //display the scaled window
        renderTexture.display();
        Sprite screen(renderTexture.getTexture());
        screen.setScale(2.0f, 2.0f);
        window.clear();
        window.draw(screen);
        window.display();

    }
    return 0;

}