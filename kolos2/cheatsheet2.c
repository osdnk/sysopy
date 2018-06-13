
// kolejki komunikatow

// System V

int main() {
    key_t publicKey = ftok(path, PROJECT_ID);
    if(publicKey == -1)
        FAILURE_EXIT("server: generation of publicKey failed\n");

    int queue_descriptor = msgget(publicKey, IPC_CREAT | IPC_EXCL | 0666);
    if(queue_descriptor == -1)
        FAILURE_EXIT("server: creation of public queue failed\n");

    Message buffer;
    while(1) {
        if (active == 0) {
            if (msgctl(queue_descriptor, IPC_STAT, &current_state) == -1)
                FAILURE_EXIT("server: getting current state of public queue failed\n");
            if (current_state.msg_qnum == 0) break;
        }

        if (msgrcv(queue_descriptor, &buffer, MSG_SIZE, 0, 0) < 0)
            FAILURE_EXIT("server: receiving message failed\n");
        handle_public_queue(&buffer);
    }

    int tmp = msgctl(queue_descriptor, IPC_RMID, NULL);
};
int sender() {
    queue_descriptor = create_queue(path, PROJECT_ID);

    key_t privateKey = ftok(path, getpid());
    if (privateKey == -1)
        FAILURE_EXIT("Generation of private key failed");

    privateID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666);
    if (privateID == -1)
        FAILURE_EXIT("Creation of private queue failed");

    Message msg;
    msg.mtype = LOGIN;
    msg.sender_pid = getpid();
    sprintf(msg.message_text, "%d", privateKey);

    if (msgsnd(queue_descriptor, &msg, MSG_SIZE, 0) == -1)
        FAILURE_EXIT("client: LOGIN request failed\n");
    if (msgrcv(privateID, &msg, MSG_SIZE, 0, 0) == -1)
        FAILURE_EXIT("client: catching LOGIN response failed\n")
}

// - POSIX

int main() {
    mqd_t queue_descriptor = mq_open(server_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posix_attr);

    if (queue_descriptor == -1)
        FAILURE_EXIT("server: creation of public queue failed\n");

    Message buffer;
    while(1) {
        if(active == 0) {
            if (mq_getattr(queue_descriptor, &current_state) == -1)
                FAILURE_EXIT("server: couldnt read public queue parameters\n");
            if (current_state.mq_curmsgs == 0) exit(0);
        }

        if (mq_receive(queue_descriptor,(char*) &buffer, MESSAGE_SIZE, NULL) == -1)
            FAILURE_EXIT("server: receiving message by server failed\n");
        handle_public_queue(&buffer);
    }
    mq_close(queue_descriptor);
    mq_unlink(server_path);

};

ins sender ( ){
    queue_descriptor = mq_open(server_path, O_WRONLY);
    mq_send(queue_descriptor, (char*) &msg, MESSAGE_SIZE, 1);
    mq_close(queue_descriptor);
}

//Shared memory - SYSTEM V


struct shared_struct{
    int val;
};
// server
int main(int argc, char **argv)
{

    if(argc !=2){
        printf("Not a suitable number of program parameters\n");
        return(1);
    }
    key_t my_key = ftok(SHM_NAME, 4);
    int shm_id = shmget(my_key, sizeof(struct shared_struct), IPC_CREAT | S_IRWXU);
    struct shared_struct *my_shared = shmat(shm_id, NULL, 0);
    my_shared -> val = atoi(argv[1]);
    return 0;
}

//receiver

int main(int argc, char **argv)
{

    sleep(1);
    int val =0;
    key_t my_key = ftok(SHM_NAME, 4);
    int shm_id = shmget(my_key, 0, 0);
    struct shared_struct *got_shared_struct=shmat(shm_id, NULL, 0);
    printf("Liczba: %d\n", got_shared_struct -> val);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}


// POSIX
// sender
int main(int argc, char **argv)
{

    if(argc !=2){
        printf("Not a suitable number of program parameters\n");
        return(1);
    }

    int fd_shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRWXU);
    off_t shared_struct_size=sizeof(struct shared_struct);
    ftruncate(fd_shm, shared_struct_size);
    void* shm_addr = mmap(NULL, shared_struct_size, PROT_WRITE, MAP_SHARED, fd_shm, 0);
    struct shared_struct *my_shared = (struct shared_struct *) shm_addr;
    my_shared->val=atoi(argv[1]);
    sleep(152);
    munmap(shm_addr, shared_struct_size);
    shm_unlink(SHM_NAME);

    return 0;
}

// receiver
int main(int argc, char **argv)
{

    sleep(1);
    int val =0;
    int fd_shm=shm_open(SHM_NAME, O_RDWR, S_IRWXU);
    off_t shared_struct_size=sizeof(struct shared_struct);
    ftruncate(fd_shm, shared_struct_size);
    void* shm_addr = mmap(NULL, shared_struct_size, PROT_WRITE, MAP_SHARED, fd_shm, 0);
    struct shared_struct *my_shared = (struct shared_struct*) shm_addr;
    printf("My val: %d\n", my_shared->val);
    return 0;
}


// SEMAFOR - V

key_t sem_key = ftok(FILE_NAME, 2);
int sem_id = semget(sem_key, 1, IPC_CREAT | S_IRWXU);
union semun sem_info;
sem_info.val =1;
semctl(sem_id, 0, SETVAL, sem_info);

struct sembuf* sops=malloc(sizeof(struct sembuf));
sops -> sem_num=0;
sops -> sem_flg=0;


sops -> sem_op = -1;
semop(sem_id, sops, 1);


sops -> sem_op = 1;
semop(sem_id, sops, 1);



// POSIX
sem_t *my_semaphore=sem_open(SEM_NAME, O_RDWR | O_CREAT, S_IRWXU, 1);

sem_wait(my_semaphore);
sem_post(my_semaphore)K

sem_close(my_semaphore);
sem_unlink(SEM_NAME);

// Wątki


int i;
pthread_t arr[n];
int indexes[n];
for(i=0; i<n; i++) {
indexes[i]=i;
pthread_create(&arr[i], NULL, hello, &indexes[i]);
}



for(i=0; i<n; i++)
pthread_cancel(arr[i]);

for(i=0; i<n; i++)
pthread_join(arr[i], NULL);

// MUTEX



int x;
pthread_mutex_t x_mutex = PTHREAD_MUTEX_INITIALIZER;

void my_thread_safe_function(...) {
    /* Każdy dostęp do zmiennej x powinien się odbywać w następujący sposób: */
    pthread_mutex_lock(&x_mutex);
    /* operacje na x... */
    pthread_mutex_unlock(&x_mutex);
}



int x,y;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;

...
k

// (Watek 1)
// Czekanie aż x jest większe od y jest
// przeprowadzane następująco:

pthread_mutex_lock(&mutex);
while (x <= y) {
pthread_cond_wait(&cond, &mutex);
}

...

pthread_mutex_unlock(&mutex);

...


// (Watek 2)
// Kazda modyfikacja x lub y może
// powodować zmianę warunków. Należy
// obudzić pozostałe wątki, które korzystają
// z tego warunku sprawdzającego.

pthread_mutex_lock(&mutex);
/* zmiana x oraz y */
if (x > y)
pthread_cond_broadcast(&cond);
pthread_mutex_unlock(&mutex);
