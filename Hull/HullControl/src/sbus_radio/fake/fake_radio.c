#include <curses.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>

#include "sclog4c/sclog4c.h"
#include <lcm/lcm.h>

#include "lcm_channels.h"
#include "lcm/stomp_control_radio.h"
#include "sbus_channels.h"

#define MAX(a, b) (((a)>(b)) ? (a) : (b))
#define MIN(a, b) (((a)<(b)) ? (a) : (b))
#define CLAMP(x, mn, mx) MIN(MAX(x, mn), mx)

char *stomp_control_radio_toggle_str(int8_t x)
{
    if(x==STOMP_CONTROL_RADIO_OFF)
        return "OFF";
    else if(x==STOMP_CONTROL_RADIO_CENTER)
        return "CEN";
    else if(x==STOMP_CONTROL_RADIO_ON)
        return " ON";
    else
        return "UNK";
}

int next_toggle(int current, int step)
{
    int ret = current;
    if( step == 1)
    {
        switch(current)
        {
            case STOMP_CONTROL_RADIO_OFF:
                ret = STOMP_CONTROL_RADIO_CENTER;
                break;
            case STOMP_CONTROL_RADIO_CENTER:
                ret = STOMP_CONTROL_RADIO_ON;
                break;
            case STOMP_CONTROL_RADIO_ON:
                ret = STOMP_CONTROL_RADIO_ON;
                break;
        }
    }
    else if(step == 10)
    {
        ret = STOMP_CONTROL_RADIO_ON;
    }
    else if(step == -1)
    {
        switch(current)
        {
            case STOMP_CONTROL_RADIO_OFF:
                ret = STOMP_CONTROL_RADIO_OFF;
                break;
            case STOMP_CONTROL_RADIO_CENTER:
                ret = STOMP_CONTROL_RADIO_OFF;
                break;
            case STOMP_CONTROL_RADIO_ON:
                ret = STOMP_CONTROL_RADIO_CENTER;
                break;
        }
    }
    else if(step == -10)
    {
        ret = STOMP_CONTROL_RADIO_OFF;
    }
    return ret;
}

void step(int row, int col, int step, stomp_control_radio* sbus_msg)
{
    switch(row)
    {
        case 0:
            sbus_msg->toggle[col] = next_toggle(sbus_msg->toggle[col], step);
            break;
        case 1:
            sbus_msg->axis[col] = CLAMP(sbus_msg->axis[col] + 1e-3f*step, -1.0f, 1.0f);
            break;
        case 2:
            if(col==0)
                sbus_msg->failsafe = step > 0;
            else if(col==1)
                sbus_msg->no_data = step > 0;
            break;
    }
}

const int TOGGLE_ROW=2;
const int AXIS_ROW=4;
const int SAFE_ROW=6;

const int STARTCOL=5;

void display_message(stomp_control_radio *sbus_msg, int row, int col)
{
    move(TOGGLE_ROW, STARTCOL);
    clrtoeol();
    move(TOGGLE_ROW + 2, STARTCOL);
    clrtoeol();
    for(int i=0;i<STOMP_CONTROL_RADIO_TOGGLES;i++)
    {
        move(TOGGLE_ROW, STARTCOL + 7*i);
        if(row==0 && col == i)
            attron(A_BOLD);
        printw(" %3s", stomp_control_radio_toggle_str(sbus_msg->toggle[i]));
        if(i==HULL_ENABLE)
            mvprintw(TOGGLE_ROW + 1, STARTCOL + 7*i, " %s", (sbus_msg->toggle[i]==STOMP_CONTROL_RADIO_ON) ? "walk" : "dis");
        else if(i==HULL_MODE)
            mvprintw(TOGGLE_ROW + 1, STARTCOL + 7*i, " %s", (sbus_msg->toggle[i]==STOMP_CONTROL_RADIO_ON) ? "lock" : "free");
        attroff(A_BOLD);
    }
    move(AXIS_ROW, STARTCOL);
    clrtoeol();
    for(int i=0;i<STOMP_CONTROL_RADIO_AXES;i++)
    {
        move(AXIS_ROW, STARTCOL + 7*i);
        if(row==1 && col == i)
            attron(A_BOLD);
        if(sbus_msg->axis[i] < -1.0 || 1.0 < sbus_msg->axis[i])
            printw("****** ");
        else
            printw("%6.3f ", sbus_msg->axis[i]);
        if(i==HULL_VELOCITY_X)
            mvprintw(AXIS_ROW + 1, STARTCOL + 7*i, " %s", "vel x");
        else if(i==HULL_OMEGA_Z)
            mvprintw(AXIS_ROW + 1, STARTCOL + 7*i, " %s", "omg z");
        attroff(A_BOLD);
    }
    move(6, 6);
    clrtoeol();
    if(row==2 && col==0)
        attron(A_BOLD);
    printw("failsafe: %s", sbus_msg->failsafe ? " on" : "off");
    attroff(A_BOLD);
    move(6, 6 + 14);
    if(row==2 && col==1)
        attron(A_BOLD);
    printw("no_data: %s", sbus_msg->no_data ? " on" : "off");
    attroff(A_BOLD);

}

#define positive_wrap(n, op, max) \
    do { \
        n = n op; \
        if(n>=max) n-=max; \
        if(n<0) n+=max; \
    } while(0);

int main(int argc, char** argv)
{
    int period = 10; // ms
    int opt;
    while((opt = getopt(argc, argv, "v::p:")) != -1)
    {
        switch(opt)
        {
            case 'v':
                if(optarg) // with arg, set custom level
                {
                    sclog4c_level = atoi(optarg);
                }
                else //v for verbose, set log level to debug
                {
                    sclog4c_level = SL4C_DEBUG;
                }
                break;
            case 'p':
                period = atoi(optarg);
                break;
        }
    }

    lcm_t *lcm = lcm_create(NULL);
    if(!lcm)
    {
        logm(SL4C_FATAL, "Failed to init LCM.");
        return 1;
    }

    stomp_control_radio sbus_msg;
    memset(&sbus_msg.toggle, STOMP_CONTROL_RADIO_OFF, sizeof(sbus_msg.toggle));
    memset(&sbus_msg.axis, 0, sizeof(sbus_msg.axis));
    sbus_msg.failsafe = false;
    sbus_msg.no_data = false;

    initscr();
    start_color();
    cbreak();
    noecho();
    clear();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    bool go=true;
    int ch, pch = 0;
    struct timeval now, last_wakeup, dt;
    gettimeofday(&last_wakeup, NULL);
    int sleep;
    int row=0, col=0;
    while(go)
    {
        ch = getch();
        if(ch != -1)
        {
            pch = ch;
        }
        move(0, 25);
        clrtoeol();
        printw("'%s':(%d)", keyname(pch), pch);
        int rowlen[3] = { STOMP_CONTROL_RADIO_TOGGLES,
                          STOMP_CONTROL_RADIO_AXES,
                          2};
        switch(ch)
        {
            case ERR:
                refresh();
                gettimeofday(&now, NULL);
                timersub(&now, &last_wakeup, &dt);
                sleep = period*1000 - dt.tv_usec;
                if(sleep>0)
                    usleep(sleep);
                gettimeofday(&last_wakeup, NULL);
                break;
            case '':
                go = false;
                break;
            case KEY_F(1):
                break;
            case KEY_LEFT:
                positive_wrap(col, -1, rowlen[row])
                break;
            case KEY_RIGHT:
                positive_wrap(col, +1, rowlen[row])
                break;
            case KEY_UP:
                positive_wrap(row, -1, 3)
                break;
            case KEY_DOWN:
                positive_wrap(row, +1, 3)
                break;
            case 'q':
                step(row, col, 1, &sbus_msg);
                break;
            case 'Q':
                step(row, col, 10, &sbus_msg);
                break;
            case 'a':
                step(row, col, -1, &sbus_msg);
                break;
            case 'A':
                step(row, col, -10, &sbus_msg);
                break;
             default:
                break;
        }
        display_message(&sbus_msg, row, col);
        stomp_control_radio_publish(lcm, SBUS_RADIO_COMMAND, &sbus_msg);
    }

    endwin();
}
