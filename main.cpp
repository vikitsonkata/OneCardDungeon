#include <iostream>
#include <random>
#include <sstream>

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

template<class T>
void log(T message, std::string divider = "\n")
{
    std::cout << message << divider;
}
void log(std::string message = "", std::string divider = "\n")
{
    log<std::string>(message, divider);
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
    bool isAdjacent(coord to)
    {
        return distance(to) < 1.5;
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
    void Print()
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
        log(ss.str());
    }
    void Move(coord from, coord to)
    {
//        log("from ");
//        log<coord>(from);
//        log("to ");
//        log<coord>(to);
        std::swap(grid.at(from.y).at(from.x), grid.at(to.y).at(to.x));
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
    void PrintHP()
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
    coord GetPos()
    {
        return pos;
    }
    void SetStats(Stats newStat)
    {
        stats = newStat;
    }
    Stats GetStats()
    {
        return stats;
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
        stats.health -= attackDamage/stats.defence;
        if(stats.health <= 0)
            log(name+" is dead");
    }
    const std::string& GetName()
    {
        return name;
    }

    void Print()
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

            if(step.distance() <= stats.range * 0.5)
            {
                // it adjacent
                return;
            }

            if(step.x != 0 && step.y != 0
                && speed > 2 && field.isFree(pos + step.direction()))
            {
                log("monster move diagonal");
                pos += step.direction();
                step -= step.direction();
                speed -= 3;
            }
            else if((step.x != 0 || step.y != 0))
            {
                //                log("monster move ortogonal");
                //priority horizontal movement
                if(step.x != 0 && field.isFree(pos + coord(step.direction().x, 0)))
                {
                    log("monster move horizontal");
                    pos.x += step.direction().x;
                    step.x -= step.direction().x;
                    speed -= 2;
                }
                else if(step.y != 0 && field.isFree(pos + coord(0, step.direction().y)) && speed > 1)
                {
                    log("monster move vertical");
                    pos.y += step.direction().y;
                    step.y -= step.direction().y;
                    speed -= 2;
                }
                else
                {
                    log("monster don't know");
                    return;
                }
            }
            else
            {
                log("monster don't move");
                return;
            }
            log(name, " ");
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

    void RollDice(int count = 3)
    {
        std::vector<int> dies;
        for(int i = 0; i < count; i++)
        {
            dies.push_back(rollDie());
        }

        log("RNG : ");
        for(auto &die : dies)
        {
            log(die, " ");
        }
        stats = baseStats;
        PrintStats();

        for(int i = 0; i < count; i++)
        {
            log("Set " + std::to_string(i+1) +" die to (1-3):", " ");
            int choice;
            std::cin >> choice;
            switch (choice) {
            case 1:
                stats.move = baseStats.move + dies.at(i);
                break;
            case 2:
                stats.attack = baseStats.attack + dies.at(i);
                break;
            case 3:
                stats.defence = baseStats.defence + dies.at(i);
                break;
            default:
                log("wrong number, AUTO set");
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
            f.Move(posBegin, pos);
            f.Print();
        }
    }

    void Attack(std::vector<Monster>& enemies)
    {
        log("Hero Attack!");
        std::vector<Monster> closeMonsters;
        for(auto& monster : enemies)
        {
            if(monster.GetPos().distance(pos) <= stats.range * 0.5)
            {
                log(monster.GetName() + " HP: " + std::to_string(monster.GetStats().health));
                closeMonsters.push_back(monster); // todo: copy of the monster, set original!
            }
        }
        std::vector<Monster>::iterator attacked;
        if(closeMonsters.size() > 1)
        {
            log("Choose monster to attack (0,1,..)");
            int num;
            std::cin >> num;
            if(num < closeMonsters.size())
            {
                attacked = std::next(closeMonsters.begin(), num);
            }
            else
            {
                attacked = closeMonsters.begin();
            }
        }
        else if(closeMonsters.size() == 1)
        {
            attacked = closeMonsters.begin();
        }
        else
        {
            return;
        }

        attacked->Defend(stats.attack);
        if(attacked->GetStats().health <= 0)
        {
            log(name+" is dead");
            enemies.erase(attacked);
        }
        else
        {
            attacked->Print();
        }

    }
    void PrintStats()
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
    Level(int number): id(number)
    {
        log("LEVEL " + std::to_string(number));

        coord begin(0, MAX_ROW-1);
        hero.SetPosition(begin);
        field.SetCell(begin, cell::hero);
        hero.Print();
    }

    int id;
    Field field;
    Hero hero;
    std::vector<Monster> enemies;


    void HeroTurn()
    {
        log("Hero turn!");
        log("HP: " + std::to_string(hero.GetStats().health));
        hero.RollDice();
        hero.Move(field);
        hero.Attack(enemies);
    }
    void EnemiesTurn()
    {
        log();
        log("Enemies turn!");
        for(auto &i : enemies)
        {
            coord pos = i.GetPos();
            i.Move(hero.GetPos(), field);
            field.Move(pos, i.GetPos());
        }
        int attack = 0;
        for(auto &i : enemies)
        {
            if(i.GetPos().isAdjacent(hero.GetPos()))
            {
                attack += i.GetStats().attack;
            }
        }
        hero.Defend(attack);

        for(auto &i : enemies)
        {
            i.Print();
        }
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
};

int main() {

    Level level1(1);
    level1.AddWall({1, 1});
    level1.AddWall({3, 3});
    level1.AddWall({3, 1});
    level1.AddEnemy("Spider1", {2, 4, 4, 4, 2}, {3, 0});
    level1.AddEnemy("Spider2", {2, 4, 4, 4, 2}, {4, 3});

    while(level1.hero.GetStats().health > 0)
    {
        level1.field.Print();
        level1.HeroTurn();
        level1.EnemiesTurn();
    }

    return 0;
}
