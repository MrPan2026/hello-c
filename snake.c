#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>

// 游戏配置
#define WIDTH  30   // 地图宽度
#define HEIGHT 20   // 地图高度
#define INIT_LEN 3  // 蛇初始长度

// 方向枚举
typedef enum {
    STOP = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// 蛇身坐标
int snakeX[WIDTH * HEIGHT];
int snakeY[WIDTH * HEIGHT];
// 蛇长度、当前方向、食物坐标、分数
int length, dir, foodX, foodY, score;
// 终端原始属性（用于按键监听）
struct termios old_tio;

// 恢复终端原始模式（程序退出时调用）
void resetTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

// 设置终端为无缓冲模式（无需回车监听按键）
void setTerminal() {
    tcgetattr(STDIN_FILENO, &old_tio);
    atexit(resetTerminal); // 程序退出自动恢复终端
    struct termios new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO); // 关闭回显+行缓冲
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

// 非阻塞监听按键
int keyHit() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

// 读取按键
char getKey() {
    if (keyHit()) return getchar();
    return 0;
}

// 初始化游戏
void initGame() {
    setTerminal();
    srand(time(0)); // 随机数种子
    dir = STOP;     // 初始静止
    // 蛇初始位置（屏幕中心）
    snakeX[0] = WIDTH / 2;
    snakeY[0] = HEIGHT / 2;
    length = INIT_LEN;
    score = 0;
    // 随机生成第一个食物
    foodX = rand() % WIDTH;
    foodY = rand() % HEIGHT;
}

// 绘制游戏界面
void draw() {
    system("clear"); // 清屏（Linux终端命令）
    // 绘制上边界
    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) printf("#"); // 左边界

            if (i == snakeY[0] && j == snakeX[0]) printf("O"); // 蛇头
            else {
                int isBody = 0;
                for (int k = 1; k < length; k++) { // 蛇身
                    if (snakeX[k] == j && snakeY[k] == i) {
                        printf("o");
                        isBody = 1;
                        break;
                    }
                }
                if (!isBody) {
                    if (i == foodY && j == foodX) printf("F"); // 食物
                    else printf(" "); // 空白
                }
            }
            if (j == WIDTH - 1) printf("#"); // 右边界
        }
        printf("\n");
    }

    // 绘制下边界
    for (int i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");
    printf("分数: %d | 控制: WASD/方向键 | 退出: Ctrl+C\n", score);
}

// 处理按键输入
void input() {
    char c = getKey();
    switch (c) {
        case 'w': case 'W': case 65: if (dir != DOWN) dir = UP; break;    // 上
        case 's': case 'S': case 66: if (dir != UP) dir = DOWN; break;    // 下
        case 'a': case 'A': case 68: if (dir != RIGHT) dir = LEFT; break; // 左
        case 'd': case 'D': case 67: if (dir != LEFT) dir = RIGHT; break; // 右
        case 3: exit(0); // Ctrl+C 退出
    }
}

// 游戏逻辑更新
void update() {
    // 蛇身跟随蛇头移动（从尾部向前更新）
    for (int i = length - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    // 移动蛇头
    switch (dir) {
        case UP: snakeY[0]--; break;
        case DOWN: snakeY[0]++; break;
        case LEFT: snakeX[0]--; break;
        case RIGHT: snakeX[0]++; break;
        default: break;
    }

    // 撞墙检测
    if (snakeX[0] < 0 || snakeX[0] >= WIDTH || snakeY[0] < 0 || snakeY[0] >= HEIGHT) {
        printf("\n游戏结束！撞墙了！最终分数：%d\n", score);
        exit(0);
    }

    // 撞自身检测
    for (int i = 1; i < length; i++) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            printf("\n游戏结束！撞到自己了！最终分数：%d\n", score);
            exit(0);
        }
    }

    // 吃到食物
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
        score += 10;   // 加分
        length++;      // 蛇变长
        // 重新生成食物（不生成在蛇身上）
        foodX = rand() % WIDTH;
        foodY = rand() % HEIGHT;
    }
}

// 主函数
int main() {
    initGame();
    while (1) {
        draw();   // 绘制界面
        input();  // 监听按键
        update(); // 更新游戏逻辑
        usleep(150000); // 控制游戏速度（150ms刷新一次）
    }
    return 0;
}
