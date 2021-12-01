#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

int main(){
    // create fifo if not exist
    if(mkfifo("send_to_back", 0777)==-1||mkfifo("send_to_front", 0777)==-1){
        if(errno!=EEXIST){
            printf("建立FIFO檔發生錯誤，程式中止。\n");
            return 2;
        }
    }
    // open fifo
    int fd[2];
    fd[0] = open("send_to_back", O_WRONLY);
    fd[1] = open("send_to_front", O_RDONLY);
    if(fd[0] == -1 || fd[1] == -1){
        printf("開啟檔案時發生錯誤，程式中止。\n");
        return 1;
    }
    // send command repeatedly
    int x;
    while ((printf("請輸入要使用的功能對應的號碼：\n1: Insert data record\n2: Search data record\n3: Delete data record\n4: List all data record\n5: Close the front-end\n\n選擇功能: "))&&(cin >> x)){
        if (x > 5 || x < 0)
        {   
            system("clear");
            printf("輸入的值有誤，請再試一次。\n");
            continue;
        }
        else if(x==5){
            system("clear");
            printf("Front-end closed.\n");
            break;
        }
        else{
            if (write(fd[0], &x, sizeof(int)) == -1){
                printf("寫入檔案發生錯誤，程式中止。\n");
                return 2;
            }
            if(x==1){   // insert data record
                char id[10], name[20];
                int deposit;
                // get the parameters and send to back-end
                printf("請輸入Name: ");
                scanf("%s", name);
                printf("請輸入ID: ");
                scanf("%s", id);
                printf("請輸入Deposit: ");
                scanf("%d", &deposit);
                if( write(fd[0], name, sizeof(char) * 20)==-1||
                    write(fd[0], id, sizeof(char) * 10)==-1||
                    write(fd[0], &deposit, sizeof(int))==-1)
                {
                    return 2;
                }
                // get result of insert from back-end
                int insert_resulf;
                read(fd[1], &insert_resulf, sizeof(int));
                if(insert_resulf==1){
                    system("clear");
                    printf("該ID已經存在，請重試。\n\n");
                }
                else if (insert_resulf==0){
                    system("clear");
                    printf("新增資料成功！\n\n");
                }
            }
            else if(x==2){  // search data record
                char target_id[10];
                // get target id and send to back-end
                printf("請輸入要查詢的ID: ");
                scanf("%s", target_id);
                if(write(fd[0], target_id, sizeof(char) * 10)==-1){
                    return 2;
                }
                system("clear");
                // get search result
                int len;
                char search_result[100];
                if(read(fd[1], &len, sizeof(int))==-1){
                    return 2;
                }
                if(read(fd[1], search_result, sizeof(char) * len)==-1){
                    return 2;
                }
                cout << search_result << endl;
            }
            else if(x==3){  // delete data record
                char target_id[10];
                // get target id and send to back-end
                printf("請輸入要刪除的ID: ");
                scanf("%s", target_id);
                if(write(fd[0], target_id, sizeof(char) * 10)==-1){
                    return 2;
                }
                system("clear");
                // get delete result
                int result;
                if (read(fd[1], &result, sizeof(int))==-1){
                    return 2;
                }
                if(result==0)
                    cout << "刪除成功！\n\n";
                else if(result==1)
                    cout << "ID不存在，請重試。\n\n";
            }
            else if(x==4){
                system("clear");

                // 獲取 data_count
                int data_count;
                read(fd[1], &data_count, sizeof(int));

                // 宣告用來存資料的陣列
                if(data_count==0)   // 先確認data_count是否為0，因為沒有資料時還是要傳"list為空"
                    data_count++;
                char **data = new char *[data_count];
                for (int i = 0; i < data_count;i++){
                    data[i] = new char[100];
                }
                // 宣告用來儲存data[i]長度的陣列
                int data_length[data_count] = {0};

                // 先接收儲存data[i]長度的陣列
                for (int i = 0; i < data_count; i++)
                {
                    if(read(fd[1], &data_length[i], sizeof(int)) == -1){
                        return 2;
                    }
                }
                // 再依照長度個別接收data[i]
                for (int i = 0; i < data_count; i++){
                    if(read(fd[1], data[i], sizeof(char)*data_length[i]) == -1){
                        return 2;
                    }
                    cout << data[i];
                }
                cout << endl;
            }
        }
    }
    // close fifo file
    close(fd[0]);
    close(fd[1]);

    return 0;
}