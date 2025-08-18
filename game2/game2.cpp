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

//drawVisibleTiles(renderTexture, tileMap, tileSprite, tileSet, tileSize, cameraX, cameraY);

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
    int damage = 20;;
    //cooldown time if zombie took a damage
    float damageTakenCooldown = 0.0f;
    Vector2f knockback = Vector2f(0, 0);

};
//creating MainPlayer from playerState class.
PlayerState MainPlayer;





Vector2f getZombieSpawnPosition(Vector2f playerPos) {

    //create a rectangle around the camera
    FloatRect camera(playerPos.x - 170, playerPos.y - 130, 330, 250);

    float x, y;
    do {
        x = 200 + rand() % 801;
        y = 200 + rand() % 801;
    } while (camera.contains(x, y)); // try again if inside camera


    x -= (playerPos.x - 160);
	y -= (playerPos.y - 120);


    return { x, y };
}









class ZombieDeath {

public:
    static Texture zombieDeathTex;
    Sprite zombieDeathSprite;
    Vector2f XY;
    int frame = 0;
    float timePassed = 0.0f;
    ZombieDeath(Vector2f z,Vector2f worldPos) {
        XY = z + worldPos;
        zombieDeathSprite.setTexture(zombieDeathTex);
        zombieDeathSprite.setPosition(XY);
        zombieDeathSprite.setTextureRect(IntRect(0, 0, 32, 32));
    }
    void updateZombieDeath(float dt, RenderTexture& renderTexture,float CameraX,float CameraY) {
        timePassed += dt;
        if(frame <=2){
        if (timePassed >= 0.2f) {
            frame++;
            timePassed = 0.0f;
        }
            }

            zombieDeathSprite.setPosition(XY - Vector2f(CameraX , CameraY));
            zombieDeathSprite.setTextureRect(IntRect(frame * 24 , 0, 24, 32));
            renderTexture.draw(zombieDeathSprite);
        
    }
};

Texture ZombieDeath::zombieDeathTex;








//zombie class
class Zombie {
public:
    static Texture zombieTex;
    Sprite zombieSprite;
    Vector2f XY;
	bool isDead = false;
    float speed = 40.0f;
	int health = 100;
    int direction = 0; // 0 = down, 1 = right, 2 = up, 3 = left
    Vector2f knockback = Vector2f(0, 0);
    
	bool bleeding = false; 

	int frame = 0;
	float timePassed = 0.0f;
	int directionRow = 0; // 0 = down, 1 = right, 2 = up, 3 = left


   
    void location() {
		cout << "Zombie Location: " << XY.x << ", " << XY.y << endl;
    }

    void zombieKnockbackUpdate(float dt) {
        if (knockback.x != 0 || knockback.y != 0) {
            XY += knockback * dt;
            knockback *= 0.9f;

            if (abs(knockback.x) < 1 && abs(knockback.y) < 1)
                knockback = Vector2f(0, 0);

            zombieSprite.setPosition(XY);
        }
    }


    void proccessZombieKnockback(Vector2f z,Vector2f playerPosition) {
        Vector2f knockDir = z - playerPosition;
        float knockLen = sqrt(knockDir.x * knockDir.x + knockDir.y * knockDir.y);
        if (knockLen != 0)
            knockDir /= knockLen;

        knockback = knockDir * 200.0f;
    }



    Zombie(Vector2f playerPos) {
       
        XY = getZombieSpawnPosition(playerPos);
        zombieSprite.setTexture(zombieTex);
       
        zombieSprite.setPosition(XY);
       zombieSprite.setTextureRect(IntRect(0,0,16,32));

    }

    void zombieBleedAnimation(RenderTexture& renderTexture, float dt, Sprite& blood) {
        static int frame = 0;
        static float time = 0;
        if (time > 0.10f) {
            frame++;
            time = 0;
        }
        
        time += dt;
		
        blood.setTextureRect(IntRect(frame * 32, 0 , 32, 16));
        blood.setPosition(XY.x - 8, XY.y + 16);
		renderTexture.draw(blood);
        if (frame >= 4) {
            frame = 0;
			bleeding = false; // stop bleeding animation
        }
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
        if (timePassed >= 0.25f) {
            frame++;
            timePassed = 0.0f;
            if (frame >= 4)frame = 0;
            
            zombieSprite.setTextureRect(IntRect(frame * 16, directionRow * 32, 16, 32));
            zombieSprite.setPosition(XY.x, XY.y);


        }
    }
    
    void zombieDamage(int width, int height,Vector2f playerLocation,Vector2f zombieLocation,int damage) {
        
        Vector2f direction = Vector2f(width / 2 - 8, height / 2 - 16) - XY;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length < 20 && MainPlayer.damageTakenCooldown <= 0) {
            MainPlayer.health -= damage;
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


void playerAttackAnimation(float dt,Sprite playerSwordAnimation,int direction,bool& playerAttackAnimationBOOL, RenderTexture& renderTexture){
	static int frame = 0;
    static float time = 0;
        if (time > 0.10f) {
            frame++;
            time = 0;
        }
        if (frame >= 4) {
            frame = 0;
			playerAttackAnimationBOOL = false;

		}
 
    time += 0.016f;
    playerSwordAnimation.setOrigin(18, 14);
    playerSwordAnimation.setTextureRect(IntRect(0 * 16,frame  * 16, 32, 16));
    switch(direction) {
        case 0: // down 
            playerSwordAnimation.setPosition(320 / 2 + 6, 240 / 2 + 8);
           
            if(frame == 0)playerSwordAnimation.setRotation(45);
            else playerSwordAnimation.setRotation(200);
            break;
			
        case 1: // right
            playerSwordAnimation.setPosition(167,130);
            if (frame == 0)playerSwordAnimation.setRotation(-45);
            else playerSwordAnimation.setRotation(110);
            break;
        case 2: // up

            playerSwordAnimation.setPosition(160, 118);
            if (frame == 0)playerSwordAnimation.setRotation(-135);
            else playerSwordAnimation.setRotation(20);

            break;
           
        case 3: // left
            playerSwordAnimation.setPosition(152, 130);
            if (frame == 0)playerSwordAnimation.setRotation(135);
            else playerSwordAnimation.setRotation(290);
            break;
	}
	renderTexture.draw(playerSwordAnimation);

}

//function to calculate the distance between two zombies
//subtract the x and y then find the length of the triangle
float distanceBetweenZombies(Vector2f a, Vector2f b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

void checkForNewLevel(vector<Zombie> zom,int &Level,bool &spawn){
    if (zom.size() == 0) {
        Level++;
        spawn = true;
    }
}


void buttonAnimation(RenderTexture& renderTexture, Sprite& button) {
    static int frame = 0;
    static float time = 0.0f;
    if (time > 0.15f) {
        frame++;
        time = 0.0f;
    }
    if (frame >= 4) frame = 0;
    time += 0.016f;
    button.setTextureRect(IntRect( 0 ,frame * 32, 48, 32));
    renderTexture.draw(button);
}







void renderScreen(RenderTexture& renderTexture, RenderWindow& window,bool display) {
    renderTexture.display();
    Sprite screen(renderTexture.getTexture());
    screen.setScale(2.0f, 2.0f);
    window.clear(Color::White);
    window.draw(screen);
    if(display)window.display();
    
}


void handleEvents(RenderWindow& window)
{
    Event event;
    while (window.pollEvent(event))
    {
        if (event.type == Event::Closed)
        {
            window.close();
        }
    }
}





void typeText(RenderTexture& renderTexture,RenderWindow& window, Text& text, const string& message, int speedMs = 500)
{
    string shown = "";
    for (size_t i = 0; i <= message.size(); i++)
    {
        handleEvents(window);
        shown = message.substr(0, i);
        text.setString(shown);
        renderScreen(renderTexture, window, false);
        window.draw(text);
        window.display();
       // renderScreen(renderTexture, window,true);

        
        
        sleep(milliseconds(speedMs)); // wait a little
    }
}




void showCutsceneBG(RenderTexture& renderTexture, Sprite& noFadeHouse, Sprite& noFadeTalkBar, int genderIndex) {

    renderTexture.draw(noFadeHouse);
    noFadeTalkBar.setPosition(0, 166);
    if (genderIndex == 0) {

        noFadeTalkBar.setTextureRect(IntRect(0, 0, 320, 74));
    }
    else {
        noFadeTalkBar.setTextureRect(IntRect(0, 74, 320, 74));
    }
    renderTexture.draw(noFadeTalkBar);
}


void typingDialogFunc(bool& showSpaceButton,bool& typingDialogContinues, RenderTexture& renderTexture, Sprite& noFadeHouse, Sprite& noFadeTalkBar, unsigned char gender, Text& dialog, float dt, string text, bool& currentDialog, bool& nextDialog) {


    static int typingI = 0;
    static float timeTypingI = 0;
    typingDialogContinues = true;
    string message = text;
    string shown = "";
    shown = message.substr(0, typingI);
    typingI++;
    dialog.setString(shown);
    showCutsceneBG(renderTexture, noFadeHouse, noFadeTalkBar, gender);
    if (typingI >= message.size()) {
        timeTypingI += dt;
        if (timeTypingI >= 2.0f) { // wait for 2 seconds before resetting

            showSpaceButton = true;
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                currentDialog = false;
                showSpaceButton = false;
                nextDialog = true;
                typingI = 0; // reset typing index
                timeTypingI = 0;
            }
        }
    }
    sleep(milliseconds(100));
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
    vector<ZombieDeath> deadZombie;
    Texture tileSet, playerTex, swordTex, start, bloodTex, buttonTex, talkBarTex, houseTex;
    if (
        !tileSet.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/tilesTex.png") ||
        !playerTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/playerTex.png") ||
        !swordTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/swordTex.png") ||
        !start.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/theme0.png") ||
        !buttonTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/buttonTex.png") ||
        !houseTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/houseTex.png") ||
        !talkBarTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/talkingBar.png") ||
        !bloodTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/bloodTex.png") ||
        !ZombieDeath::zombieDeathTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/zombieDeathTex.png") ||
        !Zombie::zombieTex.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/zombieTex.png")) {
        cerr << "Error loading Textures\n";
    }
    Font font;
    if (!font.loadFromFile("C:/Users/Best Tech/source/repos/game2/x64/Debug/assets/LibertinusSerif-Regular.ttf")) {
        // Error loading font
        cerr << "Error loading font\n";
    }

    Sprite tileSprite, player, playerSwordAnimation, start1, bleeding, button, talkBar, house, noFadeHouse, noFadeTalkBar;
    noFadeTalkBar.setTexture(talkBarTex);
    talkBar.setTexture(talkBarTex);
    house.setTexture(houseTex);

    noFadeHouse.setTexture(houseTex);
    Color color = house.getColor();

    button.setTexture(buttonTex);
    bleeding.setTexture(bloodTex);
    start1.setTexture(start);
    player.setTexture(playerTex);
    player.setTextureRect(IntRect(0, 0, 16, 32));
    player.setPosition(width / 2 - player.getGlobalBounds().width / 2, height / 2 - player.getGlobalBounds().height / 2);
    tileSprite.setTexture(tileSet);
    playerSwordAnimation.setTexture(swordTex);

    button.setPosition(198, 112);
    playerSwordAnimation.setPosition(width / 2 + 6, height / 2 + 8);


    //creating a rectangle shape for health text background
    RectangleShape healthBG;
    healthBG.setSize(Vector2f(165, 15));
    healthBG.setFillColor(Color::White);
    healthBG.setPosition(5, 5);




    //creating health text on screen
    Text healthText, zombieNumber, levelNumber, Enter, GameName, dialog, SpaceButton;
    SpaceButton.setFont(font);
    SpaceButton.setCharacterSize(16);
    SpaceButton.setFillColor(Color::White);
    //SpaceButton.setPosition(Vector2f(250, 220));
    SpaceButton.setPosition(Vector2f (500, 440));
    SpaceButton.setString("Space");
	SpaceButton.setStyle(Text::Bold);


    dialog.setFont(font);
    dialog.setCharacterSize(20);
    dialog.setFillColor(Color::Black);
    dialog.setPosition(Vector2f(190, 390));
    //dialog.setPosition(Vector2f(100, 190));

    healthText.setFont(font);
    healthText.setCharacterSize(24);
    healthText.setFillColor(Color::Red);
    healthText.setPosition(Vector2f(12, 9));


    levelNumber.setFont(font);
    levelNumber.setCharacterSize(24);
    levelNumber.setFillColor(Color::Black);
    levelNumber.setPosition(Vector2f(135, 9));

    zombieNumber.setFont(font);
    zombieNumber.setCharacterSize(24);
    zombieNumber.setFillColor(Color::Black);
    zombieNumber.setPosition(Vector2f(209, 9));

    Enter.setFont(font);
    Enter.setCharacterSize(30);
    Enter.setFillColor(Color::Black);
    Enter.setPosition(Vector2f(410, 235));
    Enter.setString("Enter");

    GameName.setFont(font);
    GameName.setCharacterSize(30);
    GameName.setFillColor(Color::Black);
    GameName.setPosition(Vector2f(350, 150));
    GameName.setString("No Name Yet!");
    GameName.setStyle(Text::Bold);



    //float alpha = 0;
    Vector2f worldPos = Vector2f(0.0f, 0.0f);
    //bool gameStarted = false;
    int Level = 1;
    int zombieDamage = 10;
    int playerAnimate = 0;
    //for delta time
    Clock clock;
    bool zombieSpawn = true;
    //player speed and camera position
    float speed = 80;
    float cameraX = 300, cameraY = 300;
    bool playerAttackKey = false;
    bool playerAttackAnimationBOOL = false;
    static int playerAttackDirection = 0;
    float playerAttackTime = 0;










   
  //  static int typingI = 0;
    //static float timeTypingI = 0;


    bool gameTheme = true;
    bool gameStartedT = false;


    bool gameStarted = false;
    bool fadingHouse = false;
    bool fadingTalkBar = false;
    
    bool showSpaceButton = false;
	bool showBGCutscene = false;

    bool 
        typingDialog = false,
        typingDialog1 = false,
        typingDialog2 = false,
        typingDialog3 = false,
        typingDialog4 = false;

    bool typingDialogContinues = false;

    int alpha = 0;

    Color houseColor = Color::White;
    Color talkBarColor = Color::White;




    // Game loop
    while (window.isOpen()) {
       

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();
        renderTexture.clear(Color::Black);
        // ---- MENU STATE ---- 
        if (gameTheme)
        {
            renderTexture.clear();
            renderTexture.draw(start1);
            buttonAnimation(renderTexture, button);

            window.draw(Enter);
            window.draw(GameName);


            if (Keyboard::isKeyPressed(Keyboard::Enter))
            {

                fadingHouse = true;
                gameTheme = false;
                // alpha = 0;
            }
        }

        if (fadingHouse) {
            if (alpha <= 255) {
                alpha += 2;
                houseColor.a = alpha;
                house.setColor(houseColor);
                renderTexture.draw(house);
            }
            else {
                fadingHouse = false;
                fadingTalkBar = true;
                alpha = 0;
            }
            //fix 1 frame glitch between fade house and no fade house
            if (alpha > 250 && alpha <= 256) {
                renderTexture.clear(Color::White);
                renderTexture.draw(noFadeHouse);
            }
        }

		
        

        if (fadingTalkBar)
        {
            if (alpha <= 255)
            {
                alpha += 2;
                talkBarColor.a = alpha;
                talkBar.setColor(talkBarColor);
                talkBar.setPosition(0, 166);
                talkBar.setTextureRect(IntRect(0, 0, 320, 74));

                houseColor.a = 255;
                house.setColor(houseColor);
                renderTexture.draw(house);


                renderTexture.draw(talkBar);
            }

           
            else
            {
                fadingTalkBar = false;
                typingDialog = true;
                clock.restart();
                dialog.setString(""); // clear text first

                renderTexture.clear(Color::Transparent);
            
            }
            //fix 1 frame glitch between fade talkBar and no fade talkBar+
            if (alpha > 250 && alpha <= 256) { 
            noFadeTalkBar.setPosition(0, 166); 
            noFadeTalkBar.setTextureRect(IntRect(0, 0, 320, 74));
            renderTexture.draw(noFadeTalkBar); 
            }
        }

		//Handling all game dialiog with typingDialogFunc function. just need to pass the parameters like gender , message , current if statement , next if statement

        //typingDialogFunc(             ,                      ,              ,             ,             ,gender ,      ,    , text,currentDialog  , nextDialog ); 
        //typingDialogFunc(showSpaceButton, typingDialogContinues, renderTexture, noFadeHouse, noFadeTalkBar, gender, dialog, dt, text, currentDialog, nextDialog);
        
        if (typingDialog) {
            typingDialogFunc(showSpaceButton, typingDialogContinues, renderTexture, noFadeHouse, noFadeTalkBar, 0, dialog, dt, "Are You Really .... Have to? . . . \nI Want You To Be Here.", typingDialog, typingDialog1);
        }
        if (typingDialog1) {
            typingDialogFunc(showSpaceButton, typingDialogContinues, renderTexture, noFadeHouse, noFadeTalkBar, 1, dialog, dt, "My Dear . ..I Have No Choice.      \nI Must Go", typingDialog1, typingDialog2);
        }
        if (typingDialog2) {
            typingDialogFunc(showSpaceButton, typingDialogContinues, renderTexture, noFadeHouse, noFadeTalkBar, 0, dialog, dt, "How Long This Battle Will Last?              ", typingDialog2, typingDialog3);
        }
      




        if (gameStartedT) {



            //show white background
            renderTexture.clear(Color::White);
            bool playerAllowToKnockback = true;
            //calculate delta time
            //float dt = clock.restart().asSeconds();
            //if (dt > 0.018f) dt = 0.017f;
            playerAttackTime += dt;






            //show animation if the player is moving also the player will animate in the direction they are moving
            bool isPlayerMoving = false;


            //store the zombie movement will be same with different framerate
            float zombieMovementX = 0, zombieMovementY = 0;
            //camera movement
            if (!playerAttackAnimationBOOL) {
                if (Keyboard::isKeyPressed(Keyboard::W)) {
                    cameraY -= speed * dt; isPlayerMoving = true; playerAnimate = 2; playerAttackDirection = 2; zombieMovementY += speed * dt;
                    if (cameraY < 70) { cameraY += speed * dt; zombieMovementY -= speed * dt; }
                }
                if (Keyboard::isKeyPressed(Keyboard::S)) {
                    cameraY += speed * dt; isPlayerMoving = true; playerAnimate = 0; playerAttackDirection = 0;  zombieMovementY -= speed * dt;
                    if (cameraY > 925) { cameraY -= speed * dt; zombieMovementY += speed * dt; }
                }
                if (Keyboard::isKeyPressed(Keyboard::A)) {
                    cameraX -= speed * dt; isPlayerMoving = true; playerAnimate = 3; playerAttackDirection = 3;  zombieMovementX += speed * dt;
                    // if (cameraX < 45) { cameraX += speed * dt; zombieMovementX -= speed * dt; }
                }
                if (Keyboard::isKeyPressed(Keyboard::D)) {
                    cameraX += speed * dt; isPlayerMoving = true; playerAnimate = 1; playerAttackDirection = 1;  zombieMovementX -= speed * dt;
                    if (cameraX > 860) { cameraX -= speed * dt; zombieMovementX += speed * dt; }
                }

                if (Keyboard::isKeyPressed(Keyboard::W) && Keyboard::isKeyPressed(Keyboard::D)) {
                    cameraY += speed * dt * 0.292893;
                    cameraX -= speed * dt * 0.292893;
                    zombieMovementY -= speed * dt * 0.292893;
                    zombieMovementX += speed * dt * 0.292893;
                    // prevent camera from going too far left
                }
                if (Keyboard::isKeyPressed(Keyboard::W) && Keyboard::isKeyPressed(Keyboard::A)) {
                    cameraY += speed * dt * 0.292893;
                    cameraX += speed * dt * 0.292893;
                    zombieMovementY -= speed * dt * 0.292893;
                    zombieMovementX -= speed * dt * 0.292893;
                }
                if (Keyboard::isKeyPressed(Keyboard::S) && Keyboard::isKeyPressed(Keyboard::D)) {
                    cameraY -= speed * dt * 0.292893;
                    cameraX -= speed * dt * 0.292893;
                    zombieMovementY += speed * dt * 0.292893;
                    zombieMovementX += speed * dt * 0.292893;
                }
                if (Keyboard::isKeyPressed(Keyboard::S) && Keyboard::isKeyPressed(Keyboard::A)) {
                    cameraY -= speed * dt * 0.292893;
                    cameraX += speed * dt * 0.292893;
                    zombieMovementY += speed * dt * 0.292893;
                    zombieMovementX -= speed * dt * 0.292893;
                }

                if (cameraX < 55 || cameraX > 860 || cameraY < 70 || cameraY > 925)  playerAllowToKnockback = false;

                if (Keyboard::isKeyPressed(Keyboard::Space) && playerAttackTime >= 0.4) {
                    playerAttackKey = true;
                    playerAttackAnimationBOOL = true;
                    playerAttackTime = 0;
                }
            }

            if (playerAttackAnimationBOOL) { if (cameraX < 55 || cameraX > 860 || cameraY < 70 || cameraY > 925)  playerAllowToKnockback = false; }


            cerr << "x " << cameraX + 160 << " --- " << "y " << cameraY + 130 << endl;

            //loop for spawning zombies by a value

            if (zombieSpawn) {
                int N;
                switch (Level) {
                case 1:
                    N = 2;
                    break;
                case 2:
                    N = 4;
                    break;
                case 3:
                    N = 6;
                    break;
                case 4:
                    N = 8;
                    break;
                case 5:
                    N = 10;
                    break;
                case 6:
                    N = 12;
                    break;
                case 7:
                    N = 14;
                    break;
                case 8:
                    N = 16;
                    break;
                case 9:
                    N = 18;
                    break;
                case 10:
                    N = 20;
                    break;

                }
                for (int i = 0; i < N; i++) {
                    zombie.push_back(Zombie(Vector2f(cameraX, cameraY)));
                }
                zombieSpawn = false;
            }

            checkForNewLevel(zombie, Level, zombieSpawn);

            if (MainPlayer.damageTakenCooldown > 0) MainPlayer.damageTakenCooldown -= dt;


            //draw background 
            drawVisibleTiles(renderTexture, tileMap, tileSprite, tileSet, tileSize, cameraX, cameraY);

            //draw dead zombies
            for (auto& zDead : deadZombie) {
                zDead.updateZombieDeath(dt, renderTexture, cameraX, cameraY);
                renderTexture.draw(zDead.zombieDeathSprite);
            }




            //draw player
            if (playerAttackDirection == 2) {
                if (playerAttackAnimationBOOL) playerAttackAnimation(dt, playerSwordAnimation, playerAttackDirection, playerAttackAnimationBOOL, renderTexture);
            }
            playerAnimation(player, dt, isPlayerMoving, playerAnimate);
            renderTexture.draw(player);




            Vector2f getPlayerPosition = player.getPosition();
            if (playerAttackKey) {

                for (auto& z : zombie) {
                    switch (playerAnimate) {
                    case 0:



                        if (z.XY.x > getPlayerPosition.x - 20 && z.XY.x < getPlayerPosition.x + 20 && z.XY.y > getPlayerPosition.y && z.XY.y < getPlayerPosition.y + 40) {
                            z.health -= MainPlayer.damage;

                            z.bleeding = true;

                            z.proccessZombieKnockback(z.XY, getPlayerPosition);
                        }
                        break;
                    case 1:
                        if (z.XY.x > getPlayerPosition.x && z.XY.x < getPlayerPosition.x + 40 && z.XY.y > getPlayerPosition.y - 20 && z.XY.y < getPlayerPosition.y + 20) {
                            z.health -= MainPlayer.damage;
                            z.bleeding = true;

                            z.proccessZombieKnockback(z.XY, getPlayerPosition);

                        }
                        break;
                    case 2:
                        if (z.XY.x > getPlayerPosition.x - 20 && z.XY.x < getPlayerPosition.x + 20 && z.XY.y > getPlayerPosition.y - 40 && z.XY.y < getPlayerPosition.y) {
                            z.health -= MainPlayer.damage;
                            z.bleeding = true;
                            z.proccessZombieKnockback(z.XY, getPlayerPosition);


                        }
                        break;
                    case 3:
                        if (z.XY.x > getPlayerPosition.x - 40 && z.XY.x < getPlayerPosition.x && z.XY.y > getPlayerPosition.y - 20 && z.XY.y < getPlayerPosition.y + 20) {
                            z.health -= MainPlayer.damage;
                            z.bleeding = true;
                            z.proccessZombieKnockback(z.XY, getPlayerPosition);


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

                z.zombieDamage(width, height, player.getPosition(), z.zombieSprite.getPosition(), zombieDamage);
                z.zombieKnockbackUpdate(dt);

                if (z.health <= 0) deadZombie.push_back(ZombieDeath(z.XY, Vector2f(cameraX, cameraY)));


                //z.location();

             // Check distance between all zombies and push them apart if too close
                float minDistance = 25.0f; // minimum distance be zombies
                for (auto& z1 : zombie) {
                    for (auto& z2 : zombie) {
                        if (&z1 == &z2) continue; // skip same zombie

                        float distance = distanceBetweenZombies(z1.XY, z2.XY);
                        if (distance < minDistance) {
                            Vector2f diff = z1.XY - z2.XY;
                            if (distance != 0) diff /= distance; // normalize

                            z1.XY += diff * (minDistance - distance) * 0.5f;
                            z2.XY -= diff * (minDistance - distance) * 0.5f;
                        }
                    }
                }
                //RenderTexture& renderTexture, float dt, Sprite blood)
                renderTexture.draw(z.zombieSprite);


            }
            ////playerSwordAnimation(playerSwordAnimation,playerAnimate);
            if (playerAttackDirection != 2) {
                if (playerAttackAnimationBOOL) playerAttackAnimation(dt, playerSwordAnimation, playerAttackDirection, playerAttackAnimationBOOL, renderTexture);
            }

            for (int i = 0; i < zombie.size(); i++) {
                if (zombie[i].health <= 0) {
                    zombie.erase(zombie.begin() + i);
                    i--; // go back one step because the list is now smaller
                }
            }


            //update health , level and zombie number text
            //to_string convert number to string
            healthText.setString("Health: " + to_string(MainPlayer.health));
            levelNumber.setString("LVL: " + to_string(Level));
            zombieNumber.setString("Zombies: " + to_string(zombie.size()));
            //draw background for states
            renderTexture.draw(healthBG);





            for (auto& z : zombie) {
                if (z.bleeding) {
                    z.zombieBleedAnimation(renderTexture, dt, bleeding);
                    renderTexture.draw(bleeding);
                }
            }


            if (playerAllowToKnockback) {
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
            }

        }// if gameStartedT





        //display the scaled window
        renderTexture.display();
        Sprite screen(renderTexture.getTexture());
        screen.setScale(2.0f, 2.0f);
        window.clear();
        window.draw(screen);


        if (gameStartedT) {
            //draw states in higher resolution
            window.draw(healthText);
            window.draw(levelNumber);
            window.draw(zombieNumber);
        }

        //if (typingDialog) window.draw(dialog);

        if(typingDialogContinues)window.draw(dialog);

        if (showSpaceButton) window.draw(SpaceButton);
        if (gameTheme)
        {
            window.draw(Enter);
            window.draw(GameName);
        }


        window.display();

    }
    return 0;

}