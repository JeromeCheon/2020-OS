/* This is jerry.c file which roles to intermediate between mousehole and user to communicate in some way. 
 *
 * jerry has to get a specific user name and fname from user (using pid functions maybe) then report it to mousehole to prevent.
 * Also, through this jerry.c, we have to contact with /proc/mousehole in user level. (Maybe through using echo ? )
 *
 * Key Features :
 * 1. Block Files Opening of User
 *	- (Hint) Get a user ID using linux/cred.h and translate the string uname to integer(sscanf)
 * 2. Prevent Killing of Processes
 *	- a user will pass a username to jerry(get fname using pointer?)
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <windows.h>
#include <pwd.h>

int main(int argc, char* argv[]){ // input으로 uname과 fname을 받을 것.
    //argv[0]= filepath, argv[1] = uname, argv[2] = fname;
    int fd1 =0, fd2 =0, wr =0, rd= 0, result=0;
    struct passwd * u_info;
    struct passwd pwd ;
    char key1, key2 ;
//    char buf[256] ;
    if (argc !=3){
        printf("Wrong input! Usage : %s <uname> <fname>\n ", argv[0]) ;
        exit(0);
    }
    if((u_info = getpwnam(argv[1]))==NULL) {
        printf("Invalid user name! Please input the user who exists in the OS \n") ;
        exit(0) ;
    }
    memcpy(&pwd, u_info, sizeof(struct passwd));
       
    printf("Hi. I'm Jerry. Nice to meet you. \n") ; sleep(1000);
    
    printf("My role is to intermediate between you and a mousehole. I'll deal with something right now. \n") ; sleep(1000);
    
    printf("You pressed some file name %s, right? For now this file won't be able to be opened by %s \n", argv[2], argv[1]) ; sleep(1000);
    
    printf("If you want to keep going this procedure, please press 'Y'. Unless, Press any keys. \n") ;
    scanf("%c", &key1) ;
    if (key1 != 'Y') {
        printf(" The jerry program will be terminated. Thanks. \n");
        exit(0);
    }
    // open 을 두개 써서 하나는 uname을 보내고 다른 하나는 fname을 보내게 해야겠구나.
    printf("Okay, let's start blocking the opening file behaviors from a specific user \n");
    fd1 = open("/proc/mousehole", O_RDWR) ; // 이 때 uid랑 gid는 프로세스 생성자의 것을 따라감.
    
    if (fd==-1) {
        perror("Failed connecting /proc/mousehole LKM") ;
        exit(1);
    }
    wr = write(fd, argv[2], strlen(argv[2])) ;
    result = chmod(argv[2], 0077); // 파일 소유자의 권한 제거.
    printf("From now on, a specific file won't be opened by a user who has specific uname \n") ; sleep(1000);
    printf("Do you want to prevent killing of processes obsessed by the specific user as well? Please press 'Y' .\n ") ;
    scanf("%c", &key2) ;
    if (key2 != 'Y') {
        printf(" Okay! Now the user will use his/her file again. The jerry program will be terminated. Thanks. \n");
        close(fd);
        exit(0);
    }
    
    sleep(1000);
    
    
}
