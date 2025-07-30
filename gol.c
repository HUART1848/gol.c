#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>

typedef uint8_t u8;
typedef uint32_t u32;

typedef struct {
    u8 *back;
    u8 *front;
    u32 w;
    u32 h;
} gol;

void gol_init(gol *g, u32 w, u32 h) {
    g->w = w;
    g->h = h;
    g->back = malloc(sizeof(u8) * w * h);
    g->front = malloc(sizeof(u8) * w * h);

    memset(g->back, 0, w * h);
    memset(g->front, 0, w * h);
}

void gol_free(gol *g) {
    free(g->back);
    free(g->front);
}

u8 gol_get(gol *g, u32 x, u32 y, u8 back) {
    x = x % g->w;
    y = y % g->h;

    if (back) {
	return g->back[y * g->w + x];
    }

    return g->front[y * g->w + x];
}

void gol_set(gol *g, u32 x, u32 y, u8 state, u8 back) {
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
	gol_set(g, i % g->w, i / g->w, 255 * (rand() % 2), 0);
    }
}

void gol_swap(gol *g) {
    u8 *tmp = g->back;
    g->back = g->front;
    g->front = tmp;
}

u32 gol_count(gol *g, u32 x, u32 y) {
    u32 count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
	for (int dy = -1; dy <= 1; ++dy) {
	    if (dx != 0 || dy != 0) {
		count += gol_get(g, x + dx, y + dy, 0) > 0;
	    }
	}
    }

    return count;
}

void gol_step(gol *g) {
    #pragma omp parallel for
    for (u32 i = 0; i < g->w; ++i) {
	for (u32 j = 0; j < g->h; ++j) {
	    u32 count = gol_count(g, i, j);
	    u8 alive = gol_get(g, i, j, 0);

	    if ((!alive && count == 3) || (alive && (count == 2 || count == 3))) {
		gol_set(g, i, j, 255, 1);
	    } else {
		gol_set(g, i, j, 0, 1);
	    }
	}
    }

    gol_swap(g);
}

void gol_display(gol *g) {
    for (u32 i = 0; i < g->w * g->h; ++i) {
	char c = gol_get(g, i % g->w, i / g->w, 0) ? '*' : ' ';
	printf("%c", c);

	if (i % g->w == g->w - 1) {
	    printf("\n");
	}
    }
}

void gol_pgm(gol *g, FILE *f) {
    fprintf(f, "P5\n%d %d\n255\n", g->w, g->h);
    fwrite(g->front, 1, g->w * g->h, f);
}

void gol_pgm_upscaled(gol *g, FILE *f, u8 *buf, u8 scale) {
    if (scale < 2) {
	printf("ERROR: scale should be > 1");
	exit(1);
    }

    fprintf(f, "P5\n%d %d\n255\n", g->w * scale, g->h * scale);

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
}

void gol_pgm_frames(gol *g, FILE *f, u32 n) {
    for (u32 i = 0; i < n; ++i) {
	gol_pgm(g, f);
	gol_step(g);
    }
}

void gol_pgm_frames_upscaled(gol *g, FILE *f, u32 n, u32 scale) {
    u8 *buf = malloc(sizeof(u8) * g->w * g->h * scale * scale);

    for (u32 i = 0; i < n; ++i) {
	gol_pgm_upscaled(g, f, buf, scale);
	gol_step(g);
    }
    
    free(buf);
}

void gol_ascii(gol *g, u32 n) {
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

void gol_save_frames(gol *g, char *dirname, u32 n) {
    char path[100];

    for (u32 i = 0; i <= n; ++i) {
	sprintf(path, "%s/%d.pgm", dirname, i);
	FILE *f = fopen(path, "w");
	if (f == NULL) {
	    printf("ERROR: Can't open %s\n", path);

	    fclose(f);
	    exit(1);
	}

	if (i != 0) {
	    gol_step(g);
	}

	printf("%s\n", path);
	gol_pgm(g, f);

	fclose(f);
    }
}

void gol_saves_frames_upscaled(gol *g, char *dirname, u32 n, u32 scale) {
    char path[100];

    u8 *buf = malloc(sizeof(u8) * g->w * g->h * scale * scale);
    for (u32 i = 0; i <= n; ++i) {
	sprintf(path, "%s/%d.pgm", dirname, i);
	FILE *f = fopen(path, "w");
	if (f == NULL) {
	    printf("ERROR: Can't open %s\n", path);
	    exit(1);
	}

	if (i != 0) {
	    gol_step(g);
	}

	printf("%s\n", path);
	gol_pgm_upscaled(g, f, buf, scale);

	fclose(f);
    }

    free(buf);
}

#ifndef LIBGOLONLY

#define DEFAULT_W 50
#define DEFAULT_H 50
#define DEFAULT_SCALE 10
#define DEFAULT_N 300

int shiftargs(int *argc, char ***argv) {
    if (*argc < 1) return 0;

    --(*argc);
    ++(*argv);
  
    return *argc;
}

void u32parse(u32 *into, char **argv, char* argname) {
    if (!strcmp(argv[0], argname)) {
	long tmp = strtol(argv[1], NULL, 10);
	if (tmp) *into = tmp;
    }
}

void showhelp() {
    printf("Usage: gol -w [WIDTH (%d)] -h [HEIGHT (%d)] -s [SCALING_FACTOR (%d)] -n [FRAMES (%d)] \n", DEFAULT_W, DEFAULT_H, DEFAULT_SCALE, DEFAULT_N);
}

int main(int argc, char **argv) {
    if (argc == 1) {
	showhelp();
	exit(0);
    }
    
    u32 w = DEFAULT_W;
    u32 h = DEFAULT_H;
    u32 scale = DEFAULT_SCALE;
    u32 n = DEFAULT_N;

    long tmp;
    while (shiftargs(&argc, &argv)) {
	u32parse(&w, argv, "-w");
	u32parse(&h, argv, "-h");
	u32parse(&scale, argv, "-s");
	u32parse(&n, argv, "-n");
    }

    gol g = {0};
    gol_init(&g, w, h);
    gol_random(&g);

    if (scale < 2) {
	gol_pgm_frames(&g, stdout, n);
    } else {
	gol_pgm_frames_upscaled(&g, stdout, n, scale);
    }
    
    gol_free(&g);
    return 0;
}
#endif
