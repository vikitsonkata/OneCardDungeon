#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <iostream>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif

void enableAnsiColors() {
#ifdef _WIN32
    // Get the console handle
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Enable ANSI escape sequences
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}
int OS() {
#if defined(_WIN32)
    std::cout << "Running on Windows (32-bit or 64-bit)\n";
    return 1;
#elif defined(__APPLE__)
    std::cout << "Running on macOS\n";
    return 2;
#elif defined(__linux__)
    std::cout << "Running on Linux\n";
    return 3;
#elif defined(__unix__)
    std::cout << "Running on a Unix-like system\n";
    return 4;
#elif defined(__ANDROID__)
    std::cout << "Running on Android\n";
    return 5;
#else
    std::cout << "Unknown operating system\n";
    return 0;
#endif
}

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

const int DELAY = 100;
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
    std::cout << "\e[3" + std::to_string(int(fontColor)) + "m"
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
void log(int message, std::string divider = "\n",
         colorCode fontColor = colorCode::black)
{
    log<int>(message, divider, fontColor);
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
    double distance(coord to = coord(0,0)) const
    {
        coord radius(*this - to);
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
    bool operator< (const coord& other) const
    {
        return (distance() < other.distance());
    }
    bool operator<= (const coord& other) const
    {
        return (distance() <= other.distance());
    }
    bool operator== (const coord& other) const
    {
        return (x==other.x && y==other.y);
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
        grid.at(pos.x).at(pos.y) = cell::wall;
    }
    void SetCell(coord pos, cell type)
    {
        grid.at(pos.x).at(pos.y) = type;
    }
    cell GetCell(coord pos)
    {
        return grid.at(pos.x).at(pos.y);
    }
    bool isFree(coord pos) const
    {
        if(pos.x < 0 || pos.x >= MAX_ROW)
            return false;
        if(pos.y < 0 || pos.y >= MAX_COL)
            return false;
        return grid.at(pos.x).at(pos.y) == cell::empty;
    }
    void Print(colorCode color = colorCode::normal) const
    {
        std::stringstream ss;
        int enemyID = 0;
        for(auto &row : grid)
        {
            for(auto &el : row)
            {
                ss << '|';
                if(el == cell::enemy)
                {
                    ss << enemyID;
                    enemyID++;
                }
                else
                {
                    ss << CellToDraw(el);
                }
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
            std::swap(grid.at(from.x).at(from.y),
                      grid.at(to.x).at(to.y));
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
    void SetStat(char type, int val)
    {
        switch (type) {
        case 'h':
            stats.health = val;
            break;
        case 'm':
            stats.move = val;
            break;
        case 'a':
            stats.attack = val;
            break;
        case 'd':
            stats.defence = val;
            break;
        case 'r':
            stats.range = val;
            break;
        default:
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
        case int(direction::left):      pos.y--;
        case int(direction::right):     pos.y++;
        case int(direction::up):        pos.x--;
        case int(direction::down):      pos.x++;
            stats.move -= 2;
            break;
        case int(direction::leftUp):    pos.y--; pos.x--;
        case int(direction::leftDown):  pos.y--; pos.x++;
        case int(direction::rightUp):   pos.y++; pos.x--;
        case int(direction::rightDown): pos.y++; pos.x++;
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
            log("** Lethaly wounded **", colorCode::red);
            break;
        if(stats.health <= 0)
            log("## " + name+" is dead ##", colorCode::red);
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

bool lineOfSight(const Field& field, coord from, coord to, int range)
{
    coord step = to - from;

    // in range
    if(step.distance() <= range * 0.5)
    {
        if(step.distance() == 1)
            return true;

        bool sight = true;
        if(step.x == 0)
        {
            for(int i = 1; i < step.y; i++)
            {
                if(!field.isFree({from.x, from.y+i}))
                {
                        sight = false;
                }
            }
        }
        if(step.y == 0)
        {
            for(int i = 1; i < step.x; i++)
            {
                if(!field.isFree({from.x+i, from.y}))
                {
                        sight = false;
                }
            }
        }
        if(step.x == step.y && step.distance() )
        {
            for(int i = 1; i < step.x; i++)
            {
                if(!field.isFree({from.x+i, from.y+i}))
                {
                        sight = false;
                }
            }
        }
        return sight;
    }
    return false;
}
enum class monster
{
    spider,
    skeletonArcher,
    minotaur,
    dragon
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
    Monster(monster type, coord pos)
    {
        switch (type) {
        case monster::spider:
            name = "Spider";
            stats = {2, 5, 4, 4, 2};
            break;
        case monster::skeletonArcher:
            name = "Skeleton";
            stats = {3, 4, 5, 4, 4};
            break;
        case monster::minotaur:
            name = "Minotaur";
            stats = {5, 3, 7, 7, 2};
            break;
        case monster::dragon:
            name = "Dragon";
            stats = {5, 5, 5, 5, 5};
            break;
        default:
            break;
        }
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
            if(step.distance() <= stats.range * 0.5
                && lineOfSight(field, pos, to, stats.range))
            {
                return;
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
                //priority vertical movement
                if(step.x != 0 && field.isFree(pos + coord(step.direction().x, 0)))
                {
                    log("\tMonster move vertical");
                    pos.x += step.direction().x;
                    step.x -= step.direction().x;
                    speed -= 2;
                }
                else if(step.y != 0 && field.isFree(pos + coord(0, step.direction().y)) && speed > 1)
                {
                    log("\tMonster move horizontal");
                    pos.y += step.direction().y;
                    step.y -= step.direction().y;
                    speed -= 2;
                }
                else
                {
                    // TO DO !
                    log("\tMonster don't know");
                    std::vector<coord> adj;
                    for(int i = pos.x-1; i <= pos.x+1; i++)
                        for(int j = pos.y-1; j <= pos.y+1; j++)
                            if(field.isFree({i, j}))
                                adj.push_back({i, j});
                    std::sort(adj.begin(), adj.end());
                    int count = 0;
                    for(auto &p : adj)
                    {
                        if(p.distance(to) == adj.front().distance(to))
                            count++;
                    }
                    int index = rng(count);

                    if(adj.at(index).distance() > 1)
                        speed -= 3;
                    else
                        speed -= 2;
                    pos = adj.at(index);
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

    void Buff(char type)
    {
        switch (type) {
        case 'm':
            baseStats.move++;
            break;
        case 'a':
            baseStats.attack++;
            break;
        case 'd':
            baseStats.defence++;
            break;
        case 'r':
            baseStats.range++;
            break;
        case 'h':
        default:
            baseStats.health = 6;
            break;
        }
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
                pos.y--;
                pos.x++;
                stats.move -= 3;
                break;
            case 's':
            case 'S':
            case '2':
                pos.x++;
                stats.move -= 2;
                break;
            case '3':
                pos.y++;
                pos.x++;
                stats.move -= 3;
                break;
            case 'a':
            case 'A':
            case '4':
                pos.y--;
                stats.move -= 2;
                break;
            case 'd':
            case 'D':
            case '6':
                pos.y++;
                stats.move -= 2;
                break;
            case '7':
                pos.y--;
                pos.x--;
                stats.move -= 3;
                break;
            case 'w':
            case 'W':
            case '8':
                pos.x--;
                stats.move -= 2;
                break;
            case '9':
                pos.y++;
                pos.x--;
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
        log("LEVEL " + std::to_string(id) + " create.", colorCode::green);
        color = colorCode(id);

        coord begin(MAX_ROW-1, number%2 ? 0 : MAX_COL-1);
        field.SetCell(begin, cell::hero);
        hero = myHero;
        hero.SetPosition(begin);
    }
    void Print() const
    {
        log("LEVEL " + std::to_string(id) + " ready!", colorCode::green);
        field.Print(color);
    }
    void HeroTurn()
    {
        log("Hero turn!", colorCode::cyan);
        log("HP: " + std::to_string(hero.GetStats().health));
        hero.RollDice();
        hero.Move(field);
        hero.Attack(enemies, field);
        hero.Move(field);
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
            if(monster.GetPos().isAdjacent(hero.GetPos(), monster.GetStats().range)
                && lineOfSight(field, monster.GetPos(), hero.GetPos(), monster.GetStats().range))
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
    void AddEnemy(monster type, coord pos)
    {
        field.SetCell(pos, cell::enemy);
        enemies.push_back(Monster(type, pos));
    }
    void PrintEnemies() const
    {
        for(int id = 0; id < enemies.size(); id++)
        {
            log(id, " ");
            enemies.at(id).Print();
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
    enableAnsiColors();
    OS();

    Hero adventurer("Viktor");
    std::vector<Level> levels;

   levels.push_back(Level(adventurer, 1));
   levels.back().AddWall({1, 3});
   levels.back().AddWall({3, 3});
   levels.back().AddWall({3, 1});
   levels.back().AddEnemy(monster::spider, {0, 3});
   levels.back().AddEnemy(monster::spider, {2, 4});
    adventurer.SetStat('h', 5);
    adventurer.Buff('a');

    levels.push_back(Level(adventurer, 2));
    levels.back().AddWall({2, 3});
    levels.back().AddWall({3, 3});
    levels.back().AddWall({3, 0});
    levels.back().AddEnemy(monster::skeletonArcher, {1, 0});
    levels.back().AddEnemy(monster::skeletonArcher, {0, 2});

    levels.push_back(Level(adventurer, 3));
    levels.back().AddWall({3, 1});
    levels.back().AddWall({1, 1});
    levels.back().AddWall({1, 3});
    levels.back().AddEnemy(monster::minotaur, {1, 4});

    levels.push_back(Level(adventurer, 4));
    levels.back().AddWall({1, 1});
    levels.back().AddWall({2, 1});
    levels.back().AddWall({2, 4});
    levels.back().AddEnemy(monster::dragon, {0, 1});

    log();
    auto currLevel = levels.begin();
    while(currLevel->GetHero().GetStats().health > 0)
    {
        currLevel->Print();
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
                continue;
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
            currLevel->GetHero().Print();
            continue;
        }
        clearScrean();
    }

    return 0;
}