#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp = 0.f;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 130, ball_dp_y, ball_half_size = 1;

int player_1_score, player_2_score;

internal void
simulate_player(float *p, float *dp, float ddp, float dt) {
    ddp -= *dp * 10.f;

    *p = *p + *dp * dt + ddp * dt * dt * .5f;
    *dp = *dp + ddp * dt;

    if (*p + player_half_size_y > arena_half_size_y) {
        *p = arena_half_size_y - player_half_size_y;
        *dp = 0;
    }
    else if (*p - player_half_size_y < -arena_half_size_y) {
        *p = -arena_half_size_y + player_half_size_y;
        *dp = 0;
    }
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
    float p2x, float p2y, float hs2x, float hs2y) {
    return (p1x + hs1x > p2x - hs2x &&
        p1x - hs1x < p2x + hs2x &&
        p1y + hs1y > p2y - hs2y &&
        p1y + hs1y < p2y + hs2y);
}

struct Controls {
    int player_1_up;
    int player_1_down;
    int player_2_up;
    int player_2_down;
};

enum GameMode {
    GM_MENU,
    GM_CONTROLS,
    GM_GAMEPLAY
};

GameMode current_gamemode;
int hot_button = 0;
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt) {
    draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa32);
    draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);

    if (current_gamemode == GM_GAMEPLAY) {
        if (pressed(BUTTON_ESC)) {
            current_gamemode = GM_MENU;
        }

        float player_1_ddp = 0.f;
        if (!enemy_is_ai) {
            if (is_down(BUTTON_UP)) player_1_ddp += 2000;
            if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
        }
        else {
            player_1_ddp = (ball_p_y - player_1_p) * 100;
            if (player_1_ddp > 1300) player_1_ddp = 1300;
            if (player_1_ddp < -1300) player_1_ddp = -1300;
        }

        float player_2_ddp = 0.f;
        if (is_down(BUTTON_W)) player_2_ddp += 2000;
        if (is_down(BUTTON_S)) player_2_ddp -= 2000;

        simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
        simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);

        //Simulate Ball
        {
            ball_p_x += ball_dp_x * dt;
            ball_p_y += ball_dp_y * dt;

            if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
                ball_p_x = 80 - player_half_size_x - ball_half_size;
                ball_dp_x *= -1;
                ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f;
            }
            else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
                ball_p_x = -80 + player_half_size_x + ball_half_size;
                ball_dp_x *= -1;
                ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f;
            }

            if (ball_p_y + ball_half_size > arena_half_size_y) {
                ball_p_y = arena_half_size_y - ball_half_size;
                ball_dp_y *= -1;
            }
            else if (ball_p_y - ball_half_size < -arena_half_size_y) {
                ball_p_y = -arena_half_size_y + ball_half_size;
                ball_dp_y *= -1;
            }

            if (ball_p_x + ball_half_size > arena_half_size_x) {
                ball_dp_x *= -1;
                ball_dp_y = 0;
                ball_p_x = 0;
                ball_p_y = 0;
                player_1_score++;
            }
            else if (ball_p_x - ball_half_size < -arena_half_size_x) {
                ball_dp_x *= -1;
                ball_dp_y = 0;
                ball_p_x = 0;
                ball_p_y = 0;
                player_2_score++;
            }
        }

        draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
        draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

        //Rendering
        draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

        draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
        draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);
        draw_text("ESC TO GO BACK", 46, 48.5, .50, 0xffffff);
    }
    else if (current_gamemode == GM_CONTROLS) {
        draw_text("Player 1 up key : 'w'", -80, 40, .75, 0xff0000);
        draw_text("Player 1 down key : 's'", -80, 30, .75, 0xff0000);
        draw_text("Player 2 up key : up arrow", -80, 20, .75, 0xff0000);
        draw_text("Player 2 down key : down arrow", -80, 10, .75, 0xff0000);
        draw_text("Press ESC to go back", -80, 0, .75, 0xff0000);
        if (pressed(BUTTON_ESC)) {
            current_gamemode = GM_MENU;
        }
    }
    else {
        if (pressed(BUTTON_UP)) {
            hot_button--;
            if (hot_button < 0) {
                hot_button = 3;
            }
        }
        else if (pressed(BUTTON_DOWN)) {
            hot_button++;
            if (hot_button > 3) {
                hot_button = 0;
            }
        }

        if (pressed(BUTTON_ENTER)) {
            if (hot_button == 0) {
                current_gamemode = GM_GAMEPLAY;
                enemy_is_ai = true;
            }
            else if (hot_button == 1) {
                current_gamemode = GM_GAMEPLAY;
                enemy_is_ai = false;
            }
            else if (hot_button == 2) {
                current_gamemode = GM_CONTROLS;
            }
            else if (hot_button == 3) {
                running = false;
            }
        }

        if (hot_button == 0) {
            draw_text("Single player", -80, 0, .75, 0xff0000);
            draw_text("Multiplayer", -80, -10, .75, 0xaaaaaa);
            draw_text("Controls", -80, -20, .75, 0xaaaaaa);
            draw_text("Exit", -80, -30, .75, 0xaaaaaa);
        }
        else if (hot_button == 1) {
            draw_text("Single player", -80, 0, .75, 0xaaaaaa);
            draw_text("Multiplayer", -80, -10, .75, 0xff0000);
            draw_text("Controls", -80, -20, .75, 0xaaaaaa);
            draw_text("Exit", -80, -30, .75, 0xaaaaaa);
        }
        else if (hot_button == 2) {
            draw_text("Single player", -80, 0, .75, 0xaaaaaa);
            draw_text("Multiplayer", -80, -10, .75, 0xaaaaaa);
            draw_text("Controls", -80, -20, .75, 0xff0000);
            draw_text("Exit", -80, -30, .75, 0xaaaaaa);
        }
        else if (hot_button == 3) {
            draw_text("Single player", -80, 0, .75, 0xaaaaaa);
            draw_text("Multiplayer", -80, -10, .75, 0xaaaaaa);
            draw_text("Controls", -80, -20, .75, 0xaaaaaa);
            draw_text("Exit", -80, -30, .75, 0xff0000);
        }

        draw_text("Pong Game", -73, 40, 2, 0xffffff);
        draw_text("Developed by FONXY", -73, 22, .75, 0xffffff);
    }
}