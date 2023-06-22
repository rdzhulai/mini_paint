

/*
    Technická univerzita v Košiciach
    Fakulta elektrotechniky a informatiky
    Názov: MiniPaint
    Dzhulai Roman E5 2022/2023
*/


/*
    2D svet
    Práca s farbami
    Ovládanie cez klávesnicu
    Viac úrovní (levelov)
    Práca s časomierou resp. práca v čase (s časom sa program mení)
    Práca s argumentami príkazového riadku
    Práca so súbormi

*/


/*
    The program can open and create sheets. Binary files are used to store a sheet, that is represented as 2d array, and its width and length.
    Width and length of a sheet are not exceeding width and length of the menu window.
    There are options of how to open files. You can open a file as a command line argument, and you can open the file in the user interface 		as well.
    While painting, you can use eight colors: white, black, blue, green, red, yellow, cyan, and magenta. 
*/


/*
    To execute the program you neet to compile it using -lcurses.
    Then you can write "./program file_name.bin" and it will open this file if it exists in the directory of the program. If not, user interface will appear.
    Use arrow keys and Enter to switch the buttons.
    Use Esc to save sheets and return to the menu. When entering the name of a file you can also use Esc to return to the menu. 
    In painting mode use arrows to move cursor. In the beggining cursor appears in the top left corner in black color. Use 1-8 keys to switch colors. Use ('f' + 1-8) keys to 
    fill the entire sheet with the appropriate color.
*/

#include <stdlib.h>
#include <ncurses.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define WIN_MENU_ROW_SIZE 38
#define WIN_MENU_COL_SIZE 142
#define BUTTONS_SIZE 2
#define FNAME_SIZE 10
#define FILESPEC_SIZE (FNAME_SIZE + 4)
#define MY_STRERROR_SIZE 100
#define CH '#'
#define ENTER 10
#define ESC 27


typedef enum {white = 1, black, blue, green, red, yellow, cyan, magenta} Color;

typedef enum {STATE_MENU, STATE_MENU_INIT, STATE_SHEET_OPEN, STATE_SHEET_SAVE, STATE_SHEET, STATE_EXIT} State;

typedef struct{
    int x, y;
    State state;
    char text[20];
} Button;


void win_center(const WINDOW *bwin, WINDOW *win); // Put window at the center of window 'bwin'. bwin - Back Window.
void buttons_center(const WINDOW *win, Button buttons[]); // Set x and y of each button so that they would be at the center of window.
void menu_draw(WINDOW *win, const Button buttons[], const int current_button_index); // Draw buttons to the window and highlight  the current button.
bool menu_update(WINDOW *win, const Button buttons[], const int key_pressed, State *current_state, int *current_button_index); // Deal with keys and draw buttons to the window.
bool sheet_get(WINDOW *win, char *filespec, unsigned int *sheet_row_size, unsigned int *sheet_col_size, short **sheet); // Use functions sheet_read and sheet_save to open or create user's sheets. 
char* sheet_read(short **sheet, const char *filespec, unsigned int *sheet_row_size, unsigned int *sheet_col_size); // Read sheets from files.
char* sheet_save(short *sheet, const char *filespec, const unsigned int sheet_row_size, const unsigned int sheet_col_size); // Write sheets to files.
bool cursor_update(const int key_pressed, int *y, int *x, const unsigned int sheet_row_size, const unsigned int sheet_col_size); // Deal with cursor on sheets.
void sheet_draw(WINDOW *win, const short *sheet, const unsigned int sheet_row_size, const unsigned int sheet_col_size); // Draw a sheet to the window. 
bool color_update(const int key_pressed, Color *current_color); // Deal with colors.
void sheet_fill(const unsigned int sheet_row_size, const unsigned int sheet_col_size, short *sheet,const Color c); // Fill all sheet in with some color.
void set_color(short *sheet, const Color color, const int y, const int x,  const unsigned int sheet_col_size); // Color the cursor's place.
bool sheet_update(WINDOW *win_sheet, short *sheet, const int key_pressed, State *current_state, 
                Color *current_color, int *cursor_y, int *cursor_x, const unsigned int sheet_row_size, 
                const unsigned int sheet_col_size); // Deal with all sheet's staff.



int main(int argc, char **argv) {

    // Std window
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    refresh();

    if(!has_colors())
    {
        fprintf(stderr, "Your terminal does not support color.\n");
        return 1;
    }
    // Color initialization
    start_color();
    init_pair(white, COLOR_WHITE, COLOR_WHITE);
    init_pair(black, COLOR_BLACK, COLOR_BLACK);
    init_pair(blue, COLOR_BLUE, COLOR_BLUE);
    init_pair(red, COLOR_RED, COLOR_RED);
    init_pair(green, COLOR_GREEN, COLOR_GREEN);
    init_pair(yellow, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(cyan, COLOR_CYAN, COLOR_CYAN);
    init_pair(magenta, COLOR_MAGENTA, COLOR_MAGENTA);
    // Declaration of variables
    State current_state = STATE_MENU_INIT;
    int key_pressed = ' ';
    char filespec[FILESPEC_SIZE];
    Button buttons[BUTTONS_SIZE] = 
    {
        {.state = STATE_SHEET_OPEN, .text = "OPEN FILE"}, 
        {.state = STATE_EXIT, .text = "EXIT"}
    };
    int current_button_index;
    bool EXIT = false;
    // Menu window
    WINDOW *win_menu = newwin(WIN_MENU_ROW_SIZE, WIN_MENU_COL_SIZE, 0, 0);
    keypad(win_menu, TRUE);
    // Sheet window
    WINDOW *win_sheet = NULL;
    unsigned int sheet_row_size, sheet_col_size;
    short *sheet = NULL;
    int cursor_x = 0, cursor_y = 0;
    Color current_color = black;

    if(argc > 1)
    {
        strncpy(filespec, argv[1], FILESPEC_SIZE);
        if(!sheet_read(&sheet, filespec, &sheet_row_size, &sheet_col_size)){
            box(win_menu, 0, 0);
            wrefresh(win_menu);
            win_sheet = newwin(sheet_row_size, sheet_col_size, 0, 0);
            keypad(win_sheet, TRUE);
            sheet_draw(win_sheet, sheet, sheet_row_size, sheet_col_size);
            win_center(win_menu, win_sheet);
            wrefresh(win_sheet);
            current_state = STATE_SHEET;
        }
    }
    

    // Program loop    
    while(!EXIT){
        switch(current_state){

            case STATE_MENU_INIT:
                current_button_index = 0;
                win_center(stdscr, win_menu);
                buttons_center(win_menu, buttons);
                wclear(win_menu);
                box(win_menu, 0, 0);
                menu_draw(win_menu, buttons, current_button_index);
                wrefresh(win_menu);
                current_state = STATE_MENU;
                break;


            case STATE_MENU:
                win_center(stdscr, win_menu);
                wrefresh(win_menu);
                key_pressed = wgetch(win_menu);  
                menu_update(win_menu, buttons, key_pressed, &current_state, &current_button_index);
                break;

            case STATE_SHEET_OPEN:  
                if(!sheet_get(win_menu, filespec, &sheet_row_size, &sheet_col_size, &sheet)){
                    current_state = STATE_MENU_INIT;
                    break;
                }
                wclear(win_menu);
                box(win_menu, 0, 0);
                wrefresh(win_menu);
                if(win_sheet) delwin(win_sheet);
                win_sheet = newwin(sheet_row_size, sheet_col_size, 0, 0);
                keypad(win_sheet, TRUE);
                cursor_y = cursor_x = 0;
                current_color = black;
                sheet_draw(win_sheet, sheet, sheet_row_size, sheet_col_size);
                win_center(win_menu, win_sheet);
                wrefresh(win_sheet);
                current_state = STATE_SHEET;
                break;       
            
            case STATE_SHEET:
                win_center(win_menu, win_sheet);
                wrefresh(win_sheet);
                key_pressed = wgetch(win_sheet);
                sheet_update(win_sheet, sheet, key_pressed, &current_state, &current_color, &cursor_y, &cursor_x, sheet_row_size, sheet_col_size);
                break;

            case STATE_SHEET_SAVE:
                wclear(win_sheet);
                wrefresh(win_sheet);
                char *my_strerror = sheet_save(sheet, filespec, sheet_row_size, sheet_col_size);
                if(my_strerror != NULL)
                {
                    mvwprintw(win_menu, 1, 1, "%s", my_strerror);
                    wrefresh(win_menu);
                    wgetch(win_menu);
                }
                current_state = STATE_MENU_INIT;
                break;

            case STATE_EXIT:
                free(sheet);
                EXIT = true;
                break;

        }
        
    }

    endwin();

    return EXIT_SUCCESS;
}



bool sheet_get(WINDOW *win, char *filespec, unsigned int *sheet_row_size, unsigned int *sheet_col_size, short **sheet){

    char fname[FNAME_SIZE];
    wclear(win);
    box(win, 0, 0);
    curs_set(1);
    echo();
    mvwprintw(win, 1, 1, "File name: ");
    wgetnstr(win, fname, FNAME_SIZE);
    if(fname[0] == ESC){
        curs_set(0);
        noecho();
        return false;
    }
    snprintf(filespec, FILESPEC_SIZE, "%s%s", fname, ".bin");

    char *my_strerror = sheet_read(sheet, filespec, sheet_row_size, sheet_col_size);

    while(my_strerror != NULL)
    {
        mvwprintw(win, 2, 1, "%s", my_strerror);
        mvwprintw(win, 3, 1, "Do you want to create a file named '%s'? <yes/no>: ", fname);
        char answer[3];
        wgetnstr(win, answer, 3);
        if(!strcmp(answer, "yes"))
        {
            int width, height;
            mvwprintw(win, 4, 1, "Width:");
            mvwprintw(win, 5, 1, "Height:");
            do{
            mvwprintw(win, 4, 7, "          ");
            mvwprintw(win, 5, 8, "          ");
            mvwscanw(win, 4, 8, "%d", &width);
            mvwscanw(win, 5, 9, "%d", &height);
            }while(width <= 0 || width > WIN_MENU_COL_SIZE || height <= 0 || height > WIN_MENU_ROW_SIZE);
            *sheet = realloc(*sheet, sizeof(*sheet)*height*width);
            if(*sheet == NULL)
            {
                mvwprintw(win, 6, 1, "The data were not reallocated");
                wrefresh(win);
                wgetch(win);
                return false;
            }
            sheet_fill(height, width, *sheet, white);
            my_strerror = sheet_save(*sheet, filespec, height, width);
            if(my_strerror != NULL)
            {
                mvwprintw(win, 6, 1, "%s", my_strerror);
                wrefresh(win);
                wgetch(win);
                return false;
            }
        }
        else{
            mvwprintw(win, 1, 12, "          ");
            mvwprintw(win, 2, 1, "                                                             ");
            mvwprintw(win, 3, 1, "                                                             ");
            mvwscanw(win,1, 12,"%s", fname);
            if(fname[0] == ESC){
                curs_set(0);
                noecho();
                return false;
            }
            snprintf(filespec, FILESPEC_SIZE, "%s%s", fname, ".bin");
        }
        my_strerror = sheet_read(sheet, filespec, sheet_row_size, sheet_col_size);
    }
    noecho(); 
    curs_set(0);
    return true;

}

void sheet_fill(const unsigned int sheet_row_size, const unsigned int sheet_col_size, short *sheet,const Color c)
{
    for(int y = 0; y < sheet_row_size; y++){
        for(int x = 0; x < sheet_col_size; x++){
            sheet[y * sheet_col_size + x] = c;
        }
    }
}

void set_color(short *sheet, const Color c, const int y, const int x,  const unsigned int sheet_col_size)
{
    sheet[y * sheet_col_size + x] = c;
}

void sheet_draw(WINDOW *win, const short *sheet, const unsigned int sheet_row_size, const unsigned int sheet_col_size)
{
    for(int y = 0; y < sheet_row_size; y++){
        for(int x = 0; x < sheet_col_size; x++){

            wattron(win, COLOR_PAIR(sheet[y * sheet_col_size + x]));
            mvwaddch(win, y, x, CH);
            wattroff(win, COLOR_PAIR(sheet[y * sheet_col_size + x]));

        }
    }
}

void win_center(const WINDOW *bwin, WINDOW *win)
{
    int bwin_maxy, bwin_maxx;
    int win_maxy, win_maxx;
    getmaxyx(bwin, bwin_maxy, bwin_maxx);
    getmaxyx(win, win_maxy, win_maxx);
    mvwin(win, bwin_maxy/2 - win_maxy/2, bwin_maxx/2 - win_maxx/2);
}

bool cursor_update(const int key_pressed, int *y, int *x, const unsigned int sheet_row_size, const unsigned int sheet_col_size)
{
    switch(key_pressed)
    {
        case KEY_LEFT:
            if(--(*x) != -1) return true;
            (*x)++;
            break;
        case KEY_RIGHT:
            if(++(*x) != sheet_col_size) return true;
            (*x)--;
            break;
        case KEY_UP:
            if(--(*y) != -1) return true;
            (*y)++;
            break;
        case KEY_DOWN:
            if(++(*y) != sheet_row_size) return true;
            (*y)--;
            break;
    };
    return false;
}

bool color_update(const int key_pressed, Color *current_color){
    
    switch ((key_pressed - '0'))
    {
        case black:
            *current_color = black;
            return true;
        case white:
            *current_color = white;
            return true;
        case blue:
            *current_color = blue;
            return true;
        case red:
            *current_color = red;
            return true;
        case yellow:
            *current_color = yellow;
            return true;
        case green:
            *current_color = green;
            return true;
        case cyan:
            *current_color = cyan;
            return true;
        case magenta:
            *current_color = magenta;
            return true;
        default:
            return false;

    };
}

char* sheet_save(short *sheet, const char *filespec, const unsigned int sheet_row_size, const unsigned int sheet_col_size)
{
    FILE *fp;
    errno = 0;
    fp = fopen(filespec, "wb");
    if(errno)
        return strerror(errno);
    if(fwrite(&sheet_row_size, sizeof(sheet_row_size), 1, fp) != 1 || fwrite(&sheet_col_size, sizeof(sheet_col_size), 1, fp) != 1)
        return "The data were not written";
    if(fwrite(sheet, sizeof(*sheet), sheet_row_size * sheet_col_size, fp) != sheet_row_size * sheet_col_size)
        return "The data were not written";
    fclose(fp);
    return NULL;
}

char* sheet_read(short **sheet, const char *filespec, unsigned int *sheet_row_size, unsigned int *sheet_col_size)
{
    FILE *fp;
    errno = 0;
    fp = fopen(filespec, "rb");
    if(errno)
        return strerror(errno);
    if(fread(sheet_row_size, sizeof(*sheet_row_size), 1, fp) != 1 || fread(sheet_col_size, sizeof(*sheet_col_size), 1, fp) != 1)
        return "The data were not read";
    *sheet = realloc(*sheet, sizeof(**sheet) * (*sheet_row_size) * (*sheet_col_size));
    if(*sheet == NULL)
        return "The data were not reallocated";
    if(fread(*sheet, sizeof(**sheet), (*sheet_row_size) * (*sheet_col_size), fp) != (*sheet_row_size) * (*sheet_col_size))
        return "The data were not read";
    fclose(fp);
    return NULL;
}

void buttons_center(const WINDOW *win, Button buttons[])
{
    int maxy, maxx;
    getmaxyx(win, maxy, maxx);
    for(int i = 0, y_start = maxy/2 - BUTTONS_SIZE/2; i < BUTTONS_SIZE; i++)
    {
        buttons[i].y = y_start + i;
        buttons[i].x = maxx/2 - strlen(buttons[i].text)/2;
    }
}

bool menu_update(WINDOW *win, const Button buttons[], const int key_pressed, State *current_state, int *current_button_index){

    switch(key_pressed){
        case KEY_UP:
            (*current_button_index)--;
            if((*current_button_index) != -1) break;
            (*current_button_index)++;
            return false;
        case KEY_DOWN:
            (*current_button_index)++;
            if((*current_button_index) != BUTTONS_SIZE) break;
            (*current_button_index)--;
            return false;
        case ENTER:
            *current_state = buttons[*current_button_index].state;
            return true;
        default:
            return false;

    }
    menu_draw(win, buttons, *current_button_index);
    return true;

}
 
void menu_draw(WINDOW *win, const Button buttons[], const int current_button_index){
    for(int i = 0; i < BUTTONS_SIZE; i++)
    {
        if(current_button_index == i)
            wattron(win, A_REVERSE);
        mvwprintw(win, buttons[i].y, buttons[i].x, "%s", buttons[i].text);
        wattroff(win, A_REVERSE);
    }
}

bool sheet_update(WINDOW *win_sheet, short *sheet, const int key_pressed, State *current_state, Color *current_color, int *cursor_y, int *cursor_x, const unsigned int sheet_row_size, const unsigned int sheet_col_size){

    static bool fill = false;
    if(color_update(key_pressed, current_color)){
        if(fill){
            sheet_fill(sheet_row_size, sheet_col_size, sheet, *current_color);
            fill = false;
        }
        else
            set_color(sheet, *current_color, *cursor_y, *cursor_x, sheet_col_size);
        sheet_draw(win_sheet, sheet, sheet_row_size, sheet_col_size);
        return true;
    }
    if(cursor_update(key_pressed, cursor_y, cursor_x, sheet_row_size, sheet_col_size)){
        set_color(sheet, *current_color, *cursor_y, *cursor_x, sheet_col_size);
        sheet_draw(win_sheet, sheet, sheet_row_size, sheet_col_size);
        return true;
    }
    switch(key_pressed)
    {
        case ESC:
            *current_state = STATE_SHEET_SAVE;
            break;
        case 'f':
            fill = true;
            break;
        default:
            break;
    }
    return false;

}

