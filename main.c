#include <stdio.h>
#include <string.h>
#include <stdbool.h>

const int AVAILABLE_QUEUE = -1;
const int DATA_SIZE = 2048;
const int MAX_QUEUE_COUNT = 64;
// 130 bytes preserved for queue sizes:
        // 128 bytes for queue sizes (2 bytes * 64 queues)
        // 2 bytes for sum of queue sizes
const int MAX_BYTES_COUNT = DATA_SIZE - 130;
unsigned char data[DATA_SIZE];
typedef int16_t Q;
typedef int16_t QUEUE_BYTE_SIZE;

// error fxns
void on_out_of_memory() {}
void on_illegal_operation() {}

//helpers
void init_data();
void print_queue();
bool is_valid_queue(Q* queue);
bool must_shift_right(Q* queue);
bool must_shift_left(Q* queue);

// provided definitions
Q* create_queue();
void destroy_queue(Q* queue);
void enqueue_byte(Q* queue, unsigned char byte);
unsigned char dequeue_byte(Q* queue);

//tests
void test_fill_data_array();

int main() {
    init_data();                     
    Q* q0 = create_queue();
    enqueue_byte(q0, 0);             
    enqueue_byte(q0, 1);             
    Q* q1 = create_queue();
    enqueue_byte(q1, 3);             
    enqueue_byte(q0, 2);             
    enqueue_byte(q1, 4);                 
    printf("%d", dequeue_byte(q0));  
    printf("%d\n", dequeue_byte(q0));
    enqueue_byte(q0, 5);             
    enqueue_byte(q1, 6);
    printf("%d", dequeue_byte(q0));  
    printf("%d\n", dequeue_byte(q0));
    destroy_queue(q0);
    printf("%d", dequeue_byte(q1));
    printf("%d", dequeue_byte(q1));
    printf("%d\n", dequeue_byte(q1));
    destroy_queue(q1);
    return 0;
}

void init_data() {
    for (int i = 0; i < MAX_QUEUE_COUNT; i++) {
        memcpy(data + sizeof(QUEUE_BYTE_SIZE) * i, &AVAILABLE_QUEUE, sizeof(AVAILABLE_QUEUE));
    }
    memset(data + 128,0,sizeof(QUEUE_BYTE_SIZE)); // set total size = 0
}

void print_queue() {
    printf("QUEUE:\n");
    int16_t val;
    // queue sizes and total queue size
    for (int i = 0; i < 64; i++){
        memcpy(&val, data + 2 * i, sizeof val);
        printf("%d ",val);
    }
    memcpy(&val,&data[128], sizeof val);
    printf("\nTotal Size: %d\n",val);
    int idx = 130;
    while (idx < 2048) {
        printf("%d ",data[idx]);
        idx++;
    }
    printf("\n");
}

// ensures queue address points to correct (even) location in data array 
// ans is created already
bool is_valid_queue(Q* queue) {
    int queue_offset = (void*)queue - (void*)data;
    return queue != NULL && queue_offset < (MAX_QUEUE_COUNT * sizeof(Q)) && queue_offset % 2 == 0 && (*queue) != AVAILABLE_QUEUE;
}

bool must_shift_right(Q* queue) {
    // must shift if subsequent queues are non-empty
    queue++;
    int sizes = 0;
    Q* queue_end = ((Q*) data) + MAX_QUEUE_COUNT;
    for (; queue < queue_end; queue++) {
        if (*queue > AVAILABLE_QUEUE) {
            sizes += *queue;
        }
        if (sizes > 0) {
            return true;
        }
    }
    return sizes > 0;
}

bool must_shift_left(Q* queue) {
    Q* total_queue_size = (Q*)(data + MAX_QUEUE_COUNT * sizeof(QUEUE_BYTE_SIZE));
    return *total_queue_size != 1 || *queue != 1;
}


Q* create_queue() {
    Q* queue = (Q*) data;
    Q* queue_end = queue + MAX_QUEUE_COUNT;
    for (; queue != queue_end; queue++) {
        if (*queue == AVAILABLE_QUEUE) {
            *queue = 0;
            return queue;
        }
    }
    on_out_of_memory();
    return NULL;
}

void destroy_queue(Q* queue) {
    if (is_valid_queue(queue) && *queue == 0) {
        *queue = AVAILABLE_QUEUE;
    }
    else {
        on_illegal_operation();
    }
}

void enqueue_byte(Q* queue, unsigned char byte) {
    Q* total_queue_size = (Q*)(data + MAX_QUEUE_COUNT * sizeof(QUEUE_BYTE_SIZE));
    if (!is_valid_queue(queue)) {
        on_illegal_operation();
    }
    else if (*total_queue_size < MAX_BYTES_COUNT) {
        Q* current_queue = (Q*)data;
        int free_space_byte_offset = *queue;
        for (; current_queue != queue; current_queue++) {
            if (*current_queue > 0) {
                free_space_byte_offset += *current_queue;
            }
        }

        unsigned char* free_byte = data + 130 + free_space_byte_offset; //points to first element of next queue
        if (must_shift_right(queue)) {
            int shift_amount = *total_queue_size - free_space_byte_offset;
            memmove(free_byte + 1, free_byte, shift_amount);
        }
        *free_byte = byte;
        (*queue)++;
        (*total_queue_size)++;
    }
    else {
        on_out_of_memory();
    }
}

unsigned char dequeue_byte(Q* queue) {
    if (!is_valid_queue(queue) || *queue <= 0) {
        on_illegal_operation();
        return '\0';
    }
    else {
        Q* current_queue = (Q*) data;
        int dequeued_byte_offset = 0;
        for (; current_queue != queue; current_queue++) {
            if (*current_queue > 0) {
                dequeued_byte_offset += *current_queue;
            }
        }

        unsigned char* pByte = data + 130 + dequeued_byte_offset;
        unsigned char byte = *pByte;
        Q* total_queue_size = (Q*) (data + MAX_QUEUE_COUNT * sizeof(QUEUE_BYTE_SIZE));
        int byte_shifts = *total_queue_size - dequeued_byte_offset - 1;
        if (must_shift_left(queue)) {
            while ((pByte + 1) < &data[DATA_SIZE] && byte_shifts > 0) {
                *pByte = *(pByte + 1);
                pByte++;
                byte_shifts--;
            }
        }
        *pByte = 0;
        (*queue)--;
        (*total_queue_size)--;
        return byte;
    }
}


void test_fill_data_array() {
    Q* queues[14];
    init_data();
    for (int i = 0; i < 14; i++) {
        Q* q = create_queue();
        for (int b = 0; b < 137; b++) {
            enqueue_byte(q,b);
        }
        queues[i] = q;
    }
    print_queue();
    dequeue_byte(queues[13]);
    print_queue();
    dequeue_byte(queues[13]);
    print_queue();
    dequeue_byte(queues[0]);
    print_queue();
    dequeue_byte(queues[0]);
    print_queue();
}