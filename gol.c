#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

typedef uint32_t u32;

typedef struct {
    bool *back;
    bool *front;
    u32 w;
    u32 h;
} gol;

void gol_init(gol *g, u32 w, u32 h) {
    g->w = w;
    g->h = h;
    g->back = malloc(sizeof(bool) * w * h);
    g->front = malloc(sizeof(bool) * w * h);

    memset(g->back, false, w * h);
    memset(g->front, false, w * h);
}

void gol_free(gol *g) {
    free(g->back);
    free(g->front);
}

bool gol_get(gol *g, u32 x, u32 y, bool back) {
    x = x % g->w;
    y = y % g->h;

    if (back) {
        return g->back[y * g->w + x];
    }

    return g->front[y * g->w + x];
}

void gol_set(gol *g, u32 x, u32 y, bool state, bool back) {
    x = x % g->w;
    y = y % g->h;

    if (back) {
        g->back[y * g->w + x] = state;
        return;
    }

    g->front[y * g->w + x] = state;
}

void gol_random(gol *g) {
    for (u32 i = 0; i < g->w * g->h; ++i) {
        gol_set(g, i % g->w, i / g->w, rand() % 2 == 0, false);
    }
}

void gol_swap(gol *g) {
    bool *tmp = g->back;
    g->back = g->front;
    g->front = tmp;
}

u32 gol_count(gol *g, u32 x, u32 y) {
    u32 count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                count += gol_get(g, x + dx, y + dy, false);
            }
        }
    }

    return count;
}

void gol_step(gol *g) {
    for (u32 i = 0; i < g->w; ++i) {
        for (u32 j = 0; j < g->h; ++j) {
            u32 count = gol_count(g, i, j);
            bool alive = gol_get(g, i, j, false);

            if ((!alive && count == 3) || (alive && (count == 2 || count == 3))) {
                gol_set(g, i, j, true, true);
            } else {
                gol_set(g, i, j, false, true);
            }
        }
    }

    gol_swap(g);
}

void gol_display(gol *g) {
    for (u32 i = 0; i < g->w * g->h; ++i) {
        char c = gol_get(g, i % g->w, i / g->w, false) ? '*' : ' ';
        printf("%c", c);

        if (i % g->w == g->w - 1) {
            printf("\n");
        }
    }
}

void gol_to_ppm(gol *g, char *path) {
    FILE *f;

    f = fopen(path, "w");
    if (f == NULL) {
        printf("Can't write file %s\n", path);
        gol_free(g);
        exit(1);
    }

    printf("%s\n", path);
    fprintf(f, "P5\n%d %d\n1\n", g->w, g->h);
    fwrite(g->front, 1, g->w * g->h, f);

    fclose(f);
}

void gol_to_ppm_upscaled(gol *g, bool *buf, char *path, u32 scale) {
    FILE *f;

    f = fopen(path, "w");
    if (f == NULL) {
        printf("Can't write file %s\n", path);
        fclose(f);

        gol_free(g);
        exit(1);
    }

    printf("%s\n", path);

    if (scale < 2) {
        printf("ERROR: scale should be > 1");

        fclose(f);
        free(buf);
        gol_free(g);

        exit(1);
    }

    fprintf(f, "P5\n%d %d\n1\n", g->w * scale, g->h * scale);

    for (u32 i = 0; i < g->w * g->h; ++i) {
        for (u32 dx = 0; dx < scale; ++dx) {
            for (u32 dy = 0; dy < scale; ++dy) {
                u32 x = (i % g->w) * scale + dx;
                u32 y = (i / g->w) * scale + dy;

                buf[y * g->w * scale + x] = g->front[i];
            }
        }
    }

    fwrite(buf, 1, g->w * g->h * scale * scale, f);
    fclose(f);
}

void gol_console(gol *g, u32 n) {
    for (u32 i = 0; i <= n; ++i) {
        printf("Step %d\n", i);
        if (i == 0) {
            printf("Start\n");
            gol_display(g);
        }
        gol_step(g);
        gol_display(g);
    }
}

void gol_frames(gol *g, char *dirname, u32 n) {
    char path[100];

    for (u32 i = 0; i <= n; ++i) {
        sprintf(path, "%s/%d.pgm", dirname, i);
        if (i != 0) {
            gol_step(g);
        }
        gol_to_ppm(g, path);
    }
}

void gol_frames_upscaled(gol *g, char *dirname, u32 n, u32 scale) {
    char path[100];

    bool *buf = malloc(sizeof(bool) * g->w * g->h * scale * scale);
    for (u32 i = 0; i <= n; ++i) {
        sprintf(path, "%s/%d.pgm", dirname, i);
        if (i != 0) {
            gol_step(g);
        }
        gol_to_ppm_upscaled(g, buf, path, scale);
    }

    free(buf);
}

int main(void) {
    gol g = {0};
    gol_init(&g, 16, 16);

    srand(time(NULL));
    gol_random(&g);

    gol_frames_upscaled(&g, "out", 10000, 32);

    gol_free(&g);
    return 0;
}
