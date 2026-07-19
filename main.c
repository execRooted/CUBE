#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define WIDTH 80
#define HEIGHT 40
#define CHAR_COUNT 12

static const char shade[CHAR_COUNT + 1] = ".,-~:;=!*#$@";
static char screen[HEIGHT][WIDTH];
static float zbuffer[HEIGHT][WIDTH];

static float cube[8][3] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
};

static int edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
};

static inline float dot(float ax, float ay, float az, float bx, float by, float bz) {
    return ax * bx + ay * by + az * bz;
}

static void render_frame(float A, float B) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            screen[y][x] = ' ';
            zbuffer[y][x] = 0.0f;
        }
    }

    float sinA = sinf(A), cosA = cosf(A);
    float sinB = sinf(B), cosB = cosf(B);

    for (int e = 0; e < 12; e++) {
        int i = edges[e][0], j = edges[e][1];
        float p[2][3];

        for (int k = 0; k < 2; k++) {
            float x = cube[(k == 0) ? i : j][0];
            float y = cube[(k == 0) ? i : j][1];
            float z = cube[(k == 0) ? i : j][2];

            float x1 = cosB * x + sinB * z;
            float z1 = -sinB * x + cosB * z;
            float y1 = cosA * y + sinA * z1;
            float z2 = -sinA * y + cosA * z1;

            float ooz = 1.0f / (z2 + 5.0f);
            p[k][0] = WIDTH / 2 + (WIDTH / 2) * ooz * x1 * 2.0f;
            p[k][1] = HEIGHT / 2 - (HEIGHT / 2) * ooz * y1 * 2.0f;
            p[k][2] = ooz;
        }

        float x0 = p[0][0], y0 = p[0][1], z0 = p[0][2];
        float x1 = p[1][0], y1 = p[1][1], z1 = p[1][2];

        float dx = x1 - x0, dy = y1 - y0;
        int steps = (int)(fabsf(dx) > fabsf(dy) ? fabsf(dx) : fabsf(dy)) + 1;
        float nx = -dy, ny = dx;
        float len = sqrtf(nx * nx + ny * ny) + 1e-6f;
        nx /= len; ny /= len;
        float lum = fabsf(dot(nx, ny, 0.0f, 0.0f, 0.0f, 1.0f));
        int idx = (int)(lum * (CHAR_COUNT - 1));
        if (idx < 0) idx = 0;
        if (idx > CHAR_COUNT - 1) idx = CHAR_COUNT - 1;
        char c = shade[idx];

        for (int s = 0; s <= steps; s++) {
            float t = (float)s / steps;
            int px = (int)(x0 + dx * t);
            int py = (int)(y0 + dy * t);
            float pz = z0 + (z1 - z0) * t;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                if (pz > zbuffer[py][px]) {
                    zbuffer[py][px] = pz;
                    screen[py][px] = c;
                }
            }
        }
    }
}

int main(void) {
    printf("\x1b[2J");
    float A = 0.0f, B = 0.0f;
    while (1) {
        render_frame(A, B);
        printf("\x1b[H");
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                putchar(screen[y][x]);
            }
            putchar('\n');
        }
        fflush(stdout);
        A += 0.04f;
        B += 0.02f;
        usleep(33000);
    }
    return 0;
}
