#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

using namespace std;

class LinkedList;    // 為了將class LinkedList設成class ListNode的friend,
                     // 需要先宣告
class ListNode{
    char id[10];
    char name[20];
    int deposit;
    ListNode *next;
public:
    ListNode():name(""),id(""),deposit(0),next(nullptr){};
    ListNode(char* p_name, char* p_id, int p_deposit){
        strncpy(id, p_id, 20);
        strncpy(name, p_name, 20);
        deposit = p_deposit;
    }

    friend class LinkedList;
};

class LinkedList{
    ListNode *head;     // list的第一個node
    int node_count;     // 紀錄list內的資料量

public:
    LinkedList():head(nullptr),node_count(0){};

    // LinkedList操作類的Member function
    int search(char*, int, char*);
    int Push_back(char*, char*, int);   // 在list的尾巴新增node
    int Delete(char*);                  // 刪除list中的 int x

    // 獲取LinkedList資訊類的Member function
    void PrintList(char**, int*, int);       // 輸出list的所有資料
    int getNodeCount(){     // 獲得list內的資料量
        return node_count;
    }

    // 存檔
    void save_to_file();
};

void LinkedList::PrintList(char** data, int* data_length, int max_len){
    if (node_count==0) {                   // 如果data_count為0代表list內沒有資料
        int length = snprintf(data[0], max_len, "Linked-List內沒有資料！\n");
        data_length[0] = length;
    }

    ListNode *current = head;
    for (int i = 0; i < node_count; i++){
        int length = snprintf(data[i], max_len, "Name:%s\tID:%s\tDeposit:%d\n", current->name, current->id, current->deposit);
        data_length[i] = length;
        current = current->next;
    }
}

int LinkedList::Push_back(char* p_name, char* p_id, int p_deposit){
    ListNode *newNode = new ListNode(p_name, p_id,p_deposit);   // 配置新的記憶體

    if (head == nullptr) {                      // 若list沒有node, 令newNode為first
        head = newNode;
        node_count++;
        printf("新增成功！\n");
        return 0;
    }

    ListNode *current = head;
    while (current->next != 0) {           // Traversal
        int cmp = strcmp(p_id, current->id);
        if (cmp==0){
            return 1;
        }
        current = current->next;
    }
    int cmp = strcmp(p_id, current->id);
        if (cmp==0){
            return 1;
        }
    current->next = newNode;               // 將newNode接在list的尾巴
    node_count++;
    printf("新增成功！\n");
    return 0;
}

int LinkedList::Delete(char* t_id){

    ListNode *current = head,      
             *previous = 0;

    while (current != 0 && strcmp(current->id, t_id) != 0) {  // Traversal
        previous = current;                       // 如果current指向NULL
        current = current->next;                  // 或是current->data == x
    }                                             // 即結束while loop

    if (current == 0) {                 // list沒有要刪的node, 或是list為empty
        cout << "ID不存在！\n";
        return 1;
    }
    else if (current == head) {        // 要刪除的node剛好在list的開頭
        head = current->next;          // 把first移到下一個node
        delete current;                 // 如果list只有一個nollde, 那麼first就會指向NULL
        current = 0;                    // 當指標被delete後, 將其指向NULL, 可以避免不必要bug
        // return;
        cout << "刪除成功！\n";
        node_count--;
        return 0;
    }
    else {                              // 其餘情況, list中有欲刪除的node, 
        previous->next = current->next; // 而且node不為first, 此時previous不為NULL
        delete current;
        current = 0;
        // return;
        cout << "刪除成功！\n";
        node_count--;
        return 0;
    }
}

int LinkedList::search(char* outStr, int max_len, char* t_id){
    ListNode *current = head;

    while (current != 0 && strcmp(current->id, t_id) != 0) {
        current = current->next;
    }

    if (current == 0) {                 // list 沒有要找的 node
        cout << "There is no " << t_id << " in list.\n";
        // return;
        int str_length = snprintf(outStr, max_len, "資料不存在！\n");
        return str_length;
    }
    else {                              // list 中有要找的 node, 
        cout << "找到資料！\n";
        // return;
        int str_length = snprintf(outStr, max_len, "Name:%s\tID:%s\tDeposit:%d\n", current->name, current->id, current->deposit);
        printf("%s", outStr);
        return str_length;
    }
}

void LinkedList::save_to_file(){
    // 開檔
    FILE *fp = fopen("output.txt", "w");    //每次存檔都重新寫入全部的資料。
    ListNode *current = head;

    while(current!=nullptr){
        fprintf(fp, "Name:%s\tID:%s\tDeposit:%d\n", current->name, current->id, current->deposit);
        current = current->next;
    }
    // 關檔
    fclose(fp);
}

int main(){
    LinkedList linked_list;
    // create fifo if not exist
    if(mkfifo("send_to_back", 0777)==-1||mkfifo("send_to_front", 0777)==-1){
        if(errno!=EEXIST){
            printf("建立FIFO檔發生錯誤，程式中止。\n");
            return 2;
        }
    }
    // open fifo
    int fd[2];
    fd[0] = open("send_to_back", O_RDONLY);
    fd[1] = open("send_to_front", O_WRONLY);
    if(fd[0] == -1 && fd[1] == -1){
        printf("開啟檔案時發生錯誤，程式中止。\n");
        return 1;
    }
    // 主要功能：按照選擇的功能執行任務
    while(1){
        int x = 0;
        if (read(fd[0], &x, sizeof(int)) == -1){
            return 2;
        }
        if(x==1){   // insert data record
            char id[10], name[20];
            int deposit;
            // get 3 parameters from front-end
            read(fd[0], name, sizeof(char) * 20);
            read(fd[0], id, sizeof(char) * 10);
            read(fd[0], &deposit, sizeof(int));
            // send back the result of insert
            int insert_result = linked_list.Push_back(name, id, deposit);
            if (write(fd[1], &insert_result, sizeof(int)) == -1)
            {
                return 2;
            }
        }
        else if(x==2){  //search data record
            // get target id
            char target_id[10];
            if(read(fd[0], target_id, sizeof(char) * 10)==-1){
                return 2;
            }
            // get result of search
            char search_result[100];
            int result_length = linked_list.search(search_result, 100, target_id);
            // send result back to front-end
            if(write(fd[1], &result_length, sizeof(int))==-1){
                return 2;
            }
            if(write(fd[1], search_result, sizeof(char) * result_length)==-1){
                return 2;
            }
        }
        else if(x==3){  // delete data record
            char target_id[10];
            if(read(fd[0], target_id, sizeof(char) * 10)==-1){
                return 2;
            }
            // try to delete data record
            int result = linked_list.Delete(target_id);
            // send result back to front-end
            if(write(fd[1], &result, sizeof(int))==-1){
                return 2;
            }
        }
        else if(x==4){  // list all data record
            // 獲取data_count
            int data_count = linked_list.getNodeCount();
            if (write(fd[1], &data_count, sizeof(int)) == -1)
            {
                return 2;
            }
            // 宣告用來存資料的陣列
            if(data_count==0)   // 先確認data_count是否為0，因為沒有資料時還是要傳"list為空"
                data_count++;   // 在宣告陣列大小時要留一格空間，所以此處的data_count並不完全等於node_count。
            char **data = new char *[data_count];
            for (int i = 0; i < data_count;i++){
                data[i] = new char[100];
            }
            // 宣告用來儲存data[i]長度的陣列
            int data_length[data_count] = {0};

            // 獲取資料
            linked_list.PrintList(data, data_length, 100);

            // 開始傳送資料，先傳送每個data[i]的長度
            for (int i = 0; i < data_count; i++){
                if(write(fd[1], &data_length[i], sizeof(int)) == -1){
                    return 2;
                }
            }
            // 再依照data[i]的長度傳送每一個data[i]
            for (int i = 0; i < data_count; i++){
                if(write(fd[1], data[i], sizeof(char) * data_length[i]) == -1){
                    return 2;
                }
            }
        }
        // auto save after list edited
        if(x!=0&&x!=4){
            linked_list.save_to_file();
        }
        // x初始化，避免等待前端時發生錯誤。
        x = 0;
    }
    // close fifo file
    close(fd[0]);
    close(fd[1]);

    return 0;
}