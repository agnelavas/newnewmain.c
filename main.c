#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_WIDTH 80
#define MAX_HEIGHT 40
#define DEFAULT_WIDTH 80
#define DEFAULT_HEIGHT 24
#define MAX_SHAPES 100
// ANSI Escape Codes for CLI styling
#define ANSI_RESET    "\033[0m"
#define ANSI_BOLD     "\033[1m"
#define ANSI_RED      "\033[31m"
#define ANSI_GREEN    "\033[32m"
#define ANSI_YELLOW   "\033[33m"
#define ANSI_BLUE     "\033[34m"
#define ANSI_MAGENTA  "\033[35m"
#define ANSI_CYAN     "\033[36m"
#define ANSI_BG_DARK  "\033[40m"
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;
typedef struct {
    int x1, y1;
    int x2, y2;
} LineParams;
typedef struct {
    int x1, y1; // Top-left
    int x2, y2; // Bottom-right
    int filled; // 1 = filled, 0 = outline
} RectParams;
typedef struct {
    int cx, cy; // Center
    int r;      // Radius
} CircleParams;
typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriParams;
typedef struct {
    int id;
    ShapeType type;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriParams triangle;
    } params;
    char draw_char;
} Shape;
// Global Editor State
int canvas_width = DEFAULT_WIDTH;
int canvas_height = DEFAULT_HEIGHT;
char canvas[MAX_HEIGHT][MAX_WIDTH];
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_shape_id = 1;
// Drawing & Canvas Functions
void init_canvas(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            canvas[y][x] = '_';
        }
    }
}
void draw_line(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h, int x1, int y1, int x2, int y2, char c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
            canvas[y1][x1] = c;
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
void draw_rect(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h, int x1, int y1, int x2, int y2, int filled, char c) {
    int start_x = (x1 < x2) ? x1 : x2;
    int end_x = (x1 < x2) ? x2 : x1;
    int start_y = (y1 < y2) ? y1 : y2;
    int end_y = (y1 < y2) ? y2 : y1;
    if (filled) {
        for (int y = start_y; y <= end_y; y++) {
            for (int x = start_x; x <= end_x; x++) {
                if (x >= 0 && x < w && y >= 0 && y < h) {
                    canvas[y][x] = c;
                }
            }
        }
    } else {
        // Draw horizontal edges
        for (int x = start_x; x <= end_x; x++) {
            if (x >= 0 && x < w) {
                if (start_y >= 0 && start_y < h) canvas[start_y][x] = c;
                if (end_y >= 0 && end_y < h) canvas[end_y][x] = c;
            }
        }
        // Draw vertical edges
        for (int y = start_y; y <= end_y; y++) {
            if (y >= 0 && y < h) {
                if (start_x >= 0 && start_x < w) canvas[y][start_x] = c;
                if (end_x >= 0 && end_x < w) canvas[y][end_x] = c;
            }
        }
    }
}
void plot_circle_points(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h, int xc, int yc, int x, int y, char c) {
    int px[] = {xc + x, xc - x, xc + x, xc - x, xc + y, xc - y, xc + y, xc - y};
    int py[] = {yc + y, yc + y, yc - y, yc - y, yc + x, yc + x, yc - x, yc - x};
    for (int i = 0; i < 8; i++) {
        if (px[i] >= 0 && px[i] < w && py[i] >= 0 && py[i] < h) {
            canvas[py[i]][px[i]] = c;
        }
    }
}
void draw_circle(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h, int xc, int yc, int r, char c) {
    if (r < 0) return;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    plot_circle_points(canvas, w, h, xc, yc, x, y, c);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_circle_points(canvas, w, h, xc, yc, x, y, c);
    }
}
void draw_triangle(char canvas[MAX_HEIGHT][MAX_WIDTH], int w, int h, int x1, int y1, int x2, int y2, int x3, int y3, char c) {
    draw_line(canvas, w, h, x1, y1, x2, y2, c);
    draw_line(canvas, w, h, x2, y2, x3, y3, c);
    draw_line(canvas, w, h, x3, y3, x1, y1, c);
}
void render_shapes() {
    init_canvas(canvas, canvas_width, canvas_height);
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                draw_line(canvas, canvas_width, canvas_height, s.params.line.x1, s.params.line.y1, s.params.line.x2, s.params.line.y2, s.draw_char);
                break;
            case SHAPE_RECTANGLE:
                draw_rect(canvas, canvas_width, canvas_height, s.params.rect.x1, s.params.rect.y1, s.params.rect.x2, s.params.rect.y2, s.params.rect.filled, s.draw_char);
                break;
            case SHAPE_CIRCLE:
                draw_circle(canvas, canvas_width, canvas_height, s.params.circle.cx, s.params.circle.cy, s.params.circle.r, s.draw_char);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(canvas, canvas_width, canvas_height, s.params.triangle.x1, s.params.triangle.y1, s.params.triangle.x2, s.params.triangle.y2, s.params.triangle.x3, s.params.triangle.y3, s.draw_char);
                break;
        }
    }
}
void display_canvas() {
    render_shapes();
    printf("\n");
    
    // Column Index Header (Tens place)
    printf("     ");
    for (int x = 0; x < canvas_width; x++) {
        if (x % 10 == 0) {
            printf("%s%d%s", ANSI_CYAN, x / 10, ANSI_RESET);
        } else {
            printf(" ");
        }
    }
    printf("\n");
    // Column Index Header (Ones place)
    printf("     ");
    for (int x = 0; x < canvas_width; x++) {
        printf("%s%d%s", ANSI_CYAN, x % 10, ANSI_RESET);
    }
    printf("\n");
    // Top border
    printf("    %s+", ANSI_BLUE);
    for (int x = 0; x < canvas_width; x++) {
        printf("-");
    }
    printf("+%s\n", ANSI_RESET);
    // Grid rows
    for (int y = 0; y < canvas_height; y++) {
        printf("%s%3d%s %s|%s", ANSI_CYAN, y, ANSI_RESET, ANSI_BLUE, ANSI_RESET);
        for (int x = 0; x < canvas_width; x++) {
            char cell = canvas[y][x];
            if (cell == '_') {
                printf("_");
            } else {
                printf("%s%c%s", ANSI_YELLOW, cell, ANSI_RESET);
            }
        }
        printf("%s|%s\n", ANSI_BLUE, ANSI_RESET);
    }
    // Bottom border
    printf("    %s+", ANSI_BLUE);
    for (int x = 0; x < canvas_width; x++) {
        printf("-");
    }
    printf("+%s\n", ANSI_RESET);
}
// Input Helpers
int get_valid_int(const char* prompt, int min_val, int max_val) {
    while (1) {
        printf("%s (%d to %d): ", prompt, min_val, max_val);
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) > 0) {
                char *endptr;
                long val = strtol(buffer, &endptr, 10);
                if (*endptr == '\0' && val >= min_val && val <= max_val) {
                    return (int)val;
                }
            }
        }
        printf("%sInvalid input. Enter an integer between %d and %d.%s\n", ANSI_RED, min_val, max_val, ANSI_RESET);
    }
}
int get_valid_int_default(const char* prompt, int min_val, int max_val, int default_val) {
    while (1) {
        printf("%s (%d to %d) [%d]: ", prompt, min_val, max_val, default_val);
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), stdin)) {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strlen(buffer) == 0) {
                return default_val;
            }
            char *endptr;
            long val = strtol(buffer, &endptr, 10);
            if (*endptr == '\0' && val >= min_val && val <= max_val) {
                return (int)val;
            }
        }
        printf("%sInvalid input. Enter an integer between %d and %d.%s\n", ANSI_RED, min_val, max_val, ANSI_RESET);
    }
}
char get_valid_char_default(const char* prompt, char default_char) {
    printf("%s [%c]: ", prompt, default_char);
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) > 0) {
            return buffer[0];
        }
    }
    return default_char;
}
// State Operations
void list_shapes() {
    if (shape_count == 0) {
        printf("\n%sNo shapes currently exist.%s\n", ANSI_YELLOW, ANSI_RESET);
        return;
    }
    printf("\n%s--- Active Shapes ---%s\n", ANSI_BOLD, ANSI_RESET);
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        printf("[%d] ", s.id);
        switch (s.type) {
            case SHAPE_LINE:
                printf("%sLINE%s: (%d, %d) to (%d, %d) char '%c'\n", ANSI_GREEN, ANSI_RESET, s.params.line.x1, s.params.line.y1, s.params.line.x2, s.params.line.y2, s.draw_char);
                break;
            case SHAPE_RECTANGLE:
                printf("%sRECTANGLE%s: Top-left (%d, %d) to Bottom-right (%d, %d), %s, char '%c'\n", 
                       ANSI_GREEN, ANSI_RESET, s.params.rect.x1, s.params.rect.y1, s.params.rect.x2, s.params.rect.y2, 
                       s.params.rect.filled ? "filled" : "outline", s.draw_char);
                break;
            case SHAPE_CIRCLE:
                printf("%sCIRCLE%s: Center (%d, %d), Radius %d, char '%c'\n", ANSI_GREEN, ANSI_RESET, s.params.circle.cx, s.params.circle.cy, s.params.circle.r, s.draw_char);
                break;
            case SHAPE_TRIANGLE:
                printf("%sTRIANGLE%s: Vertices (%d, %d), (%d, %d), (%d, %d), char '%c'\n", 
                       ANSI_GREEN, ANSI_RESET, s.params.triangle.x1, s.params.triangle.y1, s.params.triangle.x2, s.params.triangle.y2, s.params.triangle.x3, s.params.triangle.y3, s.draw_char);
                break;
        }
    }
}
void add_shape() {
    if (shape_count >= MAX_SHAPES) {
        printf("\n%sError: Maximum shape limit (%d) reached. Please delete some first.%s\n", ANSI_RED, MAX_SHAPES, ANSI_RESET);
        return;
    }
    printf("\n%s--- Add New Shape ---%s\n", ANSI_BOLD, ANSI_RESET);
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. Cancel\n");
    int choice = get_valid_int("Choose shape type", 1, 5);
    if (choice == 5) return;
    Shape s;
    s.id = next_shape_id++;
    s.draw_char = get_valid_char_default("Enter drawing character", '*');
    switch (choice) {
        case 1:
            s.type = SHAPE_LINE;
            s.params.line.x1 = get_valid_int("Start X1", 0, canvas_width - 1);
            s.params.line.y1 = get_valid_int("Start Y1", 0, canvas_height - 1);
            s.params.line.x2 = get_valid_int("End X2", 0, canvas_width - 1);
            s.params.line.y2 = get_valid_int("End Y2", 0, canvas_height - 1);
            break;
        case 2:
            s.type = SHAPE_RECTANGLE;
            s.params.rect.x1 = get_valid_int("Top-Left X1", 0, canvas_width - 1);
            s.params.rect.y1 = get_valid_int("Top-Left Y1", 0, canvas_height - 1);
            s.params.rect.x2 = get_valid_int("Bottom-Right X2", 0, canvas_width - 1);
            s.params.rect.y2 = get_valid_int("Bottom-Right Y2", 0, canvas_height - 1);
            s.params.rect.filled = get_valid_int("Filled? (0 = Outline, 1 = Filled)", 0, 1);
            break;
        case 3:
            s.type = SHAPE_CIRCLE;
            s.params.circle.cx = get_valid_int("Center X", 0, canvas_width - 1);
            s.params.circle.cy = get_valid_int("Center Y", 0, canvas_height - 1);
            // Cap radius logically
            s.params.circle.r = get_valid_int("Radius R", 0, canvas_width);
            break;
        case 4:
            s.type = SHAPE_TRIANGLE;
            s.params.triangle.x1 = get_valid_int("Vertex 1 X1", 0, canvas_width - 1);
            s.params.triangle.y1 = get_valid_int("Vertex 1 Y1", 0, canvas_height - 1);
            s.params.triangle.x2 = get_valid_int("Vertex 2 X2", 0, canvas_width - 1);
            s.params.triangle.y2 = get_valid_int("Vertex 2 Y2", 0, canvas_height - 1);
            s.params.triangle.x3 = get_valid_int("Vertex 3 X3", 0, canvas_width - 1);
            s.params.triangle.y3 = get_valid_int("Vertex 3 Y3", 0, canvas_height - 1);
            break;
    }
    shapes[shape_count++] = s;
    printf("\n%sShape [%d] successfully added!%s\n", ANSI_GREEN, s.id, ANSI_RESET);
}
void modify_shape() {
    if (shape_count == 0) {
        printf("\n%sNo shapes to modify.%s\n", ANSI_YELLOW, ANSI_RESET);
        return;
    }
    
    list_shapes();
    int target_id = get_valid_int("\nEnter the ID of the shape to modify", 1, next_shape_id - 1);
    
    int found_index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == target_id) {
            found_index = i;
            break;
        }
    }
    
    if (found_index == -1) {
        printf("\n%sShape with ID %d not found.%s\n", ANSI_RED, target_id, ANSI_RESET);
        return;
    }
    Shape *s = &shapes[found_index];
    printf("\n%s--- Modify Shape [%d] ---%s\n", ANSI_BOLD, s->id, ANSI_RESET);
    s->draw_char = get_valid_char_default("Drawing character", s->draw_char);
    switch (s->type) {
        case SHAPE_LINE:
            s->params.line.x1 = get_valid_int_default("Start X1", 0, canvas_width - 1, s->params.line.x1);
            s->params.line.y1 = get_valid_int_default("Start Y1", 0, canvas_height - 1, s->params.line.y1);
            s->params.line.x2 = get_valid_int_default("End X2", 0, canvas_width - 1, s->params.line.x2);
            s->params.line.y2 = get_valid_int_default("End Y2", 0, canvas_height - 1, s->params.line.y2);
            break;
        case SHAPE_RECTANGLE:
            s->params.rect.x1 = get_valid_int_default("Top-Left X1", 0, canvas_width - 1, s->params.rect.x1);
            s->params.rect.y1 = get_valid_int_default("Top-Left Y1", 0, canvas_height - 1, s->params.rect.y1);
            s->params.rect.x2 = get_valid_int_default("Bottom-Right X2", 0, canvas_width - 1, s->params.rect.x2);
            s->params.rect.y2 = get_valid_int_default("Bottom-Right Y2", 0, canvas_height - 1, s->params.rect.y2);
            s->params.rect.filled = get_valid_int_default("Filled? (0=Outline, 1=Filled)", 0, 1, s->params.rect.filled);
            break;
        case SHAPE_CIRCLE:
            s->params.circle.cx = get_valid_int_default("Center X", 0, canvas_width - 1, s->params.circle.cx);
            s->params.circle.cy = get_valid_int_default("Center Y", 0, canvas_height - 1, s->params.circle.cy);
            s->params.circle.r = get_valid_int_default("Radius R", 0, canvas_width, s->params.circle.r);
            break;
        case SHAPE_TRIANGLE:
            s->params.triangle.x1 = get_valid_int_default("Vertex 1 X1", 0, canvas_width - 1, s->params.triangle.x1);
            s->params.triangle.y1 = get_valid_int_default("Vertex 1 Y1", 0, canvas_height - 1, s->params.triangle.y1);
            s->params.triangle.x2 = get_valid_int_default("Vertex 2 X2", 0, canvas_width - 1, s->params.triangle.x2);
            s->params.triangle.y2 = get_valid_int_default("Vertex 2 Y2", 0, canvas_height - 1, s->params.triangle.y2);
            s->params.triangle.x3 = get_valid_int_default("Vertex 3 X3", 0, canvas_width - 1, s->params.triangle.x3);
            s->params.triangle.y3 = get_valid_int_default("Vertex 3 Y3", 0, canvas_height - 1, s->params.triangle.y3);
            break;
    }
    printf("\n%sShape [%d] modified successfully!%s\n", ANSI_GREEN, s->id, ANSI_RESET);
}
void delete_shape() {
    if (shape_count == 0) {
        printf("\n%sNo shapes to delete.%s\n", ANSI_YELLOW, ANSI_RESET);
        return;
    }
    
    list_shapes();
    int target_id = get_valid_int("\nEnter the ID of the shape to delete", 1, next_shape_id - 1);
    
    int found_index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == target_id) {
            found_index = i;
            break;
        }
    }
    
    if (found_index != -1) {
        // Shift active shapes down
        for (int i = found_index; i < shape_count - 1; i++) {
            shapes[i] = shapes[i + 1];
        }
        shape_count--;
        printf("\n%sShape [%d] successfully deleted!%s\n", ANSI_GREEN, target_id, ANSI_RESET);
    } else {
        printf("\n%sShape with ID %d not found.%s\n", ANSI_RED, target_id, ANSI_RESET);
    }
}
void clear_canvas() {
    shape_count = 0;
    next_shape_id = 1;
    printf("\n%sAll shapes deleted! Canvas cleared.%s\n", ANSI_GREEN, ANSI_RESET);
}
void change_canvas_dimensions() {
    printf("\n%s--- Resize Canvas ---%s\n", ANSI_BOLD, ANSI_RESET);
    int new_w = get_valid_int("Enter new width", 10, MAX_WIDTH);
    int new_h = get_valid_int("Enter new height", 5, MAX_HEIGHT);
    canvas_width = new_w;
    canvas_height = new_h;
    printf("\n%sCanvas resized to %d x %d!%s\n", ANSI_GREEN, canvas_width, canvas_height, ANSI_RESET);
}
int save_shapes(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "%d %d %d\n", canvas_width, canvas_height, shape_count);
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                fprintf(f, "LINE %c %d %d %d %d\n", s.draw_char, s.params.line.x1, s.params.line.y1, s.params.line.x2, s.params.line.y2);
                break;
            case SHAPE_RECTANGLE:
                fprintf(f, "RECT %c %d %d %d %d %d\n", s.draw_char, s.params.rect.x1, s.params.rect.y1, s.params.rect.x2, s.params.rect.y2, s.params.rect.filled);
                break;
            case SHAPE_CIRCLE:
                fprintf(f, "CIRC %c %d %d %d\n", s.draw_char, s.params.circle.cx, s.params.circle.cy, s.params.circle.r);
                break;
            case SHAPE_TRIANGLE:
                fprintf(f, "TRI %c %d %d %d %d %d %d\n", s.draw_char, s.params.triangle.x1, s.params.triangle.y1, s.params.triangle.x2, s.params.triangle.y2, s.params.triangle.x3, s.params.triangle.y3);
                break;
        }
    }
    fclose(f);
    return 1;
}
int load_shapes(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    int w, h, count;
    if (fscanf(f, "%d %d %d\n", &w, &h, &count) != 3) {
        fclose(f);
        return 0;
    }
    if (w >= 10 && w <= MAX_WIDTH && h >= 5 && h <= MAX_HEIGHT) {
        canvas_width = w;
        canvas_height = h;
    }
    shape_count = 0;
    next_shape_id = 1;
    char line[256];
    while (fgets(line, sizeof(line), f) && shape_count < MAX_SHAPES) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        char draw_char;
        Shape s;
        s.id = next_shape_id++;
        if (strncmp(line, "LINE ", 5) == 0) {
            s.type = SHAPE_LINE;
            draw_char = line[5];
            if (sscanf(line + 7, "%d %d %d %d", &s.params.line.x1, &s.params.line.y1, &s.params.line.x2, &s.params.line.y2) == 4) {
                s.draw_char = draw_char;
                shapes[shape_count++] = s;
            }
        } else if (strncmp(line, "RECT ", 5) == 0) {
            s.type = SHAPE_RECTANGLE;
            draw_char = line[5];
            if (sscanf(line + 7, "%d %d %d %d %d", &s.params.rect.x1, &s.params.rect.y1, &s.params.rect.x2, &s.params.rect.y2, &s.params.rect.filled) == 5) {
                s.draw_char = draw_char;
                shapes[shape_count++] = s;
            }
        } else if (strncmp(line, "CIRC ", 5) == 0) {
            s.type = SHAPE_CIRCLE;
            draw_char = line[5];
            if (sscanf(line + 7, "%d %d %d", &s.params.circle.cx, &s.params.circle.cy, &s.params.circle.r) == 3) {
                s.draw_char = draw_char;
                shapes[shape_count++] = s;
            }
        } else if (strncmp(line, "TRI ", 4) == 0) {
            s.type = SHAPE_TRIANGLE;
            draw_char = line[4];
            if (sscanf(line + 6, "%d %d %d %d %d %d", &s.params.triangle.x1, &s.params.triangle.y1, &s.params.triangle.x2, &s.params.triangle.y2, &s.params.triangle.x3, &s.params.triangle.y3) == 6) {
                s.draw_char = draw_char;
                shapes[shape_count++] = s;
            }
        }
    }
    fclose(f);
    return 1;
}
int export_canvas(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    render_shapes();
    for (int y = 0; y < canvas_height; y++) {
        for (int x = 0; x < canvas_width; x++) {
            fputc(canvas[y][x], f);
        }
        fputc('\n', f);
    }
    fclose(f);
    return 1;
}
void save_shapes_to_file() {
    if (shape_count == 0) {
        printf("\n%sNo shapes to save.%s\n", ANSI_YELLOW, ANSI_RESET);
        return;
    }
    char filename[128];
    printf("\nEnter filename to save (e.g. drawing.txt): ");
    if (!fgets(filename, sizeof(filename), stdin)) return;
    filename[strcspn(filename, "\n")] = 0;
    if (strlen(filename) == 0) return;
    if (save_shapes(filename)) {
        printf("\n%sSuccessfully saved %d shapes to %s!%s\n", ANSI_GREEN, shape_count, filename, ANSI_RESET);
    } else {
        printf("\n%sError: Could not open file %s for writing.%s\n", ANSI_RED, filename, ANSI_RESET);
    }
}
void load_shapes_from_file() {
    char filename[128];
    printf("\nEnter filename to load (e.g. drawing.txt): ");
    if (!fgets(filename, sizeof(filename), stdin)) return;
    filename[strcspn(filename, "\n")] = 0;
    if (strlen(filename) == 0) return;
    if (load_shapes(filename)) {
        printf("\n%sSuccessfully loaded %d shapes from %s!%s\n", ANSI_GREEN, shape_count, filename, ANSI_RESET);
    } else {
        printf("\n%sError: Could not open file %s for reading.%s\n", ANSI_RED, filename, ANSI_RESET);
    }
}
void export_canvas_to_file() {
    char filename[128];
    printf("\nEnter filename to export ASCII art (e.g. art.txt): ");
    if (!fgets(filename, sizeof(filename), stdin)) return;
    filename[strcspn(filename, "\n")] = 0;
    if (strlen(filename) == 0) return;
    if (export_canvas(filename)) {
        printf("\n%sSuccessfully exported ASCII art to %s!%s\n", ANSI_GREEN, filename, ANSI_RESET);
    } else {
        printf("\n%sError: Could not open file %s for writing.%s\n", ANSI_RED, filename, ANSI_RESET);
    }
}
void load_demo() {
    shape_count = 0;
    next_shape_id = 1;
    // Line
    Shape line;
    line.id = next_shape_id++;
    line.type = SHAPE_LINE;
    line.params.line.x1 = 5;
    line.params.line.y1 = 20;
    line.params.line.x2 = 45;
    line.params.line.y2 = 20;
    line.draw_char = '*';
    shapes[shape_count++] = line;
    // Rectangle (Outline)
    Shape rect1;
    rect1.id = next_shape_id++;
    rect1.type = SHAPE_RECTANGLE;
    rect1.params.rect.x1 = 5;
    rect1.params.rect.y1 = 2;
    rect1.params.rect.x2 = 25;
    rect1.params.rect.y2 = 8;
    rect1.params.rect.filled = 0;
    rect1.draw_char = '*';
    shapes[shape_count++] = rect1;
    // Rectangle (Filled)
    Shape rect2;
    rect2.id = next_shape_id++;
    rect2.type = SHAPE_RECTANGLE;
    rect2.params.rect.x1 = 55;
    rect2.params.rect.y1 = 2;
    rect2.params.rect.x2 = 75;
    rect2.params.rect.y2 = 8;
    rect2.params.rect.filled = 1;
    rect2.draw_char = '#';
    shapes[shape_count++] = rect2;
    // Circle
    Shape circle;
    circle.id = next_shape_id++;
    circle.type = SHAPE_CIRCLE;
    circle.params.circle.cx = 40;
    circle.params.circle.cy = 10;
    circle.params.circle.r = 6;
    circle.draw_char = 'O';
    shapes[shape_count++] = circle;
    // Triangle
    Shape tri;
    tri.id = next_shape_id++;
    tri.type = SHAPE_TRIANGLE;
    tri.params.triangle.x1 = 20;
    tri.params.triangle.y1 = 12;
    tri.params.triangle.x2 = 28;
    tri.params.triangle.y2 = 18;
    tri.params.triangle.x3 = 12;
    tri.params.triangle.y3 = 18;
    tri.draw_char = 'A';
    shapes[shape_count++] = tri;
    printf("\n%sDemo scene loaded successfully!%s\n", ANSI_GREEN, ANSI_RESET);
}
#ifndef TEST_MODE
int main() {
    // Enable VT100 support on Windows console (best effort)
#ifdef _WIN32
    // Some Windows consoles need explicit initialization for ANSI,
    // but Windows 10/11 handles it natively. We'll output a clear code.
#endif
    init_canvas(canvas, canvas_width, canvas_height);
    while (1) {
        printf("\n======================================\n");
        printf("    %s%s2D GRAPHICS EDITOR (C Language)%s\n", ANSI_BOLD, ANSI_MAGENTA, ANSI_RESET);
        printf("======================================\n");
        printf(" Canvas: %s%d x %d%s | Shapes: %s%d/%d%s\n", 
               ANSI_CYAN, canvas_width, canvas_height, ANSI_RESET, 
               ANSI_CYAN, shape_count, MAX_SHAPES, ANSI_RESET);
        printf("--------------------------------------\n");
        printf("1. Display Canvas & Rendered Shapes\n");
        printf("2. Add Shape (Line, Rect, Circle, Tri)\n");
        printf("3. Modify Shape Properties\n");
        printf("4. Delete Shape\n");
        printf("5. Clear Canvas (Delete All)\n");
        printf("6. Resize Canvas Grid\n");
        printf("7. Load Demo Scene\n");
        printf("8. Save Drawing to File\n");
        printf("9. Load Drawing from File\n");
        printf("10. Export ASCII Art to Text File\n");
        printf("11. Exit Program\n");
        printf("======================================\n");
        int choice = get_valid_int("Select option", 1, 11);
        switch (choice) {
            case 1:
                display_canvas();
                break;
            case 2:
                add_shape();
                display_canvas();
                break;
            case 3:
                modify_shape();
                display_canvas();
                break;
            case 4:
                delete_shape();
                display_canvas();
                break;
            case 5:
                clear_canvas();
                display_canvas();
                break;
            case 6:
                change_canvas_dimensions();
                display_canvas();
                break;
            case 7:
                load_demo();
                display_canvas();
                break;
            case 8:
                save_shapes_to_file();
                break;
            case 9:
                load_shapes_from_file();
                display_canvas();
                break;
            case 10:
                export_canvas_to_file();
                break;
            case 11:
                printf("\nThank you for using the 2D Graphics Editor. Goodbye!\n");
                return 0;
        }
    }
    return 0;
}
#else
int main() {
    printf("Running automated tests...\n");
    
    // Initialize canvas
    init_canvas(canvas, canvas_width, canvas_height);
    
    // Test 1: Bresenham Line
    draw_line(canvas, canvas_width, canvas_height, 0, 0, 10, 10, '*');
    if (canvas[0][0] != '*' || canvas[5][5] != '*' || canvas[10][10] != '*') {
        printf("Line test FAILED!\n");
        return 1;
    }
    printf("Line test PASSED.\n");
    // Test 2: Midpoint Circle
    init_canvas(canvas, canvas_width, canvas_height);
    draw_circle(canvas, canvas_width, canvas_height, 10, 10, 5, '*');
    if (canvas[5][10] != '*' || canvas[15][10] != '*' || canvas[10][5] != '*' || canvas[10][15] != '*') {
        printf("Circle test FAILED!\n");
        return 1;
    }
    printf("Circle test PASSED.\n");
    // Test 3: Outline Rectangle
    init_canvas(canvas, canvas_width, canvas_height);
    draw_rect(canvas, canvas_width, canvas_height, 0, 0, 5, 5, 0, '*');
    if (canvas[0][0] != '*' || canvas[0][5] != '*' || canvas[5][0] != '*' || canvas[5][5] != '*' || canvas[2][2] != '_') {
        printf("Rectangle outline test FAILED!\n");
        return 1;
    }
    printf("Rectangle outline test PASSED.\n");
    // Test 4: Filled Rectangle
    init_canvas(canvas, canvas_width, canvas_height);
    draw_rect(canvas, canvas_width, canvas_height, 0, 0, 5, 5, 1, '*');
    if (canvas[2][2] != '*' || canvas[1][1] != '*') {
        printf("Rectangle filled test FAILED!\n");
        return 1;
    }
    printf("Rectangle filled test PASSED.\n");
    // Test 5: Triangle outline
    init_canvas(canvas, canvas_width, canvas_height);
    draw_triangle(canvas, canvas_width, canvas_height, 0, 0, 10, 0, 0, 10, '*');
    if (canvas[0][0] != '*' || canvas[0][10] != '*' || canvas[10][0] != '*') {
        printf("Triangle test FAILED!\n");
        return 1;
    }
    printf("Triangle test PASSED.\n");
    // Test 6: Save and Load shapes
    init_canvas(canvas, canvas_width, canvas_height);
    shape_count = 0;
    next_shape_id = 1;
    
    // Add some test shapes
    Shape s1;
    s1.id = next_shape_id++;
    s1.type = SHAPE_LINE;
    s1.draw_char = 'X';
    s1.params.line.x1 = 2;
    s1.params.line.y1 = 3;
    s1.params.line.x2 = 10;
    s1.params.line.y2 = 12;
    shapes[shape_count++] = s1;
    Shape s2;
    s2.id = next_shape_id++;
    s2.type = SHAPE_RECTANGLE;
    s2.draw_char = 'R';
    s2.params.rect.x1 = 5;
    s2.params.rect.y1 = 5;
    s2.params.rect.x2 = 20;
    s2.params.rect.y2 = 15;
    s2.params.rect.filled = 1;
    shapes[shape_count++] = s2;
    const char *temp_file = "temp_test_shapes.txt";
    if (!save_shapes(temp_file)) {
        printf("Save shapes test FAILED (could not save file)!\n");
        return 1;
    }
    // Clear shapes
    shape_count = 0;
    if (!load_shapes(temp_file)) {
        printf("Load shapes test FAILED (could not load file)!\n");
        remove(temp_file);
        return 1;
    }
    if (shape_count != 2) {
        printf("Load shapes test FAILED (shape count mismatch: expected 2, got %d)!\n", shape_count);
        remove(temp_file);
        return 1;
    }
    if (shapes[0].type != SHAPE_LINE || shapes[0].draw_char != 'X' || shapes[0].params.line.x1 != 2 || shapes[0].params.line.y2 != 12) {
        printf("Load shapes test FAILED (shape 1 details mismatch)!\n");
        remove(temp_file);
        return 1;
    }
    if (shapes[1].type != SHAPE_RECTANGLE || shapes[1].draw_char != 'R' || shapes[1].params.rect.filled != 1) {
        printf("Load shapes test FAILED (shape 2 details mismatch)!\n");
        remove(temp_file);
        return 1;
    }
    remove(temp_file);
    printf("Save/Load shapes test PASSED.\n");
    // Test 7: Export ASCII Art
    const char *temp_art_file = "temp_test_art.txt";
    if (!export_canvas(temp_art_file)) {
        printf("Export canvas test FAILED!\n");
        return 1;
    }
    remove(temp_art_file);
    printf("Export canvas test PASSED.\n");
    printf("All automated tests completed successfully!\n");
    return 0;
}
#endif
