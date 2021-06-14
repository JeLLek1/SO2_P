#include <ncurses.h>
#include "Display.hpp"
#include "Scheduler.hpp"
#include "settings.hpp"

Display::Display(Scheduler* scheduler){
    _scheduler = scheduler;
    initscr();			// uruchomienie trybu curses
    curs_set(false);    // brak kursora
	raw();				// wyłączenie buforowania linii (wyłącza równieć ctrl+c i ctrl+z, cbreak() na nie pozwala)
    noecho();           // wyłączenie wyświetlania wpisywanych znaków
    start_color();      // obsługa kolorów
    init_pair( COLOR_BORDER, COLOR_BLUE, COLOR_BLUE );
    init_pair( COLOR_GROUND, COLOR_WHITE, COLOR_WHITE );
    init_pair( COLOR_PREY, COLOR_GREEN, COLOR_WHITE );
    init_pair( COLOR_HUNTER, COLOR_RED, COLOR_WHITE );
    init_pair( COLOR_HUNTER_WITH_PREY, COLOR_MAGENTA, COLOR_WHITE );
    init_pair( COLOR_TEXT, COLOR_WHITE, COLOR_BLACK );
    init_pair( COLOR_DONATION, COLOR_GREEN, COLOR_WHITE );
}

void Display::run(){
    while(_isRunning){
        auto start = std::chrono::high_resolution_clock::now();
        try{
            clear();
            displayHUD();
            displayMap();
            displayEntities();
            refresh();
        }catch (const std::exception&) {}
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end-start;
        int waitTime = REFRESH_TIME-static_cast<int>(elapsed.count());
        if(waitTime>0){
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
        }
    }
}

void Display::displayMap(){
            int w;
    for(size_t y = 0; y < MAP_HEIGHT; y++){
        for(size_t x = 0; x < MAP_WIDTH; x++){
            if(MAP_DATA[y][x]){
                attron( COLOR_PAIR( COLOR_BORDER ) );
                mvaddch(y,x*2,' ');
                mvaddch(y,x*2+1,' ');
                attroff( COLOR_PAIR( COLOR_BORDER ) );
            }else{
                attron( COLOR_PAIR( COLOR_GROUND ) );
                mvaddch(y,x*2,' ');
                mvaddch(y,x*2+1,' ');
                attroff( COLOR_PAIR( COLOR_GROUND ) );
            }
        }
    }
}

unsigned Display::findSamePos(std::vector<std::pair<Vector2<size_t>, unsigned>>& poses, Vector2<size_t> search){
    bool found = false;
    for (auto &pos : poses){
        if(pos.first.x == search.x && pos.first.y == search.y){
            pos.second++;
            found = true;
            break;
        }
    }
    if(!found){
        poses.push_back(std::pair<Vector2<size_t>, unsigned>(search, 1));
    }
    return poses.back().second;
}

void Display::displayEntities(){
    int preysDigits = Scheduler::countDigits(PREY_COUNT);
    int huntersDigits = Scheduler::countDigits(HUNTER_COUNT);
    {
        std::vector<PreyData> data = _scheduler->getPreysData();
        size_t index = 1;
        std::vector<std::pair<Vector2<size_t>, unsigned>> samePos;
        for (auto &i : data)
        {
            if(i.state == PreyStates::RUN_AWAY){
                findSamePos(samePos, i.pos);
            }
            attron(COLOR_PAIR( COLOR_TEXT));
            mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X, "%d", index);
            mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + preysDigits, ". Stan: %s", Prey::stateNames.at(i.state).c_str());
            if(i.state == PreyStates::RUN_AWAY || i.state==PreyStates::RESPAWN){
                mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 8 + HUD_LONGEST_STATE_PREY + preysDigits, " | Progres: %3d%%", i.progress);
            }else{
                mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 8 + HUD_LONGEST_STATE_PREY + preysDigits, " | Progres: ---%%");
            }
            if(i.state == PreyStates::RUN_AWAY){
                mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 24 + HUD_LONGEST_STATE_PREY + preysDigits, " | Pozycja: ( %3d, %3d )", i.pos.x, i.pos.y);
            }else{
                mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 24 + HUD_LONGEST_STATE_PREY + preysDigits, " | Pozycja: ( ---, --- ) | ");
            }
            if(i.state == PreyStates::CAUGHT){
                mvprintw(3 + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 48 + HUD_LONGEST_STATE_PREY + preysDigits, " | Zlapany przez: %d", i.hunterIndex+1);
            }
            attroff(COLOR_PAIR( COLOR_TEXT));
            index++;
        }
        for (auto &i : samePos){
            attron( COLOR_PAIR( COLOR_PREY ) );
            attron(A_BOLD);
            mvaddch(i.first.y,i.first.x*2,'+');
            mvprintw(i.first.y,i.first.x*2+1,"%d", i.second);
            attroff(A_BOLD);
            attroff( COLOR_PAIR( COLOR_PREY ) );
        }
    }
    {
        std::vector<HunterData> data = _scheduler->getHuntersData();
        size_t index = 1;
        for (auto &i : data)
        {
            if(i.state == HunterStates::SEEK){
                attron(A_BOLD);
                attron( COLOR_PAIR( COLOR_HUNTER ) );
                mvaddch(i.pos.y,i.pos.x*2,ACS_LARROW);
                mvaddch(i.pos.y,i.pos.x*2+1,ACS_RARROW);
                attroff( COLOR_PAIR( COLOR_HUNTER ) );
                attroff(A_BOLD);
            }else if(i.state == HunterStates::HAS_PRAY){
                attron(A_BOLD);
                attron( COLOR_PAIR( COLOR_HUNTER_WITH_PREY ) );
                mvaddch(i.pos.y,i.pos.x*2,ACS_LEQUAL);
                mvaddch(i.pos.y,i.pos.x*2+1,ACS_GEQUAL);
                attroff( COLOR_PAIR( COLOR_HUNTER_WITH_PREY ) );
                attroff(A_BOLD);
            }
            attron(COLOR_PAIR( COLOR_TEXT));
            mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X, "%d", index);
            mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + huntersDigits, ". Stan: %s", Hunter::stateNames.at(i.state).c_str());
            if(i.state == HunterStates::SEEK || i.state == HunterStates::RESPAWN || i.state == HunterStates::HAS_PRAY){
                mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 8 + HUD_LONGEST_STATE_HUNTER + huntersDigits, " | Progres: %3d%%", i.progress);
            }else{
                mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 8 + HUD_LONGEST_STATE_HUNTER + huntersDigits, " | Progres: ---%%", i.progress);
            }
            if(i.state == HunterStates::SEEK || i.state == HunterStates::HAS_PRAY){
                mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 20 + HUD_LONGEST_STATE_PREY + huntersDigits, " | Pozycja: ( %3d, %3d ) | ", i.pos.x, i.pos.y);
            }else{
                mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 20 + HUD_LONGEST_STATE_PREY + huntersDigits, " | Pozycja: ( ---, --- ) | ");
            }
            if(i.state == HunterStates::HAS_PRAY){
                mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 47 + HUD_LONGEST_STATE_PREY + huntersDigits, "Zlapal: %d", i.preyIndex+1);
            }
            mvprintw(5 + PREY_COUNT + index, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X + 55 + HUD_LONGEST_STATE_PREY + huntersDigits + preysDigits, " | Punkty: %d", i.points);
            attroff(COLOR_PAIR( COLOR_TEXT));
            index++;
        }
    }
    for (size_t i = 0; i<sizeof(DONATION_POINTS)/sizeof(DONATION_POINTS[0]); i++){
        attron( COLOR_PAIR( COLOR_DONATION ) );
        attron(A_BOLD);
        mvaddch(DONATION_POINTS[i].y,DONATION_POINTS[i].x*2,ACS_CKBOARD);
        mvaddch(DONATION_POINTS[i].y,DONATION_POINTS[i].x*2+1,ACS_CKBOARD);
        attroff(A_BOLD);
        attroff( COLOR_PAIR( COLOR_DONATION ) );
    }
}

void Display::displayHUD(){
    attron(COLOR_PAIR( COLOR_TEXT));
    attron(A_BOLD);
    mvprintw(0, MAP_WIDTH*2 + HUD_SHIFT_X, "========== [ Lowcy i zwierzyna ] ==========");
    attroff(A_BOLD);
    mvprintw(2, MAP_WIDTH*2 + HUD_SHIFT_X, "obslugiwane klawisze: ");
    mvprintw(3, MAP_WIDTH*2 + HUD_SHIFT_X, "q - zakonczenie watkow i wyjscie z programu");
    mvprintw(5, MAP_WIDTH*2 + HUD_SHIFT_X, "znaczenie elementow mapy: ");
    attron(A_BOLD);
    attron( COLOR_PAIR( COLOR_PREY ) );
    mvaddch(6, MAP_WIDTH*2 + HUD_SHIFT_X,'+');
    mvaddch(6, MAP_WIDTH*2 + HUD_SHIFT_X + 1,'x');
    attroff(A_BOLD);
    attron( COLOR_PAIR( COLOR_TEXT ) );
    mvprintw(6, MAP_WIDTH*2 + HUD_SHIFT_X + 3, "- zwierzyna");
    attron(A_BOLD);
    attron( COLOR_PAIR( COLOR_HUNTER ) );
    mvaddch(7, MAP_WIDTH*2 + HUD_SHIFT_X,ACS_LARROW);
    mvaddch(7, MAP_WIDTH*2 + HUD_SHIFT_X +1,ACS_RARROW);
    attroff(A_BOLD);
    attron( COLOR_PAIR( COLOR_TEXT ) );
    mvprintw(7, MAP_WIDTH*2 + HUD_SHIFT_X + 3, "- lowca");
    attron(A_BOLD);
    attron( COLOR_PAIR( COLOR_HUNTER_WITH_PREY ) );
    mvaddch(8, MAP_WIDTH*2 + HUD_SHIFT_X,ACS_LEQUAL);
    mvaddch(8, MAP_WIDTH*2 + HUD_SHIFT_X+1,ACS_GEQUAL);
    attroff(A_BOLD);
    attron( COLOR_PAIR( COLOR_TEXT ) );
    mvprintw(8, MAP_WIDTH*2 + HUD_SHIFT_X + 3, "- lowca ze zlapana zwierzyna");
    attron( COLOR_PAIR( COLOR_DONATION ) );
    attron(A_BOLD);
    mvaddch(9, MAP_WIDTH*2 + HUD_SHIFT_X,ACS_CKBOARD);
    mvaddch(9, MAP_WIDTH*2 + HUD_SHIFT_X +1,ACS_CKBOARD);
    attroff(A_BOLD);
    attroff( COLOR_PAIR( COLOR_DONATION ) );
    mvprintw(9, MAP_WIDTH*2 + HUD_SHIFT_X + 3, "- punkt oddania zwierzyny");
    mvprintw(2, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X, "informacje o watkach:");
    mvprintw(3, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X, "zwierzyna:");
    mvprintw(5 + PREY_COUNT, MAP_WIDTH*2 + HUD_ENTITIES_SHIFT_X, "Lowcy:");
    attroff( COLOR_PAIR( COLOR_TEXT ) );

    if(!_scheduler->isRunning()){
        mvprintw(MAP_HEIGHT + 1, 0, "zamykanie...");
    }
}

void Display::start(){
    if(!_thread.joinable())
        _thread = std::thread(&Display::run, this);
}

void Display::endProcess(){
    _isRunning = false;
    _scheduler->join(_thread);
}

Display::~Display(){
    endProcess();
    endwin();
}
