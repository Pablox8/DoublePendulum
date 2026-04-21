#include "raylib.h"
#include "states.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

Rectangle make_rectangle(float x, float y, float w, float h) {
    return (Rectangle){ .x = x, .y = y, .width = w, .height = h };
}

bool is_number_within_bounds(float number, float min_value, float max_value) {
    return number >= min_value && number <= max_value;
}

bool update_angle_control(Rectangle slider, Rectangle valuebox, float min, float max, float *value, char valueBoxTextValue[32], bool *editmode) {
    bool changed = false;

    char max_text[16];
    char min_text[16];

    if (max != 2*PI) {
        sprintf(max_text, "%.0f", max);
        sprintf(min_text, "%.0f", min);
    }
    else {
        strcpy(max_text, "2*PI");
        strcpy(min_text, "0");
    }

    if (GuiSlider(slider, min_text, max_text, value, min, max)) {
        changed = true;
        sprintf(valueBoxTextValue, "%.3f", *value);
    }

    if (GuiValueBoxFloat(valuebox, NULL, valueBoxTextValue, value, *editmode)) {
        if (is_number_within_bounds(*value, min, max)) {
            changed = true;
            *editmode = !*editmode;
        }
    }
    
    return changed;
}

int main() {
    // Initialization
    const int screen_width = 1000;
    const int screen_height = 800;

    Vector2 text_position = { 5, 5 };
    
    Vector2 center = {
        .x = screen_width / 2.0,
        .y = screen_height / 2.0 - 50
    };

    Pendulum red = make_pendulum(50, 200);
    Pendulum blue = make_pendulum(50, 200);

    State initial = { 0, 0, 0, 0 };
    State current;

    char variables_text[100];
    sprintf(variables_text, "Red Angle: %.3lf\nRed Angular Velocity: %.3lf\nBlue Angle: %.3lf\nBlue Angular Velocity: %.3lf\n", 
        initial.theta1, initial.omega1, initial.theta2, initial.omega2);

    Vector2 red_center = {
        .x = center.x + red.L * sin(initial.theta1),
        .y = center.y + red.L * cos(initial.theta1)
    };

    Vector2 blue_center = {
        .x = red_center.x + blue.L * sin(initial.theta2),
        .y = red_center.y + blue.L * cos(initial.theta2)
    };

    double dt = 1.0 / 60.0;
    const int steps_per_frame = 5;

    double theta1_lim = 2*PI;
    double theta2_lim = 2*PI;

    InitWindow(screen_width, screen_height, "Double Pendulum");
    SetTargetFPS(60);

    Rectangle start_bounds = make_rectangle(screen_width - 150, 5, 100, 40);
    Rectangle reset_bounds = make_rectangle(screen_width - 150, 60, 100, 40);

    Rectangle red_angle_slider = make_rectangle(screen_width / 2.0 - 100, 5, 100, 20);
    Rectangle red_angular_velocity_slider = make_rectangle(screen_width / 2.0 - 100, 30, 100, 20); 
    Rectangle blue_angle_slider = make_rectangle(screen_width / 2.0 - 100, 55, 100, 20); 
    Rectangle blue_angular_velocity_slider = make_rectangle(screen_width / 2.0 - 100, 80, 100, 20); 

    Rectangle red_angle_valuebox = make_rectangle(screen_width / 2.0 + 100, 5, 100, 20); 
    Rectangle red_angular_velocity_valuebox = make_rectangle(screen_width / 2.0 + 100, 30, 100, 20);
    Rectangle blue_angle_valuebox = make_rectangle(screen_width / 2.0 + 100, 55, 100, 20);
    Rectangle blue_angular_velocity_valuebox = make_rectangle(screen_width / 2.0 + 100, 80, 100, 20);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    bool running = false;

    struct {
        float theta1;
        float theta2; 
        float omega1;
        float omega2;
    } gui_values;
    gui_values.theta1 = 0;
    gui_values.theta2 = 0; 
    gui_values.omega1 = 0;
    gui_values.omega2 = 0;
    
    char valueBoxTheta1[32] = { 0 };
    char valueBoxOmega1[32] = { 0 };
    char valueBoxTheta2[32] = { 0 };
    char valueBoxOmega2[32] = { 0 };

    bool red_angle_valuebox_editmode = true;
    bool red_angular_velocity_valuebox_editmode = true;
    bool blue_angle_valuebox_editmode = true;
    bool blue_angular_velocity_valuebox_editmode = true;

    RenderTexture2D trail_texture = LoadRenderTexture(screen_width, screen_height);
    BeginTextureMode(trail_texture);
        ClearBackground(BLANK);
    EndTextureMode();

    while (!WindowShouldClose()) {
        red_center.x = center.x + red.L * sin(initial.theta1);
        red_center.y = center.y + red.L * cos(initial.theta1);
        blue_center.x = red_center.x + blue.L * sin(initial.theta2);
        blue_center.y = red_center.y + blue.L * cos(initial.theta2);

        sprintf(variables_text, "Red Angle: %.3lf\nRed Angular Velocity: %.3lf\nBlue Angle: %.3lf\nBlue Angular Velocity: %.3lf\n", 
            initial.theta1, initial.omega1, initial.theta2, initial.omega2);
        
        // Update
        if (running) {
            for (int i = 0; i < steps_per_frame; i++) {
                State next = rk4(current, dt, red, blue);

                red_center.x = center.x + red.L * sin(next.theta1);
                red_center.y = center.y + red.L * cos(next.theta1);

                blue_center.x = red_center.x + blue.L * sin(next.theta2);
                blue_center.y = red_center.y + blue.L * cos(next.theta2);

                current = next;
            }

            if (current.theta1 > 0) theta1_lim = 2*PI;
            else theta1_lim = -2*PI;
            
            if (current.theta2 > 0) theta2_lim = 2*PI;
            else theta2_lim = -2*PI;
            
            sprintf(variables_text, "Red Angle: %.3lf\nRed Angular Velocity: %.3lf\nBlue Angle: %.3lf\nBlue Angular Velocity: %.3lf\n", 
                fmod(current.theta1, theta1_lim), current.omega1, fmod(current.theta2, theta2_lim), current.omega2);
            
            BeginTextureMode(trail_texture);
                DrawCircleV(red_center, 1, RED);
                DrawCircleV(blue_center, 1, BLUE);
            EndTextureMode();
        }

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // angle and velocity values and fps
            DrawTextEx(GetFontDefault(), variables_text, text_position, 20, 3, BLACK);
            DrawFPS(screen_width - 25, 5);

            // pendulums
            DrawLineV(center, red_center, BLACK);
            DrawLineV(red_center, blue_center, BLACK);
            DrawCircleV(red_center, 15, RED);
            DrawCircleV(blue_center, 15, BLUE);

            // trail texture
            DrawTextureRec(trail_texture.texture,
                (Rectangle){ 0, 0, screen_width, -screen_height },  
                (Vector2){ 0, 0 },
                WHITE);
            
            // gui
            if (running) GuiSetState(STATE_DISABLED);

            if (update_angle_control(red_angle_slider, red_angle_valuebox, 0, 2*PI, &gui_values.theta1, valueBoxTheta1, &red_angle_valuebox_editmode))
                initial.theta1 = gui_values.theta1;

            if (update_angle_control(red_angular_velocity_slider, red_angular_velocity_valuebox, -1, 1, &gui_values.omega1, valueBoxOmega1, &red_angular_velocity_valuebox_editmode))
                initial.omega1 = gui_values.omega1;

            if (update_angle_control(blue_angle_slider, blue_angle_valuebox, 0, 2*PI, &gui_values.theta2, valueBoxTheta2, &blue_angle_valuebox_editmode))
                initial.theta2 = gui_values.theta2;

            if (update_angle_control(blue_angular_velocity_slider, blue_angular_velocity_valuebox, -1, 1, &gui_values.omega2, valueBoxOmega2, &blue_angular_velocity_valuebox_editmode))
                initial.omega2 = gui_values.omega2;

            GuiSetState(STATE_NORMAL);
            
            if (GuiButton(start_bounds, "Start") && !running) {
                current = initial;

                running = true;
            }
            if (GuiButton(reset_bounds, "Reset")) {
                red_center.x = center.x + red.L * sin(initial.theta1);
                red_center.y = center.y + red.L * cos(initial.theta1);

                blue_center.x = red_center.x + blue.L * sin(initial.theta2);
                blue_center.y = red_center.y + blue.L * cos(initial.theta2);

                sprintf(variables_text, "Red Angle: %.3lf\nRed Angular Velocity: %.3lf\nBlue Angle: %.3lf\nBlue Angular Velocity: %.3lf\n", 
                    initial.theta1, initial.omega1, initial.theta2, initial.omega2);

                BeginTextureMode(trail_texture);
                    ClearBackground(BLANK);
                EndTextureMode();
                
                running = false;
            }

        EndDrawing();
    }

    // De-Initialization
    UnloadRenderTexture(trail_texture);
    CloseWindow();

    return 0;
}