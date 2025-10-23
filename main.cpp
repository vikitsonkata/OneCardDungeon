#include <iostream>
#include <random>
#include <sstream>
#include <iostream>
#include <thread>

int rng(int range)
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution( 0, range - 1 );
    return (distribution(generator));
}
int rollDie(int sides = 6)
{
    return rng(sides) + 1;
}

enum class colorCode
{
    black = 0,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,
    normal = 9
};

constexpr int DELAY = 100;
void wait(int delayTime)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
}
template<class T>
void log(T message, std::string divider = "\n",
         colorCode fontColor = colorCode::black,
         colorCode backgroudColor = colorCode::normal,
         int delayTime = DELAY)
{
    std::cout
        << "\e[3" + std::to_string(int(fontColor)) + "m"
              << "\e[10" + std::to_string(int(backgroudColor)) + "m"
              << message << divider << "\e[49m";
    wait(delayTime);
}
template<class T>
void log(T message, colorCode fontColor)
{
    std::cout << "\e[3" + std::to_string(int(fontColor)) + "m"
              << message << "\n\e[49m";
}
void log(std::string message = "", std::string divider = "\n",
         colorCode fontColor = colorCode::black)
{
    log<std::string>(message, divider, fontColor);
}
void clearScrean()
{
    log("\x1b[2J\x1b[H");
}

enum class cell
{
    empty, wall, hero, enemy
};
char CellToDraw(const cell& c)
{
    switch (c) {
    case cell::empty:
        return '.';
    case cell::wall:
        return '#';
    case cell::hero:
        return 'H';
    case cell::enemy:
        return '@';
    default:
        return'?';
    }
}
enum class direction
{
    left, right, up, down,
    leftUp, rightUp, leftDown, rightDown
};

struct coord
{
    int x, y;
    coord(int xx = 0, int yy = 0)
    {
        x = xx;
        y = yy;
    }
    coord direction()
    {
        coord pos;
        if(x) pos.x = x/abs(x);
        if(y) pos.y = y/abs(y);
        return pos;
    }
    double distance(coord to = coord(0,0))
    {
        coord radius(to - coord(x, y));
        return sqrt(radius.x*radius.x + radius.y*radius.y);
    }
    bool isAdjacent(coord to, int atthackRange = 2)
    {
        return distance(to) <= 0.5 * atthackRange;
    }

    coord operator+ (const coord& other) const
    {
        return {x + other.x, y + other.y};
    }
    void operator+= (const coord& other)
    {
        x += other.x;
        y += other.y;
    }
    coord operator- (const coord& other) const
    {
        return {x - other.x, y - other.y};
    }
    void operator-= (const coord& other)
    {
        x -= other.x;
        y -= other.y;
    }
    friend std::ostream& operator<< (std::ostream& os, coord pos)
    {
        os << pos.y << ' ' << pos.x;
        return os;
    }
};

int MAX_ROW = 5;
int MAX_COL = 5;
class Field
{
public:
    Field()
    {
        for(int i = 0; i < MAX_ROW; i++)
        {
            grid.push_back(std::vector<cell>());
            for(int j = 0; j < MAX_COL; j++)
            {
                grid.at(i).push_back(cell::empty);
            }
        }
    }
    void AddWall(coord pos)
    {
        grid.at(pos.y).at(pos.x) = cell::wall;
    }
    void SetCell(coord pos, cell type)
    {
        grid.at(pos.y).at(pos.x) = type;
    }
    cell GetCell(coord pos)
    {
        return grid.at(pos.y).at(pos.x);
    }
    bool isFree(coord pos)
    {
        return grid.at(pos.y).at(pos.x) == cell::empty;
    }
    void Print(colorCode color = colorCode::normal) const
    {
        std::stringstream ss;
        for(auto &row : grid)
        {
            for(auto &el : row)
            {
                ss << '|' << CellToDraw(el);
            }
            ss << "|\n";
        }
        log(ss.str(), color);
    }
    bool Move(coord from, coord to)
    {
        if(to.x >= 0 && to.x < MAX_ROW &&
            to.y >= 0 && to.y < MAX_COL)
        {
            std::swap(grid.at(from.y).at(from.x),
                      grid.at(to.y).at(to.x));
            return true;
        }
        return false;
    }

private:
    // 5x5
    std::vector<std::vector<cell>> grid;
};

struct Stats
{
    int health;

    int move;
    int attack;
    int defence;
    int range;

    Stats(int hp = 6, int mov = 1, int att = 1, int def = 1, int ran = 2)
    {
        health = hp;
        move = mov;
        attack = att;
        defence = def;
        range = ran;
    }
    void PrintHP()  const
    {
        log("HP:", " ");
        log(health);
    }
    friend std::ostream& operator<<(std::ostream& os, Stats stats)
    {
        os << "HP: " << stats.health
           << "\tM: " << stats.move
           << "\tA: " << stats.attack
           << "\tD: " << stats.defence
           << "\tR: " << stats.range
           << "\n";
        return os;
    }
};

class Character
{
public:
    Character(std::string name = "NoName")
    {
        this->name = name;
    }
    void SetPosition(int x, int y)
    {
        pos = {x, y};
    }
    void SetPosition(coord pos)
    {
        this->pos = pos;
    }
    coord GetPos() const
    {
        return pos;
    }
    void SetStats(Stats newStat)
    {
        stats = newStat;
    }
    void Buff(char type)
    {
        switch (type) {
        case 'm':
            stats.move++;
            break;
        case 'a':
            stats.attack++;
            break;
        case 'd':
            stats.defence++;
            break;
        case 'r':
            stats.range++;
            break;
        case 'h':
        default:
            stats.health = 6;
            break;
        }
    }
    Stats GetStats() const
    {
        return stats;
    }
    const std::string& GetName() const
    {
        return name;
    }
    void Move(direction d)
    {
        if(stats.move < 2 ||
            (stats.move < 3 && d >= direction::leftUp))
        {
            log("not enough movement");
            return;
        }
        if(((d == direction::left || d == direction::leftUp || d == direction::leftDown) && pos.x == 0) ||
            ((d == direction::right || d == direction::rightUp || d == direction::rightDown) && pos.x == MAX_COL-1) ||
            ((d == direction::up || d == direction::leftUp || d == direction::rightUp) && pos.y == 0) ||
            ((d == direction::down || d == direction::leftDown || d == direction::rightDown) && pos.y == MAX_ROW-1))
        {
            log("coordinates out of map, nothing happen");
            return;
        }

        switch (int(d)) {
        case int(direction::left):      pos.x--;
        case int(direction::right):     pos.x++;
        case int(direction::up):        pos.y--;
        case int(direction::down):      pos.y++;
            stats.move -= 2;
            break;
        case int(direction::leftUp):    pos.x--; pos.y--;
        case int(direction::leftDown):  pos.x--; pos.y++;
        case int(direction::rightUp):   pos.x++; pos.y--;
        case int(direction::rightDown): pos.x++; pos.y++;
            stats.move -= 3;
            break;
        }
    }
    int operator+ (Character other)
    {
        return stats.attack + other.stats.attack;
    }
    void Defend(int attackDamage)
    {
        int damage = attackDamage/stats.defence;
        stats.health -= damage;
        switch (damage) {
        case 0:
            log(") Defended (", colorCode::yellow);
            break;
        case 1:
            log("> Damaged <", colorCode::blue);
            break;
        case 2:
            log(">> Smashed <<", colorCode::blue);
            break;
        default:
            if(stats.health <= 0)
                log("## " + name+" is dead ##", colorCode::red);
            else
                log("** Lethaly wounded **", colorCode::red);
            break;
        }
    }
    void Print() const
    {
        log(name);
        log<Stats>(stats);
    }
protected:
    std::string name;
    coord pos;
    Stats stats;
};

class Monster : public Character
{
public:
    Monster(std::string name, Stats stats, coord pos)
        :Character(name)
    {
        this->stats = stats;
        this->pos = pos;
    }
    void Move(coord to, Field& field)
    {
        int speed = stats.move;
        while(speed > 1)
        {

            coord step = to - pos;

            // adjacent
            if(step.distance() <= stats.range * 0.5)
            {
                // line of sight
                bool sight = true;
                if(step.x == 0)
                {
                    for(int i = 0; i < step.y; i++)
                    {
                        if(!field.isFree({pos.x, pos.y+i}))
                        {
                            sight = false;
                        }
                    }
                }
                if(step.y == 0)
                {
                    for(int i = 0; i < step.x; i++)
                    {
                        if(!field.isFree({pos.x+i, pos.y}))
                        {
                            sight = false;
                        }
                    }
                }
                if(step.x == step.y)
                {
                    for(int i = 0; i < step.x; i++)
                    {
                        if(!field.isFree({pos.x+i, pos.y+i}))
                        {
                            sight = false;
                        }
                    }
                }
                if(sight) return;
            }

            if(step.x != 0 && step.y != 0
                && speed > 2 && field.isFree(pos + step.direction()))
            {
                log("\tMonster move diagonal");
                pos += step.direction();
                step -= step.direction();
                speed -= 3;
            }
            else if((step.x != 0 || step.y != 0))
            {
                //priority horizontal movement
                if(step.x != 0 && field.isFree(pos + coord(step.direction().x, 0)))
                {
                    log("\tMonster move horizontal");
                    pos.x += step.direction().x;
                    step.x -= step.direction().x;
                    speed -= 2;
                }
                else if(step.y != 0 && field.isFree(pos + coord(0, step.direction().y)) && speed > 1)
                {
                    log("\tMonster move vertical");
                    pos.y += step.direction().y;
                    step.y -= step.direction().y;
                    speed -= 2;
                }
                else
                {
                    log("\tMonster don't know");
                    if(step.x > step.y)
                        step.y = 0;
                    else
                        step.x = 0;

                    int random = rng(2);
                    if(random)
                    {

                    }
                    return;
                }
            }
            else
            {
                log("\tMonster don't move");
                return;
            }
            log("\t" + name + " is at ", " ");
            log(pos);
        }
    }
};

class Hero : public Character
{
public:
    Hero(std::string name = "Hero")
        :Character(name)
    {
//        baseStats = Stats(6, 1, 1, 1, 2);
    }

    void ResetStatsToBase()
    {
        int currHP = stats.health;
        stats = baseStats;
        stats.health = currHP;
    }
    void RollDice(int count = 3)
    {
        std::vector<int> dies;
        for(int i = 0; i < count; i++)
        {
            dies.push_back(rollDie());
        }

        ResetStatsToBase();
        PrintStats();
        for(auto &die : dies)
        {
            log(die, " ");
        }
        log("\t: RNG");

        for(int i = 0; i < count; i++)
        {
            log("Set " + std::to_string(i+1) +" die to S/A/D - Speed/Attack/Defence:", " ");
            char choice;
            std::cin >> choice;
            switch (choice) {
            case '1':
            case 'S':
            case 's':
                stats.move = baseStats.move + dies.at(i);
                break;
            case '2':
            case 'A':
            case 'a':
                stats.attack = baseStats.attack + dies.at(i);
                break;
            case '3':
            case 'D':
            case 'd':
                stats.defence = baseStats.defence + dies.at(i);
                break;
            default:
                log("\t ... invalid, AUTO set");
                stats.move = baseStats.move + dies.at(0);
                stats.attack = baseStats.attack + dies.at(1);
                stats.defence = baseStats.defence + dies.at(2);
                PrintStats();
                return;
            }
        }
        log();
        PrintStats();
    }

    void Move(Field& f)
    {
        while (stats.move > 1)
        {
            coord posBegin = pos;
            log("move left:", " ");
            log(stats.move);
            log("Numpad to move, 5 to stay");
            char key;
            std::cin >> key;
            switch (key) {
            case '1':
                pos.x--;
                pos.y++;
                stats.move -= 3;
                break;
            case 's':
            case 'S':
            case '2':
                pos.y++;
                stats.move -= 2;
                break;
            case '3':
                pos.x++;
                pos.y++;
                stats.move -= 3;
                break;
            case 'a':
            case 'A':
            case '4':
                pos.x--;
                stats.move -= 2;
                break;
            case 'd':
            case 'D':
            case '6':
                pos.x++;
                stats.move -= 2;
                break;
            case '7':
                pos.x--;
                pos.y--;
                stats.move -= 3;
                break;
            case 'w':
            case 'W':
            case '8':
                pos.y--;
                stats.move -= 2;
                break;
            case '9':
                pos.x++;
                pos.y--;
                stats.move -= 3;
                break;
            default:

                return;
            }
            if(!f.Move(posBegin, pos))
            {
                pos = posBegin;
                log("... out of map, sorry");
            }
            f.Print();
        }
    }

    void Attack(std::vector<Monster>& enemies, Field& field)
    {
        log("Hero Attack!", colorCode::cyan);
        std::vector<Monster*> closeMonsters;
        for(auto& monster : enemies)
        {
            if(monster.GetPos().isAdjacent(pos, stats.range))
            {
                log(monster.GetName(), "", colorCode::red);
                log(" HP: " + std::to_string(monster.GetStats().health));
                closeMonsters.push_back(&monster);
            }
        }
        Monster* attacked;
        if(closeMonsters.size() > 1)
        {
            log("Choose monster to attack (0,1,..)");
            int num;
            std::cin >> num;
            if(num < closeMonsters.size())
            {
                attacked = closeMonsters.at(num);
            }
            else
            {
                attacked = closeMonsters.front();
            }
        }
        else if(closeMonsters.size() == 1)
        {
            attacked = closeMonsters.front();
        }
        else
        {
            log("... nothing");
            return;
        }

        attacked->Defend(stats.attack);
        if(attacked->GetStats().health <= 0)
        {
            for(int i = 0; i < enemies.size(); i++)
            {
                if(&enemies.at(i) == attacked)
                {
                    field.SetCell(enemies.at(i).GetPos(), cell::empty);
                    enemies.erase(enemies.begin() + i);
                }
            }
        }
        else
        {
            attacked->Print();
        }

    }
    void PrintStats() const
    {
        log("Your stats: ");
        log(stats.move, " ");
        log(stats.attack, " ");
        log(stats.defence, " ");
        log(stats.range);
    }
private:
    Stats baseStats;
};

class Level
{
public:
    Level(Hero& myHero, int number): id(number)
    {
        log("LEVEL " + std::to_string(number), colorCode::green);
        color = colorCode(id);

        coord begin(0, MAX_ROW-1);
        field.SetCell(begin, cell::hero);
        hero = myHero;
        hero.SetPosition(begin);
        hero.Print();
    }

    void HeroTurn()
    {
        log("Hero turn!", colorCode::cyan);
        log("HP: " + std::to_string(hero.GetStats().health));
        hero.RollDice();
        hero.Move(field);
        hero.Attack(enemies, field);
    }
    bool EnemiesTurn()
    {
        log();
        log("Enemies turn!", colorCode::red);
        for(auto &i : enemies)
        {
            coord pos = i.GetPos();
            i.Move(hero.GetPos(), field);
            field.Move(pos, i.GetPos());
        }
        int attackDamage = 0;
        for(auto &monster : enemies)
        {
            if(monster.GetPos().isAdjacent(hero.GetPos(), monster.GetStats().range))
            {
                attackDamage += monster.GetStats().attack;
                log(monster.GetName() + " attack");
            }
        }
        if(attackDamage)
        {
            log("Enemies attack is " + std::to_string(attackDamage), colorCode::red);
            log("Hero defends is " + std::to_string(hero.GetStats().defence));
            hero.Defend(attackDamage);
        }
        if(hero.GetStats().health <= 0)
        {
            log("Game over!", colorCode::magenta);
            return true;
        }
        return false;
    }

    void AddWall(coord pos)
    {
        field.AddWall(pos);
    }
    void AddEnemy(std::string name, Stats stats, coord pos)
    {
        field.SetCell(pos, cell::enemy);
        enemies.push_back(Monster(name, stats, pos));
    }
    void PrintEnemies() const
    {
        for(auto &i : enemies)
        {
            i.Print();
        }
    }
    bool isClear() const
    {
        if(enemies.empty())
        {
            log("^^ You WIN! ^^", colorCode::green);
            return true;
        }
        return false;
    }

    const Field& GetField() {return field;}
    Hero& GetHero() {return hero;}
    colorCode GetColor() {return color;}

private:
    int id;
    colorCode color;
    Field field;
    Hero hero;
    std::vector<Monster> enemies;

};

int main() {

    Hero adventurer("Viktor");
    adventurer.Buff('r');
    adventurer.Buff('r');
    adventurer.Buff('r');
    std::vector<Level> levels;
    levels.push_back(Level(adventurer, 1));
    levels.back().AddWall({1, 1});
    levels.back().AddWall({3, 3});
    levels.back().AddWall({3, 1});
    levels.back().AddEnemy("Spider 1", {2, 4, 4, 4, 2}, {3, 0});
    levels.back().AddEnemy("Spider 2", {2, 4, 4, 4, 2}, {4, 3});

    levels.push_back(Level(adventurer, 2));
    levels.back().AddWall({3, 1});
    levels.back().AddWall({3, 2});
    levels.back().AddWall({0, 2});
    levels.back().AddEnemy("Skelet archer 1", {3, 4, 5, 4, 4}, {4, 3});
    levels.back().AddEnemy("Skelet archer 2", {3, 4, 5, 4, 4}, {1, 0});

    levels.push_back(Level(adventurer, 3));
    levels.back().AddWall({1, 1});
    levels.back().AddWall({3, 3});
    levels.back().AddWall({1, 3});
    levels.back().AddEnemy("Minotaur 1", {5, 3, 7, 7, 2}, {4, 1});

    levels.push_back(Level(adventurer, 4));
    levels.back().AddWall({1, 1});
    levels.back().AddWall({1, 2});
    levels.back().AddWall({4, 2});
    levels.back().AddEnemy("Dragon 1", {5, 4, 5, 5, 5}, {1, 0});

    auto currLevel = levels.begin();
    while(currLevel->GetHero().GetStats().health > 0)
    {
        currLevel->GetField().Print(currLevel->GetColor());
        currLevel->PrintEnemies();
        currLevel->HeroTurn();
        if(currLevel->isClear())
        {
            if(currLevel != levels.end())
            {
                log("Upgrade hero?", colorCode::green);
                log("Select h(raise health to 6) or m/a/d/r (to buff stat)");
                char buff;
                std::cin >> buff;
                currLevel->GetHero().Buff(buff);
                currLevel = std::next(currLevel);
            }
            else
            {
                log("...", colorCode::green);
                log("...", colorCode::green);
                log("... THE END", colorCode::green);
            }
        }
        if(currLevel->EnemiesTurn())
        {
            log(".. You lost! ..", colorCode::red);
            break;
        }
        wait(20*DELAY);
        clearScrean();
    }

    return 0;
}
