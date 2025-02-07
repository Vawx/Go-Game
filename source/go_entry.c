
#include "go_types.h"

#define BOARD_SMALL_DIMENSION  9
#define BOARD_MEDIUM_DIMENSION 13
#define BOARD_LARGE_DIMENSION  19

#define BOARD_COLOR CLITERAL(Color){ 178, 109, 57, 255 }

#define NOTIFY_FADE_IN_OUT_TIME 0.5f

typedef enum go_game_toast_notify_state {
    TOAST_NOTIFY_STATE_NONE, 
    TOAST_NOTIFY_STATE_FADE_IN,
    TOAST_NOTIFY_STATE_FADE_OUT,
    TOAST_NOTIFY_STATE_DISPLAYING,
} go_game_toast_notify_state;

typedef struct go_game_toast_notify {
    r32 display_time;
    u8 fade_in;
    u8 fade_out;
    char msg[255];
    u8 msg_len;
    
    r32 current_display_time;
    r32 current_fade_in_time;
    r32 current_fade_out_time;
    u8 active;
    go_game_toast_notify_state state;
} go_game_toast_notify;

typedef enum go_stone_type {
    GO_STONE_TYPE_NONE,
    GO_STONE_TYPE_BLACK,
    GO_STONE_TYPE_WHITE
} go_stone_type;

typedef struct go_stone {
    go_stone_type type;
    u16 idx;
    Vector2 pos;
} go_stone;

typedef struct go_stone_color {
    Color background;
    Color foreground;
} go_stone_color;

#define INTERSECTIONS_COUNT 32
#define NOTIFIES_COUNT      32

typedef struct go_board {
    go_stone *stones; // this is _every_ board piece. including those without a stone
    u32 stones_idx;
    
    s32 dimension;
    
    s32 intersections_x[INTERSECTIONS_COUNT];
    s32 intersections_y[INTERSECTIONS_COUNT];
    u32 intersections_x_count = 0;
    u32 intersections_y_count = 0;
    
    go_stone_type turn;
    
    go_game_toast_notify notifies[NOTIFIES_COUNT];
} go_board;

static go_board board;

static u32 window_width = 1920;
static u32 window_height = 1920;

// background and grid
static const s32 x = -(window_width / 2) + ((s32)(window_width * 0.05f));
static const s32 y = -(window_height / 2) + ((s32)(window_height * 0.05f));
static const s32 w = window_width - ((s32)(window_width * 0.05f) * 2);
static const s32 h = window_height - ((s32)(window_height * 0.05f) * 2);

static void init_board(u8 dim) {
    if(!board.stones) {
        board.stones = (go_stone*)malloc(sizeof(go_stone) * (BOARD_SMALL_DIMENSION * BOARD_SMALL_DIMENSION));
    }
    board.stones_idx = 0;
    board.dimension = dim;
    board.turn = GO_STONE_TYPE_BLACK;
    
    memset(board.notifies, 0, sizeof(go_game_toast_notify) * NOTIFIES_COUNT);
}

static go_stone_color get_color_for_type(go_stone_type type) {
    go_stone_color result = {0};
    
    switch(type) {
        case GO_STONE_TYPE_WHITE: {
            result.foreground = WHITE;
            result.background = {220, 220, 220, 128};
        } break;
        case GO_STONE_TYPE_BLACK: {
            result.foreground = {8, 8, 8, 255};
            result.background = {30, 30, 30, 255};
        } break;
        default: {
            result.foreground = RED;
            result.background = GREEN;
        } break;
    }
    
    return result;
}

static void init_intersections() {
    s32 x = -(window_width / 2) + ((s32)(window_width * 0.05f));
    s32 y = -(window_height / 2) + ((s32)(window_height * 0.05f));
    s32 w = window_width - ((s32)(window_width * 0.05f) * 2);
    s32 h = window_height - ((s32)(window_height * 0.05f) * 2);
    
    s32 grid_start_x = x + ((w / board.dimension) / 2);
    s32 grid_start_y = y - ((y / board.dimension));
    board.intersections_x[board.intersections_x_count++] = grid_start_x;
    board.intersections_y[board.intersections_y_count++] = grid_start_y;
    
    for(s32 i = 0; i < board.dimension; ++i) {
        board.intersections_x[board.intersections_x_count++] = grid_start_x;
        grid_start_x += (w / board.dimension);
    }
    
    grid_start_x = x + ((w / board.dimension) / 2);
    grid_start_y = y - ((y / board.dimension));
    
    for(s32 i = 0; i < board.dimension; ++i) {
        board.intersections_y[board.intersections_y_count++] = grid_start_y;
        grid_start_y += (h / board.dimension);
    }
}

static Vector2 get_nearest_intersection_from_mouse_pos() {
    Vector2 result = {4096, 4096};
    r32 result_dist = 99999999999.f;
    Vector2 mp = GetMousePosition();
    mp.x = -1.f * ((window_width / 2) - mp.x);
    mp.y = -1.f * ((window_height / 2) - mp.y);
    
    for(s32 i = 0; i < board.intersections_x_count; ++i) {
        for(s32 j = 0; j < board.intersections_y_count; ++j) {
            Vector2 intersection = {board.intersections_x[i], board.intersections_y[j]};
            r32 dist = Vector2Distance(mp, intersection);
            if(dist < result_dist) {
                result_dist = dist;
                result = intersection;
            }
        }
    }
    
    return result;
}

static void draw_board() {
    // background 
    DrawRectangle(x, y, w, h, BOARD_COLOR);
    
    // grid
    {
        s32 grid_start_x = x + ((w / board.dimension) / 2);
        s32 grid_start_y = y - ((y / board.dimension));
        
        // draw y -> bottom - top
        for(s32 i = 0; i < board.dimension; ++i) {
            DrawLineEx({(r32)grid_start_x, (r32)grid_start_y}, {(r32)grid_start_x, (r32)((y + h) + (y / board.dimension))}, 4, BLACK);
            
            grid_start_x += (w / board.dimension);
        }
        
        // draw x left - right
        grid_start_x = x + ((w / board.dimension) / 2);
        grid_start_y = y - ((y / board.dimension));
        
        for(s32 i = 0; i < board.dimension; ++i) {
            DrawLineEx({(r32)grid_start_x, (r32)grid_start_y}, {(r32)((x + w) + (x / board.dimension)), (r32)grid_start_y}, 4, BLACK);
            grid_start_y += (h / board.dimension);
        }
    }
    
    // stars
    {
        Vector2 star_pos[5] = {
            {board.intersections_x[1 + board.dimension / 4], board.intersections_y[1 + board.dimension / 4]},
            {board.intersections_x[board.dimension - (board.dimension / 4)], board.intersections_y[1 + board.dimension / 4]},
            {board.intersections_x[1 + board.dimension / 4], board.intersections_y[board.dimension - (board.dimension / 4)]},
            {board.intersections_x[board.dimension - (board.dimension / 4)], board.intersections_y[board.dimension - (board.dimension / 4)]},
            {0.f, 0.f},
        };
        for(s32 i = 0; i < 5; ++i) {
            DrawCircle(star_pos[i].x, star_pos[i].y, 16, BLACK);
        }
    }
}

static void draw_stones() {
    const r32 rad = ((w / board.dimension) / 2) - (w * 0.0035f);
    const r32 back_rad = rad + 3;
    
    for(u32 i = 0; i < board.stones_idx; ++i) {
        go_stone *stone = &board.stones[i];
        go_stone_color color = get_color_for_type(stone->type);
        
        DrawCircle(stone->pos.x, stone->pos.y, back_rad, BLACK);
        DrawCircleGradient(stone->pos.x, stone->pos.y, rad, color.foreground, color.background);
    }
}

static void draw_player_turn() {
    Vector2 best = get_nearest_intersection_from_mouse_pos();
    
    const r32 rad = ((w / board.dimension) / 2) - (w * 0.0035f);
    const r32 back_rad = rad + 3;
    
    go_stone_color color = get_color_for_type(board.turn);
    
    DrawCircle(best.x, best.y, back_rad, BLACK);
    DrawCircleGradient(best.x, best.y, rad, color.foreground, color.background);
}

static u8 is_stone_used(Vector2 pos) {
    for(s32 i = 0; i < board.stones_idx; ++i) {
        go_stone *stone = &board.stones[i];
        if(stone->pos.x == pos.x && stone->pos.y == pos.y) {
            return 1;
        }
    }
    return 0;
}

static go_stone *get_stone_at_position(Vector2 pos) {
    for(s32 i = 0; i < board.stones_idx; ++i) {
        go_stone *stone = &board.stones[i];
        if(stone->pos.x == pos.x && stone->pos.y == pos.y) {
            return stone;
        }
    }
    return NULL;
}

static void push_toast_notification(const char *msg, const r32 display_time) {
    for(s32 i = 0; i < NOTIFIES_COUNT; ++i) {
        go_game_toast_notify *notify = &board.notifies[i];
        if(!notify->active) {
            notify->active = 1;
            notify->display_time = display_time;
            notify->fade_in = 0;
            notify->fade_out = 0;
            notify->msg_len = (u8)strlen(msg);
            memcpy(notify->msg, msg, notify->msg_len);
            notify->current_display_time = 0.f;
            notify->current_fade_in_time = 0.f;
            notify->current_fade_out_time = 0.f;
            notify->state = TOAST_NOTIFY_STATE_FADE_IN;
            return;
        }
    }
}

static void player_input() {
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if(board.stones_idx >= board.dimension * board.dimension) {
            printf("BOARD FULL\n");
        } else {
            Vector2 best = get_nearest_intersection_from_mouse_pos();
            if(!is_stone_used(best)) {
                go_stone *stone = &board.stones[board.stones_idx];
                stone->type = board.turn;
                stone->idx = board.stones_idx;
                stone->pos = best;
                
                ++board.stones_idx;
                board.turn = (board.turn == GO_STONE_TYPE_BLACK) ? GO_STONE_TYPE_WHITE : GO_STONE_TYPE_BLACK;
                SetMousePosition((w / 2) + (w / board.dimension), (h / 2) + (h / board.dimension));
            } else {
                printf("Stone already in place!\n");
            }
        }
    }
    
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        static u8 toggle = 0;
        if(toggle == 0) {
            push_toast_notification("this is a test", 2);
            toggle = 1;
        } else {
            push_toast_notification("womp womp!", 2);
            toggle = 0;
        }
    }
}

static void handle_toast_notifications() {
    for(s32 i = 0; i < 32; ++i) {
        go_game_toast_notify *notify = &board.notifies[i];
        if(!notify->active) {
            continue;
        }
        
        switch(notify->state) {
            case TOAST_NOTIFY_STATE_FADE_IN: {
                notify->current_fade_in_time += GetFrameTime();
                if(notify->current_fade_in_time > NOTIFY_FADE_IN_OUT_TIME) {
                    notify->state = TOAST_NOTIFY_STATE_DISPLAYING;
                }
            } break;
            case TOAST_NOTIFY_STATE_FADE_OUT: {
                notify->current_fade_out_time += GetFrameTime();
                if(notify->current_fade_out_time > NOTIFY_FADE_IN_OUT_TIME) {
                    notify->active = 0;
                    notify->display_time = 0.f;
                    notify->fade_in = 0;
                    notify->fade_out = 0;
                    memset(notify->msg, 0, 255);
                    notify->msg_len = 0;
                    notify->current_display_time = 0.f;
                    notify->current_fade_in_time = 0.f;
                    notify->current_fade_out_time = 0.f;
                    notify->state = TOAST_NOTIFY_STATE_NONE;
                }
            } break;
            case TOAST_NOTIFY_STATE_DISPLAYING: {
                notify->current_display_time += GetFrameTime();
                if(notify->current_display_time > notify->display_time) {
                    notify->state = TOAST_NOTIFY_STATE_FADE_OUT;
                }
            } break;
        }
        return;// only 1
    }
}

static r32 toast_notification_alpha_from_state_and_time(go_game_toast_notify *notify) {
    if(!notify->active) {
        __debugbreak();
    }
    
    switch(notify->state) {
        case TOAST_NOTIFY_STATE_FADE_IN: {
            return notify->current_fade_in_time / NOTIFY_FADE_IN_OUT_TIME;
        } break;
        case TOAST_NOTIFY_STATE_FADE_OUT: {
            return 1.f - (notify->current_fade_out_time / NOTIFY_FADE_IN_OUT_TIME);
        } break;
        case TOAST_NOTIFY_STATE_DISPLAYING: {
            return 1.f;
        } break;
    }
    
    return 0.f;
}

static void display_toast_notifications() {
    static const s32 DEFAULT_TOAST_SIZE_WIDTH = w * 0.35f;
    static const s32 DEFAULT_TOAST_SIZE_HEIGHT = h * 0.1f;
    
    Vector2 start_pos = {x, y};
    for(s32 i = 0; i < NOTIFIES_COUNT; ++i) {
        go_game_toast_notify *notify = &board.notifies[i];
        if(!notify->active) {
            continue;
        }
        
        Color background_color = {50, 50, 120, 0.f};
        r32 alpha = toast_notification_alpha_from_state_and_time(notify);
        background_color.a = (s32)(255.f * alpha);
        
        Color boarder_color = BLACK;
        boarder_color.a = background_color.a;
        
        DrawRectangle(((start_pos.x + (w * 0.5f)) - (DEFAULT_TOAST_SIZE_WIDTH / 2)) - 6, (-y - (DEFAULT_TOAST_SIZE_HEIGHT * 2.f)) - 6, DEFAULT_TOAST_SIZE_WIDTH + 12, DEFAULT_TOAST_SIZE_HEIGHT + 12, boarder_color);
        
        DrawRectangle((start_pos.x + (w * 0.5f)) - (DEFAULT_TOAST_SIZE_WIDTH / 2), -y - (DEFAULT_TOAST_SIZE_HEIGHT * 2.f), DEFAULT_TOAST_SIZE_WIDTH, DEFAULT_TOAST_SIZE_HEIGHT, background_color);
        
        u32 pixel_text_offset = (notify->msg_len * 32) / 2;
        DrawText(notify->msg, ((start_pos.x + (w * 0.5f)) - (DEFAULT_TOAST_SIZE_WIDTH / 2)) + (start_pos.x + (w * 0.5f)) + (DEFAULT_TOAST_SIZE_WIDTH / 2) - pixel_text_offset, (-y - (DEFAULT_TOAST_SIZE_HEIGHT * 2.f)) + (DEFAULT_TOAST_SIZE_HEIGHT / 2) - 16, 32, WHITE);
        return; // only display 1
    }
}

int main(int argc, char **argv) {
    
    InitWindow(window_width, window_height, "GO");
    
    Camera2D camera = { 0 };
    camera.target = { 0.f, 0.f};
    camera.offset = { window_width / 2.f, window_height / 2.f };
    camera.rotation = 0.f;
    camera.zoom = 1.f;
    
    SetTargetFPS(60);
    
    init_board(BOARD_SMALL_DIMENSION);
    init_intersections();
    
    while (!WindowShouldClose()) {
        player_input();
        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera);
        draw_board();
        draw_stones();
        draw_player_turn();
        
        handle_toast_notifications();
        display_toast_notifications();
        EndMode2D();
        
        EndDrawing();
    }
    CloseWindow();
    return 0;
}