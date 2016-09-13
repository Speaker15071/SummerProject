#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "level.h"
#include "view.h"
#include "menu.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <list>

using namespace sf;
////////////////////////////////////Общий класс-родитель//////////////////////////
class Entity {
public:
	std::vector<Object> obj;
	float dx, dy, x, y, flg, speed, moveTimer;
	int w, h, health;
	bool life, isMove, onGround;
	Texture texture;
	Sprite sprite;
	String name;
	Entity(Image &image, String Name, float X, float Y, int W, int H) {
		x = X; y = Y; w = W; h = H; name = Name; moveTimer = 0;
		speed = 0; health = 100; dx = 0; dy = 0;
		life = true; onGround = false; isMove = false;
		texture.loadFromImage(image);
		sprite.setTexture(texture);
		sprite.setOrigin(w / 2, h / 2);
	}

	FloatRect getRect() {
		return FloatRect(x, y, w, h);
	}

	virtual void update(float time) = 0;//все потомки переопределяют эту ф-цию

};
////////////////////////////////////////////////////КЛАСС ИГРОКА////////////////////////
class Player :public Entity {
public:
	enum { left, right, up, down, jump, stay, right_Top } state;

	bool isShoot;
	float currentFrame;

	Player(Image &image, String Name, Level &lev, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
	state = stay; obj = lev.GetAllObjects();//инициализируем.получаем все объекты для взаимодействия персонажа с картой
		if (name == "player") {
			sprite.setTextureRect(IntRect(1, 1, w, h));
			health = 5;
		}
	}

	void control() {
		if (Keyboard::isKeyPressed) {//если нажата клавиша
			if (Keyboard::isKeyPressed(Keyboard::Left)) {//а именно левая
				sprite.setScale(-1, 1);
				state = left; speed = 0.2;
			}
			if (Keyboard::isKeyPressed(Keyboard::Right)) {
				sprite.setScale(1, 1);
				state = right; speed = 0.2;
			}

			if ((Keyboard::isKeyPressed(Keyboard::Up)) && (onGround)) {//если нажата клавиша вверх и мы на земле, то можем прыгать
				state = jump; dy = -0.8; onGround = false;//увеличил высоту прыжка
			}

			if (Keyboard::isKeyPressed(Keyboard::Down)) {
				state = down;
			}
		}
	}
	

	void checkCollisionWithMap(float Dx, float Dy)
	{

		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
			{
				if (obj[i].name == "solid")
				{
					if (Dy>0) { y = obj[i].rect.top - h;  dy = 0; onGround = true; }
					if (Dy<0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
					if (Dx>0) { x = obj[i].rect.left - w; }
					if (Dx<0) { x = obj[i].rect.left + obj[i].rect.width; }
				}
				
				// else { onGround = false; }
			}
	}

	void update(float time)
	{
		control();
		switch (state)
		{
		case right:dx = speed; break;
		case left:dx = -speed; break;
		case up: break;
		case down: dx = 0; break;
		case stay: break;
		case right_Top: dx = speed; break;//состояние вправо вверх, просто продолжаем идти вправо
		}
		x += dx*time;
		checkCollisionWithMap(dx, 0);
		y += dy*time;
		checkCollisionWithMap(0, dy);
		sprite.setPosition(x + w / 2, y + h / 2);
		currentFrame += time * 0.01;
		if (currentFrame > 5) { currentFrame -= 5; };
		if (dx > 0 && onGround == true) sprite.setTextureRect(IntRect(48 + 45 * int(currentFrame), 0, 44, 53)); 
		if (dx < 0 && onGround == true) sprite.setTextureRect(IntRect(48 + 45 * int(currentFrame), 0, 44, 53));
		if (dx == 0 && onGround == true) sprite.setTextureRect(IntRect(0, 0, 44, 53));
		if(dy != 0 && onGround == false) sprite.setTextureRect(IntRect(52, 80, 44, 66));
		if (health <= 0) { life = false; } 
		if (!isMove) { speed = 0; }
		if (life) { setPlayerCoordinateForView(x, y); }
		dy = dy + 0.0015*time;
	}
};


class Enemy :public Entity {
public:
	float currentFrame;
	Enemy(Image &image, String Name, Level &lvl, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
		obj = lvl.GetObjects("solid");
		if (name == "mob1") {
			health = 3;
			sprite.setTextureRect(IntRect(28, 0, w, h));
			dx = 0.05;
		}
		if (name == "mob2") {
			life = 5;
			sprite.setTextureRect(IntRect(1, 1, w, h));
			dx = 0.05;
		}
	}



	void checkCollisionWithMap(float Dx, float Dy)//ф ция проверки столкновений с картой
	{

		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
			{
				if (obj[i].name == "solid")
				{
					if (Dy>0) { y = obj[i].rect.top - h;  dy = 0; onGround = true; }
					if (Dy<0) { y = obj[i].rect.top + obj[i].rect.height;   dy = 0; }
					if (Dx>0) { x = obj[i].rect.left - w; }
					if (Dx<0) { x = obj[i].rect.left + obj[i].rect.width; }
				}
				else { onGround = false; }
			}

	}

	void update(float time)
	{
		if (name == "mob1" || name == "Mob2") {//для персонажа с таким именем логика будет такой

			moveTimer += time;
			if (moveTimer > 3000) {
				dx *= -1; moveTimer = 0; if (dx < 0)sprite.setScale(-1, 1); else sprite.setScale(1, 1);
			}//меняет направление примерно каждые 3 сек
			checkCollisionWithMap(dx, 0);//обрабатываем столкновение по Х
			x += dx*time;
			y += dy*time;
			checkCollisionWithMap(0, dy);
			if (onGround != true) dy = 0.2;
			currentFrame += time * 0.003;
			if (currentFrame > 3) { currentFrame -= 3; };
			if (dx > 0 && onGround == true) sprite.setTextureRect(IntRect(28 + 26 * int(currentFrame), 0, 26, 48));
			if (dx < 0 && onGround == true) sprite.setTextureRect(IntRect(28 + 26 * int(currentFrame), 0, 26, 48));
			sprite.setPosition(x + w / 2, y + h / 2); //задаем позицию спрайта в место его центра
			if (health <= 0) { life = false; }
		}
	}
};



class Bullet :public Entity//класс пули
{
public:
	float targetX, targetY;//отвечают за направление движения пули к цели
	float vx, vy, distance;
	Bullet(Image &image, String Name, Level &lvl, float X, float Y, int W, int H, float tX, float tY) :Entity(image, Name, X, Y, W, H) {//всё так же, только взяли в конце состояние игрока (int dir)
		obj = lvl.GetObjects("solid");//инициализируем .получаем нужные объекты для взаимодействия пули с картой
		x = X;
		y = Y;
		targetX = tX;
		targetY = tY;
		speed = 0.2;
		w = h = 16;
		life = true;
		distance = sqrt((targetX - x) - (targetY - y));
		vx = (targetX - x) / 100; // 100 - дистанция
		vy = (targetY - y) / 100;
		//выше инициализация в конструкторе
	}


	void update(float time)
	{

		//x += speed*time*(targetX - x) /20;//само движение пули по х
		//y += speed*time*(targetY - y) /20;//по у

		x += vx*time*speed;
		y += vy*time*speed;

		if (x <= 0) x = 1;// задержка пули в левой стене, чтобы при проседании кадров она случайно не вылетела за предел карты и не было ошибки
		if (y <= 0) y = 1;

		for (int i = 0; i < obj.size(); i++) {//проход по объектам solid
			if (getRect().intersects(obj[i].rect)) //если этот объект столкнулся с пулей,
			{
				life = false;// то пуля умирает
			}
		}

		sprite.setPosition(x + w / 2, y + h / 2);//задается позицию пуле
	}
};

class Obstacle :public Entity {
public:
	Obstacle(Image &image, String Name, Level &lvl, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
		if (name == "Spike") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
		}
		if (name == "Water") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
		}
		if (name == "Finish") {
			sprite.setTextureRect(IntRect(0, 0, w, h));
		}
	}
};

	

class MovingPlatform : public Entity {//класс движущейся платформы
public:
	MovingPlatform(Image &image, String Name, Level &lvl, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {	
		sprite.setTextureRect(IntRect(0, 0, W, H));//прямоугольник 
		obj = lvl.GetObjects("solid");
		dx = 0;//изначальное ускорение по Х
		flg = 0;
	}

	void checkCollisionWithMap(float dx)
	{
		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
			{
				if (obj[i].name == "solid")
				{
					if (dx < 0) { x = obj[i].rect.left + obj[i].rect.width; x += 5;  flg = 0; }
					if (dx > 0) { x = obj[i].rect.left - w;  x -= 5;  flg = 1; }
					
				}
				// else { onGround = false; }
			}
	}

	
	void update(float time)//функция обновления платформы.
	{
		x += dx*time;
		checkCollisionWithMap(dx); 
			
				
				
		
		
		sprite.setPosition(x + w / 2, y + h / 2);//задаем позицию спрайту
	}
};

class FallingPlatform : public Entity {//класс движущейся платформы
public:
	FallingPlatform(Image &image, String Name, Level &lvl, float X, float Y, int W, int H) :Entity(image, Name, X, Y, W, H) {
		sprite.setTextureRect(IntRect(0, 0, W, H));//прямоугольник 
		dx = 0;//изначальное ускорение по 
		onGround = true;
	}

	
	void update(float time)//функция обновления платформы.
	{
		x += dx*time;
		
		y += dy*time;
		
		//moveTimer += time;//наращиваем таймер
		//if (moveTimer>2000) { dx *= -1; moveTimer = 0; }//если прошло примерно 2 сек, то меняется направление движения платформы, а таймер обнуляется
		sprite.setPosition(x + w / 2, y + h / 2);//задаем позицию спрайту
	}
};


void changeLevel(Level &lvl, int &numberLevel) {
	if (numberLevel == 1) { lvl.LoadFromFile("map.tmx"); }
	if (numberLevel == 2) { lvl.LoadFromFile("map2.tmx"); }
	if (numberLevel == 3) { lvl.LoadFromFile("maptest.tmx"); }
	if (numberLevel > 3) { numberLevel = 1; lvl.LoadFromFile("map.tmx"); }
}

bool startGame(RenderWindow &window, int &numberLevel) {

	view.reset(FloatRect(0, 0, 640, 480));

	Level lvl;
	changeLevel(lvl, numberLevel);//для загрузки карты для нужного уровня

	SoundBuffer shootBuffer;//создаём буфер для звука
	shootBuffer.loadFromFile("shoot.ogg");//загружаем в него звук
	Sound shoot(shootBuffer);//создаем звук и загружаем в него звук из буфера

	Music music;//создаем объект музыки
	music.openFromFile("music.ogg");//загружаем файл
	music.play();//воспроизводим музыку
	music.setLoop(true);


	Image heroImage;
	heroImage.loadFromFile("images/MasterChiefOriginal.png");

	Image mob1Image;
	mob1Image.loadFromFile("images/mob1.png");
	mob1Image.createMaskFromColor(Color(255, 0, 0));

	/*Image Mob2Image;
	mob1Image.loadFromFile("images/mob2.png");
	mob1Image.createMaskFromColor(Color(255, 0, 0));*/

	/*Image Mob3Image;
	mob1Image.loadFromFile("images/Mob3.png");
	mob1Image.createMaskFromColor(Color(255, 0, 0));*/

	Image movePlatformImage;
	movePlatformImage.loadFromFile("images/MovingPlatform.png");

	Image fallPlatformImage;
	fallPlatformImage.loadFromFile("images/FallingPlatform.png");

	Image WaterImage;
	WaterImage.loadFromFile("images/Water.png");

	Image SpikeImage;
	SpikeImage.loadFromFile("images/Spike.png");

	Image FinishImage;
	FinishImage.loadFromFile("images/Door.png");

	Image BulletImage;//изображение для пули
	BulletImage.loadFromFile("images/bullet.png");//загрузили картинку в объект изображения
	BulletImage.createMaskFromColor(Color(0, 0, 0));//маска для пули по черному цвету

	std::list<Entity*>  entities;
	std::list<Entity*>::iterator it;
	std::list<Entity*>::iterator it2;//второй итератор.для взаимодействия между объектами списка


	std::vector<Object> e = lvl.GetObjects("mob1");
	for (int i = 0; i < e.size(); i++) entities.push_back(new Enemy(mob1Image, "mob1", lvl, e[i].rect.left, e[i].rect.top, 26, 48));

	e = lvl.GetObjects("Mob2");
	for (int i = 0; i < e.size(); i++) entities.push_back(new Enemy(mob1Image, "Mob2", lvl, e[i].rect.left, e[i].rect.top, 200, 97));

	e = lvl.GetObjects("Mob3");
	for (int i = 0; i < e.size(); i++) entities.push_back(new Enemy(mob1Image, "Mob3", lvl, e[i].rect.left, e[i].rect.top, 200, 97));

	Object player = lvl.GetObject("player");

	Player p(heroImage, "player", lvl, player.rect.left, player.rect.top, 42, 53);

	e = lvl.GetObjects("MovingPlatform");//забираем все платформы в вектор 
	for (int i = 0; i < e.size(); i++) entities.push_back(new MovingPlatform(movePlatformImage, "MovingPlatform", lvl, e[i].rect.left, e[i].rect.top, 128, 32));

	e = lvl.GetObjects("FallingPlatform");
	for (int i = 0; i < e.size(); i++) entities.push_back(new FallingPlatform(fallPlatformImage, "FallingPlatform", lvl, e[i].rect.left, e[i].rect.top, 192, 32));

	e = lvl.GetObjects("Spike");
	for (int i = 0; i < e.size(); i++) entities.push_back(new FallingPlatform(SpikeImage, "Spike", lvl, e[i].rect.left, e[i].rect.top, 64, 32));

	e = lvl.GetObjects("Water");                // по неизвестным причинам не определяется класс obstacle, но и так работает
	for (int i = 0; i < e.size(); i++) entities.push_back(new FallingPlatform(WaterImage, "Water", lvl, e[i].rect.left, e[i].rect.top, 64, 32));

	e = lvl.GetObjects("Finish");
	for (int i = 0; i < e.size(); i++) entities.push_back(new FallingPlatform(FinishImage, "Finish", lvl, e[i].rect.left, e[i].rect.top, 64, 32));

	Clock clock;
	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asMicroseconds();

		clock.restart();
		time = time / 800;

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == Event::MouseButtonPressed)//если нажата клавиша мыши
				if (event.key.code == Mouse::Left) //а именно левая, то стреляем 
				{ //снимаем координаты мыши
					//Vector2i pixelPos = Mouse::getPosition(window);//забираем коорд курсора
					//Vector2f pos = window.mapPixelToCoords(pixelPos);//переводим их в игровые (уходим от коорд окна)
					//entities.push_back(new Bullet(BulletImage, "Bullet", lvl, p.x, p.y, 16, 16, pos.x, pos.y)); //и передаем в конструктор пули. создается пуля
					shoot.play();//играем звук пули
				}
		}

		for (it = entities.begin(); it != entities.end();)//говорим что проходимся от начала до конца
		{
			Entity *b = *it;//для удобства, чтобы не писать (*it)->
			b->update(time);//вызываем ф-цию update для всех объектов (по сути для тех, кто жив)
			if (b->life == false) { it = entities.erase(it); delete b; }// если этот объект мертв, то удаляем его
			else it++;//и идем курсором (итератором) к след объекту. так делаем со всеми объектами списка
		}

		for (it = entities.begin(); it != entities.end(); it++)//проходимся по эл-там списка
		{
			if (((*it)->name == "MovingPlatform")){
				Entity *movPlat = *it;
				if ((*it)->flg == 0) { (*it)->dx = 0.03; }
				if ((*it)->flg == 1) { (*it)->dx = -0.03; }
			}
			if (((*it)->name == "MovingPlatform") && ((*it)->getRect().intersects(p.getRect())))//если игрок столкнулся с объектом списка и имя этого объекта movingplatform
			{
				Entity *movPlat = *it;
				if ((p.dy>0) || (p.onGround == false))//при этом игрок находится в состоянии после прыжка, т.е падает вниз
					if (p.y + p.h<movPlat->y + movPlat->h)//если игрок находится выше платформы, т.е это его ноги минимум (тк мы уже проверяли что он столкнулся с платформой)
					{
						p.y = movPlat->y - p.h + 3; p.x += movPlat->dx*time; p.dy = 0; p.onGround = true; // то выталкиваем игрока так, чтобы он как бы стоял на платформе

					}

				
			}
			
			if (((*it)->name == "Water") && ((*it)->getRect().intersects(p.getRect())))//если игрок столкнулся с объектом списка и имя этого объекта movingplatform
			{
				Entity *movPlat = *it;
				if ((p.dy>0) || (p.onGround == false))//при этом игрок находится в состоянии после прыжка, т.е падает вниз
					if (p.y + p.h<movPlat->y + movPlat->h)//если игрок находится выше платформы, т.е это его ноги минимум (тк мы уже проверяли что он столкнулся с платформой)
					{
						p.life = false;  return true;

					}


			}

			if (((*it)->name == "Spike") && ((*it)->getRect().intersects(p.getRect())))//если игрок столкнулся с объектом списка и имя этого объекта movingplatform
			{
						p.life = false;  return true;
			}

			if (((*it)->name == "Finish") && ((*it)->getRect().intersects(p.getRect())))//если игрок столкнулся с объектом списка и имя этого объекта movingplatform
			{
				numberLevel++;  return true;		
			}

			if (((*it)->name == "FallingPlatform") && ((*it)->getRect().intersects(p.getRect())))//если игрок столкнулся с объектом списка и имя этого объекта movingplatform
			{
				Entity *falPlat = *it;
				if ((p.dy>0) || (p.onGround == false))//при этом игрок находится в состоянии после прыжка, т.е падает вниз
					if (p.y + p.h<falPlat->y + falPlat->h)//если игрок находится выше платформы, т.е это его ноги минимум (тк мы уже проверяли что он столкнулся с платформой)
					{
						p.y = falPlat->y - p.h + 3; p.x += falPlat->dx*time; p.dy = 0; p.onGround = true; // то выталкиваем игрока так, чтобы он как бы стоял на платформе
						(*it)->dy = 0.6;
					}
			}

			if (((*it)->name == "mob1") && ((*it)->getRect().intersects(p.getRect())))
			{
				p.health--;
			}


			if (((*it)->name == "mob3") && ((*it)->getRect().intersects(p.getRect())))
			{
				(*it)->life = false;
			}


			if (((*it)->name == "mob2") && ((*it)->getRect().intersects(p.getRect())))
			{
				(*it)->life = false;
			}

				if (p.health <=0)
			{
				p.life = false;
			}

			for (it2 = entities.begin(); it2 != entities.end(); it2++)
			{
				if ((*it)->getRect() != (*it2)->getRect())//при этом это должны быть разные прямоугольники
					if (((*it)->getRect().intersects((*it2)->getRect())) && ((*it)->name == "mob1") && ((*it2)->name == "mob1"))//если столкнулись два объекта и они враги
					{
						(*it)->dx *= -1;//меняем направление движения врага
						(*it)->sprite.scale(-1, 1);//отражаем спрайт по горизонтали
					}
				if (((*it)->getRect().intersects((*it2)->getRect())) && ((*it)->name == "mob1") && ((*it2)->name == "bullet"))//если столкнулись два объекта и они враги
				{
					(*it)->life = false;
					(*it)->sprite.scale(-1, -1);
				}
			}

		}

		if (Keyboard::isKeyPressed(Keyboard::T)) { numberLevel++; return true; }
		if (Keyboard::isKeyPressed(Keyboard::Tab)) { return true; }//если таб, то перезагружаем игру
		if (Keyboard::isKeyPressed(Keyboard::Escape)) { window.close(); return 0; }//если эскейп, то выходим из игры



		p.update(time);//перенесли сюда update игрока
		window.setView(view);
		window.clear(Color(77, 83, 140));
		lvl.Draw(window);

		
		for (it = entities.begin(); it != entities.end(); it++) {
			window.draw((*it)->sprite);
		}
		window.draw(p.sprite);
		window.display();
	}
}

void gameRunning(RenderWindow & window, int & numberLevel) {//ф-ция перезагружает игру , если это необходимо
	if (startGame(window, numberLevel)) { gameRunning(window, numberLevel); }//принимает с какого уровня начать игру
}

int main()
{
	RenderWindow window(VideoMode(640, 480), "game");
	menu(window);
	int numberLevel = 1;//сначала 1-ый уровень
	gameRunning(window, numberLevel);
	return 0;
}
