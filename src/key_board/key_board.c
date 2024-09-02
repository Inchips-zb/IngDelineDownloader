#include "key_board.h"
#include "platform_api.h"
#include "key_board_sample.h"
#if ((KEY_LONG_SUPPORT != KEY_ENABLE) && (KEY_LONG_SUPPORT != KEY_DISABLE))
    #error "KEY_LONG_SUPPORT can be only set to KEY_ENABLE or KEY_DISABLE."
#endif

#if ((KEY_MULTI_SUPPORT != KEY_ENABLE) && (KEY_MULTI_SUPPORT != KEY_DISABLE))
    #error "KEY_MULTI_SUPPORT can be only set to KEY_ENABLE or KEY_DISABLE."
#endif

#if ((KEY_CONTINUOUS_SUPPORT != KEY_ENABLE) && (KEY_CONTINUOUS_SUPPORT != KEY_DISABLE))
    #error "KEY_CONTINUOUS_SUPPORT can be only set to KEY_ENABLE or KEY_DISABLE."
#endif

#if ((KEY_COMBINE_SUPPORT != KEY_ENABLE) && (KEY_COMBINE_SUPPORT != KEY_DISABLE))
    #error "KEY_COMBINE_SUPPORT can be only set to KEY_ENABLE or KEY_DISABLE."
#endif

#if (KEY_BOARD_MAX_NUM <= 0)
    #error "The value of KEY_BOARD_MAX_NUM must be greater than 0."
#endif

#if (KEY_MAX_NUM <= 0)
    #error "The value of KEY_MAX_NUM must be greater than 0."
#endif

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
    #if (KEY_DEFAULT_LONG_TRRIGER_TIME <= 0)
        #error "The value of KEY_DEFAULT_LONG_TRRIGER_TIME must be greater than 0."
    #endif
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
    #if (KEY_DEFAULT_MULTI_INTERVAL_TIME <= 0)
        #error "The value of KEY_DEFAULT_MULTI_INTERVAL_TIME must be greater than 0."
    #elif (KEY_DEFAULT_MULTI_INTERVAL_TIME <= KEY_DEFAULT_DEBOUNCE_TIME)
        #warning "It is strongly recommended that the value of KEY_DEFAULT_MULTI_INTERVAL_TIME is greater than the value of KEY_DEFAULT_DEBOUNCE_TIME."
    #endif
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
    #if (KEY_DEFAULT_CONTINUOUS_INIT_TRRIGER_TIME <= 0)
        #error "The value of KEY_DEFAULT_CONTINUOUS_INIT_TRRIGER_TIME must be greater than 0."
    #endif

    #if (KEY_DEFAULT_CONTINUOUS_PERIOD_TRRIGER_TIME <= 0)
        #error "The value of KEY_DEFAULT_CONTINUOUS_PERIOD_TRRIGER_TIME must be greater than 0."
    #endif
#endif

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    #if (KEY_DEFAULT_COMBINE_INTERVAL_TIME <= 0)
        #error "The value of KEY_DEFAULT_COMBINE_INTERVAL_TIME must be greater than 0."
    #elif (KEY_DEFAULT_COMBINE_INTERVAL_TIME <= KEY_DEFAULT_DEBOUNCE_TIME)
        #warning "It is strongly recommended that the value of KEY_DEFAULT_COMBINE_INTERVAL_TIME is greater than the value of KEY_DEFAULT_DEBOUNCE_TIME."
    #endif
#endif

#if (KEY_DEFAULT_DEBOUNCE_TIME < 0)
    #error "The value of KEY_DEFAULT_DEBOUNCE_TIME must be greater than or equal to 0."
#endif

#define KEY_MAX_TIME        ((1u << ((sizeof(unsigned int) << 3) - 1)) - 1)
#define KEY_TIME_CHECK(x)   ((x) < KEY_MAX_TIME ? (x) : KEY_MAX_TIME - 1)

struct event_cnt_t {
    unsigned int rd_cnt    : 16,
                 wr_cnt    : 16;
};

struct key_private_t {
    struct key_public_sig_t *property;
    enum key_state_t state;                    //key state
    bool last_level;                           //key last state (invalid/valid)
    bool this_level;                           //key this state (invalid/valid)

    struct event_cnt_t release_event;          //release event count
    struct event_cnt_t press_event;            //press event count
    struct event_cnt_t pressing_event;         //pressing event count

    unsigned int init_debounce_time;           //debouce time init
    unsigned int debounce_time;                //debounce timer

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
    struct {
        unsigned int trriger_time;             //trriger time
        unsigned int press_time;               //pressed time
        bool trriger_flag;                     //trriger flag
    }long_press, long_release;

    struct event_cnt_t press_long_event;       //press long- press event
    struct event_cnt_t release_long_event;     //release long- press event
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
    struct {
        unsigned int count;                    //multi press count
        unsigned int timecnt;                  //Record the time since the last click
        unsigned int interval;                 //Maximum allowed interval between two consecutive clicks
    }multi_click_press, multi_click_release;

    struct event_cnt_t press_multi_event;      //Press multi hit event count
    struct event_cnt_t release_multi_event;    //Release multi hit event count
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
    struct {
        unsigned int trriger_time;             //Trigger time
        unsigned int trriger_period;           //Trigger period
        unsigned int press_time;               //Time Pressed
    }continuous;

    struct event_cnt_t press_continuous_event; //Continuous press event count
#endif
};

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
struct match_list_t {
    const struct key_combine_t *match;     //Combination status to be matched
    unsigned int n;                        //Record the number of digits of combined status
    unsigned int match_count;              //Matched Success Count
    unsigned char flag;
    struct match_list_t *next;             //Combination status for linking the next registration

    struct event_cnt_t combine_event;      //Combined Event Count
};                                     //Registered chain header in combination status

struct {
    unsigned int timecnt;                      //Record the time since the last trigger
    unsigned int interval;                     //Maximum allowable interval between two consecutive triggering
    struct match_list_t head;
}static combine;
#endif

struct key_board_t {
    enum key_board_type_t type;                //Keyboard Type
    unsigned int sig_num;                      //Number of signal lines
    struct key_private_t *sig;                 //Relevant information of signal line
    unsigned int ctrl_num;                     //Number of control lines
    unsigned int sig_per_ctrl;                 //Number of signal lines on each control line
    struct key_public_ctrl_t *ctrl;            //Relevant information of control line
};
typedef struct{
    bool bEventHappen;
	void (*cbFun)(void);
}key_app_t;
static struct key_board_t *key_board[KEY_BOARD_MAX_NUM];
static unsigned int key_hash_map[KEY_MAX_NUM]; //Used to quickly find the object of the key according to the id (linear detection method)
static unsigned int key_press_count; //Used to record the count of keys currently pressed
static print_debug_callback internal_print_debug; //Debug information output
static unsigned int key_tick_ms; //tick
static struct key_board_t obj;
static struct key_private_t esig[KEY_MAX_NUM];
static struct match_list_t combin_item[KEY_COMBINE_NUM];
static key_app_t t_key_app;
static inline void debug(const char *func, int line, const char *message)
{
#if USER_KEY_DEBUG
    #include <stdio.h>

    static char buff[128];

    sprintf(buff, "func:%s, line:%u, %s\n", func, line, message);

    if(internal_print_debug)
    {
        internal_print_debug(buff);
    }
 #endif
}

int key_board_init(void)
{
    memset(key_board, 0, sizeof(key_board));
    for(unsigned int i= 0;i < GET_ARRAY_SIZE(key_hash_map);i++)
    {
        key_hash_map[i] = (unsigned int)-1;
    }
    key_press_count = 0;
    t_key_app.bEventHappen = false;
    t_key_app.cbFun = (void*)NULL;
#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    memset(&combine, 0, sizeof(combine));
    memset(&combin_item[0], 0, GET_ARRAY_SIZE(combin_item)*sizeof(struct match_list_t));
    combine.head.next = NULL;
#endif
    key_tick_ms = 0;
    return 0;
}

struct key_board_t *key_board_register(enum key_board_type_t type, 
                                       const struct key_public_sig_t sig[], 
                                       unsigned int key_sig_n, 
                                       const struct key_public_ctrl_t ctrl[], 
                                       unsigned int key_ctrl_n,
                                       void (*fun)(void))
{
    //struct key_board_t *obj;
    unsigned int handle_no_use;
    unsigned int i;

    for(handle_no_use = 0;handle_no_use < GET_ARRAY_SIZE(key_board) && key_board[handle_no_use];handle_no_use++);
    if(handle_no_use >= GET_ARRAY_SIZE(key_board))
    {
        debug(__FUNCTION__, __LINE__, "exceed the max configure value [KEY_BOARD_MAX_NUM]");
        return NULL;
    }
    memset(&obj, 0, sizeof(struct key_board_t));
    key_board[handle_no_use] = &obj;
    obj.type = type;
    obj.sig_num = key_sig_n;
    obj.sig = &esig[0];
    
    memset(obj.sig, 0, KEY_MAX_NUM * sizeof(struct key_private_t));
    for(i = 0;i < key_sig_n;i++)
    {
        obj.sig[i].property = (struct key_public_sig_t *)&sig[i];

        obj.sig[i].init_debounce_time = KEY_TIME_CHECK(KEY_DEFAULT_DEBOUNCE_TIME);

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
        obj.sig[i].long_press.trriger_time = KEY_TIME_CHECK(KEY_DEFAULT_LONG_TRRIGER_TIME);
        obj.sig[i].long_release.trriger_time = KEY_TIME_CHECK(KEY_DEFAULT_LONG_TRRIGER_TIME);
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
        obj.sig[i].multi_click_press.interval = KEY_TIME_CHECK(KEY_DEFAULT_MULTI_INTERVAL_TIME);
        obj.sig[i].multi_click_release.interval = KEY_TIME_CHECK(KEY_DEFAULT_MULTI_INTERVAL_TIME);
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
        obj.sig[i].continuous.trriger_time = KEY_TIME_CHECK(KEY_DEFAULT_CONTINUOUS_INIT_TRRIGER_TIME);
        obj.sig[i].continuous.trriger_period = KEY_TIME_CHECK(KEY_DEFAULT_CONTINUOUS_PERIOD_TRRIGER_TIME);
#endif

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
        combine.interval = KEY_TIME_CHECK(KEY_DEFAULT_COMBINE_INTERVAL_TIME);
#endif
        //Get the insertion position by taking the mold
        unsigned int idx, backup;

        backup = idx = obj.sig[i].property->id % GET_ARRAY_SIZE(key_hash_map);
        while(key_hash_map[idx] != (unsigned int)-1)
        {
            if(obj.sig[key_hash_map[idx]].property->id == obj.sig[i].property->id)
            {
                debug(__FUNCTION__, __LINE__, "the key id is already register");
                //id exists
                return NULL;
            }
            idx = (idx + 1) % GET_ARRAY_SIZE(key_hash_map);
            if(idx == backup)
            {
                debug(__FUNCTION__, __LINE__, "exceed the max configure value [KEY_MAX_NUM]");
                //No empty position
                return NULL;
            }
        }
        key_hash_map[idx] = i;
    }
    if(obj.type == KEY_BOARD_MATRIX)
    {
        obj.ctrl_num = key_ctrl_n;
        obj.sig_per_ctrl = key_sig_n / key_ctrl_n;
        obj.ctrl = (struct key_public_ctrl_t *)ctrl;
        for(i = 0;i < key_ctrl_n;i++)
        {
            //Pull the control line to the invalid level
            ctrl[i].set(ctrl[i].pin_desc, false);
        }
    }
 #if (KEY_EVENT_TRIG_MODE == KEY_TRIG_REPORT)    
    t_key_app.cbFun = fun;  //Registering Callbacks
 #else
    t_key_app.cbFun = (void*)NULL;
    #endif
    return &obj;
}

static inline unsigned int key_event(struct event_cnt_t *ev)
{
    unsigned int ret;

    ret = 0;
    if(ev->wr_cnt != ev->rd_cnt)
    {
        ++ev->rd_cnt;
        ret = 1;
    }
    return ret;
}

unsigned int key_check_state(unsigned int id, enum key_state_t state)
{
    struct key_board_t *obj;
    unsigned int i;
    unsigned int idx, backup;

    for(i = 0;i < GET_ARRAY_SIZE(key_board) && key_board[i];i++)
    {
        obj = key_board[i];

        //If the ID to be searched corresponds to a vacancy in the hash map, the ID must not exist
        backup = idx = id % GET_ARRAY_SIZE(key_hash_map);
        while(key_hash_map[idx] != (unsigned int)-1)
        {
			
			if(key_hash_map[idx] > obj->sig_num){ platform_printf("==>hash:%d,%d\n",idx,key_hash_map[idx]); break;}
            if(obj->sig[key_hash_map[idx]].property->id == id)
            {
                switch(state)
                {
                    case KEY_RELEASE:
                        return key_event(&obj->sig[key_hash_map[idx]].release_event);
                    case KEY_PRESS:
                        return key_event(&obj->sig[key_hash_map[idx]].press_event);
                    case KEY_PRESSING:
                        return key_event(&obj->sig[key_hash_map[idx]].pressing_event);
#if (KEY_LONG_SUPPORT == KEY_ENABLE)
                    case KEY_PRESS_LONG:
                        return key_event(&obj->sig[key_hash_map[idx]].press_long_event);
                    case KEY_RELEASE_LONG:
                        return key_event(&obj->sig[key_hash_map[idx]].release_long_event);
#endif
#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
                    case KEY_PRESS_MULTI:
                        return key_event(&obj->sig[key_hash_map[idx]].press_multi_event) ? obj->sig[key_hash_map[idx]].multi_click_press.count : 0;
                    case KEY_RELEASE_MULTI:
                        return key_event(&obj->sig[key_hash_map[idx]].release_multi_event) ? obj->sig[key_hash_map[idx]].multi_click_release.count : 0;
#endif
#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
                    case KEY_PRESS_CONTINUOUS:
                        return key_event(&obj->sig[key_hash_map[idx]].press_continuous_event);
#endif
                    default :
                        return 0;
                }
            }
            idx = (idx + 1) % GET_ARRAY_SIZE(key_hash_map);
            if(idx == backup)
            {
                break;
            }
        }
    }

    return 0;
}

unsigned int key_press_count_get(void)
{
    return key_press_count;
}

static inline void key_press_count_stats(struct key_private_t *sig)
{
    if(sig->state & KEY_PRESS)
    {
        ++key_press_count;
    }
    else if(sig->state & KEY_RELEASE)
    {
        --key_press_count;
    }
}

static unsigned char item_count = 0;

int key_combine_register(const struct key_combine_t c[], unsigned int n)
{
#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    if(item_count >= KEY_COMBINE_NUM)
    {
        debug(__FUNCTION__, __LINE__, "the combine key number overflow");
        return -1;
    }
    combin_item[item_count].match = c;
    combin_item[item_count].n = n;
    combin_item[item_count].flag &= 0;
    combin_item[item_count].match_count = 0;
    combin_item[item_count].next = combine.head.next;
    combin_item[item_count].combine_event.rd_cnt = 0;
    combin_item[item_count].combine_event.wr_cnt = 0;
    combine.head.next = &combin_item[item_count];
    return (unsigned int)&combin_item[item_count++];
#else
    return 0;
#endif
}

unsigned int key_check_combine_state(int id)
{
#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    if((-1 == id)||(0 == id))
        return 0;
    struct match_list_t *item;

    item = (struct match_list_t *)id;

    return key_event(&item->combine_event);
#else
    return 0;
#endif
}

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
static inline void key_check_combine_pre(void)
{
    struct match_list_t *item;
    if(combine.timecnt && (key_tick_ms - combine.timecnt < KEY_MAX_TIME))
    {
       combine.timecnt = 0;
       item = combine.head.next;
       while(item)
        {
            item->match_count = 0;
            item->flag &= 0;
            item = item->next;
        }
    }
}

static inline void key_check_combine(struct key_private_t *sig)
{
    struct match_list_t *item;
    item = combine.head.next;
    while(item)
    {
        for(int i = 0;i < item->n;i++)
        {   
            if((item->match[i].state & sig->state)&&(item->match[i].id == sig->property->id))
            {
               //Retime
                combine.timecnt = combine.interval + key_tick_ms;               
                //Matching succeeded
                if(!(item->flag & (1<<i)))
                {                 
                    item->flag |= 1<<i;
//                    platform_printf("0x%02x,%d,%d\n",item->flag,item->match_count,item->n);
                    if(++item->match_count == item->n)
                    {
                        item->flag &= 0;
                        item->match_count = 0;
						t_key_app.bEventHappen = true;
                        ++item->combine_event.wr_cnt;

                    }              
                }
            }
        }
        item = item->next;
    }
}
#endif

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
static inline void key_check_long(struct key_private_t *sig)
{
    if(sig->state & KEY_PRESS)
    {
        sig->long_press.press_time = sig->long_press.trriger_time + key_tick_ms;
        sig->long_release.press_time = sig->long_release.trriger_time + key_tick_ms;
        sig->long_press.trriger_flag = false;
    }
    else if(sig->state & (KEY_PRESSING | KEY_PRESSING_DEBOUNCING))
    {
        if((sig->property->function & KEY_FLAG_PRESS_LONG) && (key_tick_ms - sig->long_press.press_time < KEY_MAX_TIME))
        {
            if(!sig->long_press.trriger_flag)
            {
                //Trigger only once
                sig->long_press.trriger_flag = true;
                sig->state |= KEY_PRESS_LONG;
                ++sig->press_long_event.wr_cnt;
				t_key_app.bEventHappen = true;
            }
        }
    }
    else if(sig->state & KEY_RELEASE)
    {
        if((sig->property->function & KEY_FLAG_RELEASE_LONG) && (key_tick_ms - sig->long_release.press_time < KEY_MAX_TIME))
        {
            sig->state |= KEY_RELEASE_LONG;
            ++sig->release_long_event.wr_cnt;
			t_key_app.bEventHappen = true;
        }
    }
}
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
static inline void key_check_multi(struct key_private_t *sig)
{
    if((sig->property->function & KEY_FLAG_PRESS_MULTI) && (sig->state & KEY_PRESS))
    {
        //Mark press times
        ++sig->multi_click_press.count;
        if(sig->multi_click_press.count >= 2)
        {
            sig->state |= KEY_PRESS_MULTI;
            ++sig->press_multi_event.wr_cnt;
			t_key_app.bEventHappen = true;
        }
        sig->multi_click_press.timecnt = sig->multi_click_press.interval + key_tick_ms;
    }
    if(sig->multi_click_press.count)
    {
        if(key_tick_ms - sig->multi_click_press.timecnt < KEY_MAX_TIME)
        {
            sig->multi_click_press.count = 0;
        }
    }

    if((sig->property->function & KEY_FLAG_RELEASE_MULTI) && (sig->state & KEY_RELEASE))
    {
        //Mark bounce times
        ++sig->multi_click_release.count;
        if(sig->multi_click_release.count >= 2)
        {
            sig->state |= KEY_RELEASE_MULTI;
            ++sig->release_multi_event.wr_cnt;
        }
        sig->multi_click_release.timecnt = sig->multi_click_release.interval + key_tick_ms;
    }
    if(sig->multi_click_release.count)
    {
        if(key_tick_ms - sig->multi_click_release.timecnt < KEY_MAX_TIME)
        {
            sig->multi_click_release.count = 0;
        }
    }
}
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
static inline void key_check_continuous(struct key_private_t *sig)
{
    if(sig->state & KEY_PRESS)
    {
        sig->continuous.press_time = sig->continuous.trriger_time + key_tick_ms;
    }
    else if(sig->state & (KEY_PRESSING | KEY_PRESSING_DEBOUNCING))
    {
        if((sig->property->function & KEY_FLAG_PRESS_CONTINUOUS) && (key_tick_ms - sig->continuous.press_time < KEY_MAX_TIME))
        {
            sig->state |= KEY_PRESS_CONTINUOUS;
            ++sig->press_continuous_event.wr_cnt;
			t_key_app.bEventHappen = true;
            sig->continuous.press_time = sig->continuous.trriger_period + key_tick_ms;
        }
    }
}
#endif

void key_check(void)
{
    struct key_board_t *pObj;
    unsigned int i, j;
    unsigned int last, next;
    //struct key_pin_t *pdescx,*pdescy;
    ++key_tick_ms;
#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    key_check_combine_pre();
#endif
    for(i = 0;i < GET_ARRAY_SIZE(key_board) && key_board[i];i++)
    {
        pObj = key_board[i];

        last = pObj->ctrl_num - 1;
        next = 0;
        for(j = 0;j < pObj->sig_num;j++)
        {
            if((pObj->type == KEY_BOARD_MATRIX) && ((j % pObj->sig_per_ctrl) == 0))
            {
                //The last scanned control line is pulled to an invalid level
                pObj->ctrl[last].set(pObj->ctrl[last].pin_desc, false);

                last = next = j / pObj->sig_per_ctrl;

                //Pull the control line to be scanned to the active level
                pObj->ctrl[next].set(pObj->ctrl[next].pin_desc, true);
				printf("matrix \n");
            }
            pObj->sig[j].this_level = pObj->sig[j].property->get(pObj->sig[j].property->pin_desc);
			//printf("j:%d levevl:%d\n",j,pObj->sig[j].this_level);
            switch((pObj->sig[j].this_level << 4) | (pObj->sig[j].last_level))
            {
                case 0x00:
                    //Clear 0 to solve the BUG caused by multiple debounce time
                    pObj->sig[j].debounce_time = 0;
                    pObj->sig[j].state = KEY_NONE;
                    break;
                case 0x01:
                    if((0 == pObj->sig[j].init_debounce_time) ||
                       (pObj->sig[j].debounce_time && (key_tick_ms - pObj->sig[j].debounce_time < KEY_MAX_TIME)))
                    {
                        //No need to debounce or in the debounce time state and reach the shake elimination timing
                        pObj->sig[j].state = KEY_RELEASE;
                        ++pObj->sig[j].release_event.wr_cnt;
						t_key_app.bEventHappen = true;
                    }
                    else if(0 == pObj->sig[j].debounce_time)
                    {
                        pObj->sig[j].state = KEY_PRESSING_DEBOUNCING;
                        //Reset the debounce timing
                        pObj->sig[j].debounce_time = pObj->sig[j].init_debounce_time + key_tick_ms;
                    }
                    break;
                case 0x10:
                    if((0 == pObj->sig[j].init_debounce_time) ||
                       (pObj->sig[j].debounce_time && (key_tick_ms - pObj->sig[j].debounce_time < KEY_MAX_TIME)))
                    {
                        pObj->sig[j].state = KEY_PRESS;
						t_key_app.bEventHappen = true;
                       // platform_printf("x:%d,y:%d\r\n",pdescx->pin,pdescy->pin);
                        ++pObj->sig[j].press_event.wr_cnt;
                    }
                    else if(0 == pObj->sig[j].debounce_time)
                    {
                        pObj->sig[j].state = KEY_PRESS_DEBOUNCING;
                        pObj->sig[j].debounce_time = pObj->sig[j].init_debounce_time + key_tick_ms;
                    }
                    break;
                case 0x11:
                    //Clear 0 to solve the BUG caused by multiple debounce time
                    pObj->sig[j].debounce_time = 0;
                    pObj->sig[j].state = KEY_PRESSING;
                    ++pObj->sig[j].pressing_event.wr_cnt;
					t_key_app.bEventHappen = true;
                    break;
            }

            //If it is in the process of dithering elimination, the current level state will not be saved
            if(!(pObj->sig[j].state & (KEY_PRESS_DEBOUNCING | KEY_PRESSING_DEBOUNCING)))
            {
                pObj->sig[j].last_level = pObj->sig[j].this_level;
            }

            key_press_count_stats(&pObj->sig[j]);

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
            key_check_long(&pObj->sig[j]);
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
            key_check_multi(&pObj->sig[j]);
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
            key_check_continuous(&pObj->sig[j]);
#endif

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
            key_check_combine(&pObj->sig[j]);
#endif
			if((t_key_app.bEventHappen)&&(NULL != t_key_app.cbFun))
			{
				t_key_app.cbFun();
				t_key_app.bEventHappen = false;
			}
		}
    }
}

void key_board_debug_register(print_debug_callback print_debug)
{
    internal_print_debug = print_debug;
}
