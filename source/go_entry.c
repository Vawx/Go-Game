
#include "go_types.h"

#define BOARD_SMALL_DIMENSION  9
#define BOARD_MEDIUM_DIMENSION 13
#define BOARD_LARGE_DIMENSION  19

#define BOARD_COLOR CLITERAL(Color){ 178, 109, 57, 255 }

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

typedef struct go_board {
    go_stone *stones; // this is _every_ board piece. including those without a stone
    u32 stones_idx;
    
    s32 dimension;
    
    s32 intersections_x[32];
    s32 intersections_y[32];
    u32 intersections_x_count = 0;
    u32 intersections_y_count = 0;
    
    go_stone_type turn;
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
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if(board.stones_idx >= board.dimension * board.dimension) {
                printf("BOARD FULL\n");
            } else {
                Vector2 best = get_nearest_intersection_from_mouse_pos();
                go_stone *stone = &board.stones[board.stones_idx];
                stone->type = board.turn;
                stone->idx = board.stones_idx;
                stone->pos = best;
                
                ++board.stones_idx;
                board.turn = (board.turn == GO_STONE_TYPE_BLACK) ? GO_STONE_TYPE_WHITE : GO_STONE_TYPE_BLACK;
                SetMousePosition((w / 2) + (w / board.dimension), (h / 2) + (h / board.dimension));
            }
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        BeginMode2D(camera);
        draw_board();
        draw_stones();
        draw_player_turn();
        EndMode2D();
        
        EndDrawing();
    }
    CloseWindow();
    return 0;
}