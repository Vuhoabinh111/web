#include <stdio.h>
#include <time.h>

void getCurrentTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

int main() {
    char currentTime[20];
    getCurrentTime(currentTime);
    printf("Current Time: %s\n", currentTime);
    return 0;
}
