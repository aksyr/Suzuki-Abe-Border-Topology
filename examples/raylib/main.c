#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#include "sa_border_topology/sa_border_topology.h"

#define CLAMP(x,  min, max) (fminf(max, fmaxf(x, min)))

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "Suzuki Abe Border Topology");

    // load image
    Image image = LoadImage("suzuki_abe_test.png");
    Texture texture = LoadTextureFromImage(image);

    // extract topology from the image
    int* topology_image = (int*)calloc((image.width+2) * (image.height+2), sizeof(int));
    for (int y=0; y<image.height; ++y) {
        for (int x=0; x<image.width; ++x) {
            topology_image[(y+1) * (image.width+2) + (x+1)] = GetImageColor(image, x, y).a == 0 ? 0 : 1;
        }
    }
    SA_BorderTopology border_topology;
    SA_allocate_topology(&border_topology, 10, 1000);
    int result = SA_extract_topology(topology_image, image.width+2, image.height+2, &border_topology);

    // print topology extraction results
    TraceLog(LOG_INFO, "Topology analysis %s", result ? "FAILURE" : "SUCCESS");
    TraceLog(LOG_INFO, "- Background 1");
    for (size_t i=1; i<border_topology.borders_count; ++i) {
        TraceLog(LOG_INFO, "- %s %d with parent %d and %d points",
            border_topology.borders[i].is_hole ? "Hole " : "Outer",
            i+1,
            border_topology.borders[i].parent+1,
            border_topology.borders[i].points_count);
    }

    // raylib camera
    Camera2D camera = {
        .offset = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f},
        .target =  {image.width * 0.5f, image.height * 0.5f},
        .rotation = 0.0f,
        .zoom = 50.0f,
    };

    while (!WindowShouldClose()) {

        // input
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 const mouse_delta = GetMouseDelta();
                Vector2 const from = GetScreenToWorld2D((Vector2){0.0f, 0.0f}, camera);
                Vector2 const to = GetScreenToWorld2D(mouse_delta, camera);
                Vector2 const target_delta =  Vector2Subtract(to, from);
                camera.target = Vector2Subtract(camera.target, target_delta);
            }

            float const mouse_wheel = GetMouseWheelMove();
            if (mouse_wheel != 0.0f) {
                Vector2 const mouse_position = GetMousePosition();
                Vector2 const old_pos = GetScreenToWorld2D(mouse_position, camera);
                camera.zoom = CLAMP(camera.zoom * expf(mouse_wheel * 0.01f), 0.1f, 200.0f);
                Vector2 const new_pos = GetScreenToWorld2D(mouse_position, camera);
                Vector2 const target_delta = Vector2Subtract(new_pos, old_pos);
                camera.target = Vector2Subtract(camera.target, target_delta);
            }
        }

        // draw
        {
            float const line_thickness = 1.0f / camera.zoom;

            BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode2D(camera);

            DrawRectangleV((Vector2){0.0f, 0.0f}, (Vector2){texture.width, texture.height}, DARKGRAY);
            DrawTextureEx(texture, (Vector2){0.0f, 0.0f}, 0.0f, 1.0f, WHITE);

            // draw borders
            for (size_t border_idx=0; border_idx<border_topology.borders_count; ++border_idx) {
                SA_Border const border = border_topology.borders[border_idx];

                for (size_t point_idx = 0; point_idx<border.points_count; ++point_idx) {
                    size_t const next_point_idx = (point_idx+1) % border.points_count;
                    SA_Point const point = border_topology.points[border.first_point + point_idx];
                    SA_Point const next_point = border_topology.points[border.first_point + next_point_idx];

                    Vector2 const start = {(point.x-1) + 0.5f, (point.y-1) + 0.5f};
                    Vector2 const end = {(next_point.x-1) + 0.5f, (next_point.y-1) + 0.5f};

                    DrawLineEx(start, end, line_thickness, border.is_hole ? MAGENTA : YELLOW);
                }
            }

            // draw values
            Font font = GetFontDefault();
            char text_buffer[10];
            for (int y = 0; y < image.height; ++y) {
                for (int x = 0; x < image.width; ++x) {
                    int const value = topology_image[(y+1)*(image.width+2) + (x+1)];
                    if (value == 0 || value == 1) continue;

                    snprintf(text_buffer, 10, "%d", value);
                    Vector2 const size = MeasureTextEx(font, text_buffer, (float)font.baseSize, 1.0f);
                    float const scale = fminf(1.0f/size.x, 1.0f/size.y) * 0.95f;
                    Vector2 const position = {
                        (float)x + 0.5f - size.x*0.5f*scale,
                        (float)y + 0.5f - size.y*0.5f*scale
                    };

                    DrawTextEx(font, text_buffer, position, (float)font.baseSize * scale, scale, RED);
                }
            }

            EndMode2D();
            EndDrawing();
        }
    }

    // free
    free(topology_image);
    SA_free_topology(&border_topology);


    CloseWindow();

    return 0;
}
