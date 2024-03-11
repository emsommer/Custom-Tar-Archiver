#include <fcntl.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
    int magic;
    int eop;
    int block_count;
    int file_size[4];
    char deleted[4];
    int file_name[4];
    int next;
} hdr;

int main(int argc, char** argv)
{
    //for writting error messages
    char* msg;

    if(argc < 2)
    {
        msg = "No archive specified\n";
        write(1, msg, strlen(msg));
        return -1;
    }

    //for holding archive name argument
    char* name;

    //for saving file descriptors of opening files, used for writting and reading files
    int fd;

    //archive name
    name = argv[1];

    //open archive
    fd = open(name, O_RDONLY, 0644);

    //if archive invalid print error and exit
    if(fd == -1)
    {
        msg = "archive not found\n";
        write(1, msg, strlen(msg));
    }
    
    //read first header and store it in struct header
    hdr header;

    //read in first header
    lseek(fd, 0, SEEK_SET);
    read(fd, &header, sizeof(header));

    //check to make sure given archive is an archive by comparing magic number
    if(header.magic != 0x63746172)
    {
        msg = "given archive is not of ctar format\n";
        write(1, msg, strlen(msg));
        return -1;
    }

    //length of the file name
    short len = 0;

    lseek(fd, 0, SEEK_SET);

    //while more headers remaining
    while(1)
    { 
        read(fd, &header, sizeof(header));

        //for each file in a header
        for(int i = 0 ; i < header.block_count ; i++)
        {
            //check for deleted file
            if(header.deleted[i] == 1)
            {
                continue;
            }
            
            //lseek to header.file_name[i]
            lseek(fd, header.file_name[i], SEEK_SET);

            //read 2 bytes for length of file name store in variable called len
            read(fd, &len, sizeof(short));

            //name of current file to create
            char currName[len];
            //read next len bytes and store in currName
            read(fd, currName, len);
            currName[len] = 0;

            //read file contents
            char fileContent[header.file_size[i]];
            read(fd, fileContent, header.file_size[i]);

            //open a file with name currName
            int opn = open(currName, O_WRONLY | O_CREAT, 0644);

            if(opn == -1)
            {
                msg = "cannot create the file\n";
                write(1, msg, strlen(msg));
                return -1;
            }

            //write to open file
            write(opn, fileContent, header.file_size[i]);
            close(opn);
        }
        //if no more headers  and  we wrote the last file for this header
        if((header.next == 0))
        {   
            return 0;
        } 
        
        lseek(fd, header.next, SEEK_SET);
    }
    return 0;
}