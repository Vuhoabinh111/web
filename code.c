#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 50
#define MAX_NOTE 100

typedef enum {
    DANG_PHUC_VU,
    DA_THANH_TOAN,
    DON_HUY
} TrangThai;

typedef struct Dish {
    char maMon[MAX_NAME];
    int soluongDat;
    int soluongTra;
    char ghiChu[MAX_NOTE];
    struct Dish* next;
    TrangThai trangThai;
} Dish;

typedef struct Order {
    char thoiGian[20];
    char tenNhanVien[MAX_NAME];
    int maBan;
    Dish* danhSachMon;
    int tongSoMon;
    int tongSoDiaDat;
    int tongSoMonTra;
    int tongSoDiaTra;
    char thoiGianCapNhat[20];
    TrangThai trangThai;
    struct Order* next;
} Order;
void getCurrentTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}
Order* orders = NULL;

Order* search_order(int table_id) {
    Order* current = orders;
    while (current != NULL) {
        if (current->maBan == table_id && current->trangThai == DANG_PHUC_VU) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int create_order(int maBan, char* maNhanVien, char* thoiGian) {
    Order* existing = search_order(maBan);
    if (existing != NULL) return 0; 

    Order* newOrder = (Order*)malloc(sizeof(Order));
    strcpy(newOrder->thoiGian, thoiGian);
    strcpy(newOrder->tenNhanVien, maNhanVien);
    newOrder->maBan = maBan;
    newOrder->danhSachMon = NULL;
    newOrder->tongSoMon = 0;
    newOrder->tongSoDiaDat = 0;
    newOrder->tongSoMonTra = 0;
    newOrder->tongSoDiaTra = 0;
    newOrder->trangThai = DANG_PHUC_VU;
    newOrder->next = orders;
    orders = newOrder;
    return 1;
}


int add_dish(int table_id, char* maMon, int quantity, char* notes) {
    Order* order = search_order(table_id);
    if (order == NULL) {
        char currentTime[20];
        getCurrentTime(currentTime);
        if (!create_order(table_id, "DEFAULT", currentTime)) return 0;
        orders = orders;
    }

    Dish* newDish = (Dish*)malloc(sizeof(Dish));
    strcpy(newDish->maMon, maMon);
    newDish->soluongDat = quantity;
    newDish->soluongTra = 0;
    strcpy(newDish->ghiChu, notes);
    newDish->next = order->danhSachMon;
    order->danhSachMon = newDish;
    newDish->trangThai=DANG_PHUC_VU;


    order->tongSoMon++;
    order->tongSoDiaDat += quantity;
    getCurrentTime(order->thoiGianCapNhat);
    return 1;
}

int update_dish(int table_id, char* maMon, int quantity) {
    Order* order = search_order(table_id);
    if (order == NULL || order->trangThai != DANG_PHUC_VU) return 0;

    Dish* dish = order->danhSachMon;
    while (dish != NULL) {
        if (strcmp(dish->maMon, maMon) == 0) {
            dish->soluongTra += quantity;
            order->tongSoDiaTra += quantity;
            if (dish->soluongTra > 0) order->tongSoMonTra++;
            getCurrentTime(order->thoiGianCapNhat);
            return 1;
        }
        dish = dish->next;
    }
    return 0;
}

int cancel_dish(int table_id, char* maMon, char* notes) {
    Order* order = search_order(table_id);
    if (order == NULL || order->trangThai != DANG_PHUC_VU) return 0;

    Dish* dish = order->danhSachMon;
    while (dish != NULL) {
        if (strcmp(dish->maMon, maMon) == 0) {
            if (dish->soluongTra == 0) {
                strcpy(dish->ghiChu, notes);
                order->tongSoMon--;
                order->tongSoDiaDat-=dish->soluongDat;
                dish->soluongTra = 0;
                dish->trangThai = DON_HUY;
                getCurrentTime(order->thoiGianCapNhat);
                return 1;
            }
        }
        dish = dish->next;
    }
    return 0;
}

int cancel_order(int table_id) {
    Order* order = search_order(table_id);
    if (order == NULL) return 0;

    Dish* dish = order->danhSachMon;
    while (dish != NULL) {
        if (dish->soluongTra > 0) return 0;
        dish = dish->next;
    }

    order->trangThai = DON_HUY;
    getCurrentTime(order->thoiGianCapNhat);
    return 1;
}

void create_bill(int table_id) {
    Order* order = search_order(table_id);
    FILE* file = fopen("bill.txt", "w");
    if (file == NULL) return;

    while (order != NULL) {
        if (order->trangThai == DANG_PHUC_VU) {
            fprintf(file, "Thoi gian: %s\n", order->thoiGianCapNhat);
            fprintf(file, "Ban %d\n", order->maBan);

            Dish* dish = order->danhSachMon;
            while (dish != NULL) {
                if (dish->soluongTra > 0) {
                    fprintf(file, "%s : %d\n", dish->maMon, dish->soluongTra);
                }
                dish->trangThai = DA_THANH_TOAN;
                dish = dish->next;
            }

            fprintf(file, "Tong so mon: %d\n", order->tongSoMonTra);
            fprintf(file, "Tong so dia: %d\n\n", order->tongSoDiaTra);
            order->trangThai = DA_THANH_TOAN;
            break;
        }
        order = order->next;
    }
    fclose(file);
}
void finish_order(int maBan){
    Order* order = search_order(maBan);
    while(order!=NULL){
        if(order->trangThai == DANG_PHUC_VU){
            Dish *dish = order->danhSachMon;
            order->tongSoDiaTra = order->tongSoDiaDat;
            order->tongSoMonTra = order->tongSoMon;
            while(dish!=NULL){
                if(dish->trangThai == DANG_PHUC_VU){
                    dish->soluongTra = dish->soluongDat;
                }
                dish=dish->next;
            }
            order=order->next;
        }
    }
    return ;
}
int main() {
    FILE *file;
    char line[256];

    file = fopen("C:\\hoc\\OrdersManagers\\oders.txt", "r");
    if (file == NULL) {
        perror("Không thể mở file!");
        return 1;
    }

    char command[100];

    while (fgets(line, sizeof(line), file)) {  
        if (line[0] == '?') {  
            sscanf(line, "? %s", command);

            if (strcmp(command, "create_order") == 0) {
                while (fgets(line, sizeof(line), file)) {  
                    if (line[0] == '#' || strlen(line) < 2) break;

                    char tenNhanVien[MAX_NAME], thoiGian[100];
                    int maBan;

                    if (sscanf(line, "%s %d %s", tenNhanVien, &maBan, thoiGian) == 3) {
                        if (create_order(maBan, tenNhanVien, thoiGian)) {
                            printf("Order created: %s %d %s\n", tenNhanVien, maBan, thoiGian);
                        } else {
                            printf("Failed to create order for table %d\n", maBan);
                        }
                    }
                }
            }
            else if (strcmp(command, "add_dish") == 0) {
                while (fgets(line, sizeof(line), file)) {  
                    if (line[0] == '#' || strlen(line) < 2) break;

                    char maNhanVien[MAX_NAME], maMon[100], notes[MAX_NAME];
                    int maBan, quantity;
                    if (sscanf(line, "%s %d \"%[^\"]\" %d \"%[^\"]\"", 
                        maNhanVien, &maBan, maMon, &quantity, notes) == 4) {
                            char notes[]="";
                    if (add_dish(maBan, maMon, quantity, notes)) {
                     printf("Dish added: %s to table %d, quantity: %d, notes: %s\n", 
                            maMon, maBan, quantity, notes);
                    } else {
                     printf("Failed to add dish: %s to table %d\n", maMon, maBan);
                    }
                    } 
                    else if (sscanf(line, "%s %d \"%[^\"]\" %d \"%[^\"]\"", 
                        maNhanVien, &maBan, maMon, &quantity, notes) == 5) {
                    if (add_dish(maBan, maMon, quantity, notes)) {
                    printf("Dish added: %s to table %d, quantity: %d, notes: %s\n", maMon, maBan, quantity, notes);
                    } else {
                    printf("Failed to add dish: %s to table %d\n", maMon, maBan);
                    }
                    }
             
                }
            }
            else if (strcmp(command, "update_dish") == 0) {
                while (fgets(line, sizeof(line), file)) {
                    if (line[0] == '#' || strlen(line) < 2) break; 
                    
                    char maNhanVien[MAX_NAME], tenMon[MAX_NAME];
                    int maBan, soLuong;
                    
                    if (sscanf(line, "%s %d \"%[^\"]\" %d", maNhanVien, &maBan, tenMon, &soLuong) == 4) {
                        if (update_dish(maBan, tenMon, soLuong)) {
                            printf("Dish updated: %s for table %d, quantity: %d\n", tenMon, maBan, soLuong);
                        } else {
                            printf("Failed to update dish: %s for table %d\n", tenMon, maBan);
                        }
                    }
                }
            }
            else if (strcmp(command, "cancel_dish") == 0) {
                while (fgets(line, sizeof(line), file)) {
                    if (line[0] == '#' || strlen(line) < 2) break;  
                    
                    char maNhanVien[MAX_NAME], tenMon[MAX_NAME], ghiChu[MAX_NAME];
                    int maBan;
                    
                    if (sscanf(line, "%s %d \"%[^\"]\" \"%[^\"]\"", maNhanVien, &maBan, tenMon, ghiChu) == 4) {
                        if (cancel_dish(maBan, tenMon, ghiChu)) {
                            printf("Dish canceled: %s for table %d, reason: %s\n", tenMon, maBan, ghiChu);
                        } else {
                            printf("Failed to cancel dish: %s for table %d\n", tenMon, maBan);
                        }
                    }
                }
            }
            else if (strcmp(command, "create_bill") == 0) {
                while (fgets(line, sizeof(line), file)) {
                    if (line[0] == '#' || strlen(line) < 2) break;  
                    
                    char maNhanVien[MAX_NAME];
                    int maBan;
                    
                    if (sscanf(line, "%s %d", maNhanVien, &maBan) == 2) {
                        finish_order(maBan);
                        create_bill(maBan);
                        printf("Bill created for table %d by employee %s\n", maBan, maNhanVien);
                       
                    }
                }
            }
    }
    }

    fclose(file);
    return 0;
}

