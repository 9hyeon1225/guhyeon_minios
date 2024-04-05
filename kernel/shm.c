#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<unistd.h>
#include<stddef.h>
#include<sys/types.h>
#include<sys/mman.h>

#define SHM_SIZE 1048576


void SHM(){

int shm_fd;
void *ptr;

char mem[SHM_SIZE]={0}; //shm�� �Ҵ��ϴµ� �־ �ش� �޸𸮰� �Ҵ�Ǿ��ִ��� �ƴ��� üũ�ϴ� �迭

shm_fd = shm_open("memory", O_CREAT | O_RDWR, 0666); //shm ���׸�Ʈ����
if(shm_fd==-1){
perror("shm_open");
exit( EXIT_FAILURE);}

ftruncate(shm_fd, SHM_SIZE); 

ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd,0); 

if(ptr== MAP_FAILED){
perror("mmap");
exit( EXIT_FAILURE);}

int *memindex=((int*)ptr); //shm�� ���� 8��Ʈ�� IPC�� �ϱ����� �Ҵ�
*memindex=(int)8;//������ shm�� index�� ù �ּҿ� ����
char *size = ((char*)ptr+4); // shmalloc �Լ��� ������ �޸��� ũ��
char *shmallocflag=((char*)ptr+5);//shmalloc �Լ��� ����Ǹ� �ش� �ּ��� �� 1�� �����Ѵ�.
char *shmfreeflag=((char*)ptr+6);//shfree�Լ��� ����Ǹ� �ش� �ּ��� ���� 1�� �����Ѵ�.
*size=0;
*shmallocflag=0;
*shmfreeflag=0;

while(1){
if(*shmallocflag==1){//shmalloc �Լ� ������


while(mem[*memindex+*size-1])//mem[*memindex+*size-1]�� ���� 1�̸� �޸𸮰� �Ҵ�Ǿ��ִٴ� ���̴�.
{*memindex+=*size;} //������������� �Ҵ��� size��ŭ ������Ų��.

mem[*memindex-8]=1;//�Ҵ簡���ϸ� �Ҵ��Ѵ�.

*memindex+=*size; //index�� size��ŭ Ű���� ���� �Ҵ��� �غ�
*shmallocflag=(char)0;//flag�� 0���� �����Ͽ� shmalloc �Լ����� �����ܰ� ����
}

else if(*((char*)ptr+6)==1){//shfree�Լ� ������

mem[*memindex-8]=0;//�Ҵ�����
 
*shmfreeflag=(char)0; 
}
usleep(1000);
if(getppid()==1){//�θ����μ����� �ü�����Ǹ� shm�Ҵ������ϰ� �ش����μ��� ����

munmap(ptr,SHM_SIZE);
shm_unlink("memory");

exit(0);};

}}


void *shmalloc(char size){
int shm_fd;
void *temp;
void *ptr;
shm_fd = shm_open("memory", O_CREAT | O_RDWR, 0666);
if(shm_fd==-1){
perror("shm_open");
exit( EXIT_FAILURE);}

ptr=mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd,0);
if(ptr== MAP_FAILED){
perror("mmap");
exit( EXIT_FAILURE);}//shm�� �����Ͽ� ù �ּ� ����


*((char*)ptr+4)=(char)size;//4��° ����Ʈ�� �Ҵ��� ũ�� ����
*((char*)ptr+5)=(char)1;//flag�� 1�μ����Ͽ� SHM()�Լ����� ó��
while(*((char*)ptr+5)){}//flag��0�̵ɶ����� ���

temp=(void*)((char*)ptr+*((int*)ptr));//�Ҵ��� �ּ� ���
return temp;//�ּ��Ҵ�
}

void shfree(void *adress){
int shm_fd;
void *ptr;
shm_fd = shm_open("memory", O_CREAT | O_RDWR, 0666);
if(shm_fd==-1){
perror("shm_open");
exit( EXIT_FAILURE);}
ptr=mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd,0);
if(ptr== MAP_FAILED){
perror("mmap");
exit( EXIT_FAILURE);}


ptrdiff_t temp=(char*)adress-(char*)ptr;
*((char*)ptr)=(int)temp;
*((char*)ptr+6)=(char)1;
while(*((char*)ptr+6)){}
}

