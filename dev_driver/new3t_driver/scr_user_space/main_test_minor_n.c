 #include <unistd.h>
 #include <string.h>
 #include <stdio.h>
 #include <fcntl.h>

  int main() {
          int fd0,fd1,fd2,fd3, result, len;
          char buf[30];
          char *str;
          memset(buf,'a',sizeof(char)*30);
          str = "xyz"; len = strlen(str);

          if ((fd0 = open ("/dev/new_t3_main0", O_RDWR)) == -1) {
                  perror("open failed");
                  return -1;
          }
          printf("HO APERTO IL FILE /dev/new_t3_main0 \n");
          if ((fd1 = open ("/dev/new_t3_main1", O_RDWR)) == -1) {
                            perror("open failed");
                            return -1;
                    }
          printf("HO APERTO IL FILE /dev/new_t3_main1 \n");
          if ((fd2 = open ("/dev/new_t3_main2", O_RDWR)) == -1) {
                            perror("open failed");
                            return -1;
                    }
          printf("HO APERTO IL FILE /dev/new_t3_main2 \n");
          if ((fd3 = open ("/dev/new_t3_main3", O_RDWR)) == -1) {
                            perror("open failed");
                            return -1;
                    }
          printf("HO APERTO IL FILE /dev/new_t3_main3 \n");


          if ((result = read (fd0, &buf, sizeof(buf))) != len) {
              perror("read fd0 failed");
          }
          if ((result = read (fd1, &buf, sizeof(buf))) != len) {
              perror("read fd1 failed");
          }
          if ((result = read (fd2,&buf, sizeof(buf))) != len) {
              perror("read fd2 failed");
          }
          if ((result = read (fd3,&buf, sizeof(buf))) != len) {
        	  perror("read fd3 failed");
          }
          str = "SONO IL FILE =0"; len = strlen(str);
          if ((result = write (fd0, str, len)) != len) {
        	  perror("write failed");
           }
          str = "SONO IL FILE =1" ; len = strlen(str);
          if ((result = write (fd1, str, len)) != len) {
        	  perror("write failed");
          }
          str = "SONO IL FILE =2"; len = strlen(str);
          if ((result = write (fd2, str, len)) != len) {
           	  perror("write failed");
          }
          str = "SONO IL FILE =3"; len = strlen(str);
          if ((result = write (fd3, str, len)) != len) {
          	  perror("write failed");
          }
          memset(buf,'a',sizeof(char)*30);
          if ((result = read (fd0, &buf, sizeof(buf))) != len) {
              perror("read failed");
          }
          buf[result] = '\0';
          fprintf (stdout, "passed read fd0 back ::: \"%s\"\n", buf);
          memset(buf,'a',sizeof(char)*30);
          if ((result = read (fd1, &buf, sizeof(buf))) != len) {
			   perror("read failed");
		   }
		   buf[result] = '\0';
		   fprintf (stdout, "passed read fd1 back ::: \"%s\"\n", buf);

		   memset(buf,'a',sizeof(char)*30);
		   if ((result = read (fd2, &buf, sizeof(buf))) != len) {
				perror("read failed");
			}
			buf[result] = '\0';
			fprintf (stdout, "passed read fd2 back ::: \"%s\"\n", buf);

			memset(buf,'a',sizeof(char)*30);
			if ((result = read (fd3, &buf, sizeof(buf))) != len) {
				perror("read failed");
			}
			buf[result] = '\0';
			fprintf (stdout, "passed read fd3 back ::: \"%s\"\n", buf);

/*	  int i;
	  for( i=0; i<10;i++){
	  printf("sono arrivato qua!!!!!\n");
          if ((result = read (fd, &buf, sizeof(buf))) != len) {
                  perror("read failed");
                //  return -1;
          }
 	  printf("sono arrivato qua 222!!!!!\n");
          }
          buf[result] = '\0';
	printf("ma passo di qua???!!!!!\n");
          if (strncmp (buf, str, len)) {
                  fprintf (stdout, "failed: read back \"%s\"\n", buf);
          } else {
                  fprintf (stdout, "passed\n");
          }
*/
/*	fd_set rfds;
	fd_set wfds;
	struct timeval tv;
	int retval;
	/* Watch stdin (fd 0) to see when it has input. */
/*	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_SET(fd, &wfds);
 	/* Wait up to five seconds. */
/*	tv.tv_sec = 5;
	tv.tv_usec = 0;
	retval = select(1, &rfds,&wfds, NULL, &tv);
 	/* Don't rely on the value of tv now! */
 /*	if (retval == -1)
		perror("select()");
 	else if (retval)
		printf("Data is available now.\n");
 		//FD_ISSET(fd, &rfds) //will be true. */
/*	else
	         printf("No data within five seconds.\n");

 */
          close(fd0);
          close(fd1);
          close(fd2);
          close(fd3);
          return 0;

  }

