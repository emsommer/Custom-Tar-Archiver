#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#include <stdio.h>

struct stat file_stat;

typedef struct
{
    int magic; /* This must have the value 0x63746172. */
    int eop; /* End of file pointer. */
    int block_count; /* Number of entries in the block which are in-use. */
    int file_size[4]; /* File size in bytes for files 1..4 */
    char deleted[4]; /* Contains binary one at position i if i-th entry was deleted. */
    int file_name[4]; /* pointer to the name of the file. */
    int next; /* pointer to the next header block. */
} hdr;

void init_header(hdr *header);
int check(char* name, char* file[], int file_number);

int main (int argc, char** argv)
{
    //for writting error messages
    char* msg;

    //for maintaining current operation
    char operation;

    //not enough arguments case 
    if(argc < 3)
    {
        msg = "Not enough arguments!\n";
        write(1, msg, strlen(msg));
    }

    if(strlen(argv[1]) == 2)
    {
        if(argv[1][0] == '-')
        {
            if(argv[1][1] == 'a' || argv[1][1] == 'd')
            {
                operation = argv[1][1];
            }
            else 
            {
                //improper arugment
                msg = "Argument invalid, use '-a' for add and '-d' for delete\n";
                write(1, msg, strlen(msg));
                return -1;
            }
        }
        else
        {
            //improper argument
            msg = "Argument invalid, use '-a' for add and '-d' for delete\n";
            write(1, msg, strlen(msg));
            return -1;
        }
    }
    else
    {
        //improper argument
        msg = "Argument invalid, use '-a' for add and '-d' for delete\n";
        write(1, msg, strlen(msg));
        return -1;    
    }

    //for holding archive name argument
    char* name;

    //for holding file arguments 
    char* file[100];

    //for saving file descriptors of opening files, used for writting and reading files
    int fd;

    //DEFINE HEADER 
    hdr header;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ADD FILES COMMAND ////////////////////////////////////////////////////

    //IF OPERATION == '-a', add file to archive
    if(operation == 'a')
    {
        //archive name 
        name = argv[2];

        //counter for number of files passed
        int file_number = 0;

        //extract file names from passed arguments
        if(argc > 3)
        {
            for(int i = 0 ; i < argc-3 ; i++)
            {
                //variable that contains given file names
                file[i] = argv[i+3];

                //if file name exists, increment number of files in arguments
                if(file[i] != 0)
                {
                    file_number++;
                }
            }
        }      
        
        //check if files already exist in archive
        int valid = check(name, file, file_number);

        //if file aready exists in archive
        if(valid == -1)
        {
            msg = "One of the passed file arguments already exists in the given archive\n";
            write(1, msg, strlen(msg));
        }

        //open the archive or create it if needed
        fd = open(name, O_WRONLY | O_CREAT, 0644);

        if(fd == -1)
        {
            msg = "Unable to open/create the archive\n";
            write(1, msg, strlen(msg));
            return -1;
        }

        int chunks = ((float) file_number / 4) + 1;

        int remaining_files = file_number;

        int currFile = 0;

        //for each header
    

        for(int j = 0 ; j < chunks ; j++)
        {
            
            //make a header 
            init_header(&header);
            //write the header to the end of the archive
            int off = lseek(fd, 0, SEEK_END);
            write(fd, &header, sizeof(header));

            //determine number of files to go in the header
            int chunk_size = (remaining_files >= 4) ? 4 : remaining_files;
            remaining_files -= chunk_size;


            //for each file to go in a chunk
            for(int i = 0 ; i < chunk_size; i++)
            {
                int opn = open(file[currFile], O_RDONLY, 0644);
                if(opn == -1)
                {
                    msg = "unable to open file, archive still created but is empty\n";
                    write(1, msg, strlen(msg));
                    return -1;
                }

                //lseek to end of file
                int file_name_offset = lseek(fd, 0, SEEK_END);

                //write 2 bytes for short 
                short name_length = strlen(file[currFile]);
                write(fd, &name_length, sizeof(short));

                //write file name
                write(fd, file[currFile], name_length);

                //lseek to end of file
                lseek(fd, 0, SEEK_END);

                //just because windows is stupid and i cant use lstat
                //file_stat.st_size = 5;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //write file contents (works on linux not windows)
                lstat(file[currFile], &file_stat); //to get size of file
                char content[file_stat.st_size];

                //read file
                read(opn, content, file_stat.st_size);
                //write file
                write(fd, content, file_stat.st_size);

                //update header
                header.block_count++;
                header.file_name[i] = file_name_offset;
                header.file_size[i] = file_stat.st_size;
                header.eop = lseek(fd, 0, SEEK_END);

                //update header.next if current header is full 
                if(header.block_count == 4)
                {
                    header.next = lseek(fd, 0, SEEK_END);
                    lseek(fd, off, SEEK_SET);
                    write(fd, &header, sizeof(header));
                }

                //seek to the offset that we created the most recent header at
                lseek(fd, off, SEEK_SET);
                write(fd, &header, sizeof(header));

                currFile++;
            }
        }
    }


    //IF OPERATION == '-d', delete file from archive 
    if(operation == 'd')
    {
        int file_number = 0 ;

        //obtain name of archive to interact with
        name = argv[2];

        //open the archive
        fd = open(name, O_RDWR, 0644);

        if(fd == -1)
        {
            msg = "Unable to open/create the archive\n";
            write(1, msg, strlen(msg));
            return -1;
        }


        //extract file names from passed arguments
        if(argc > 3)
        {
            for(int i = 0 ; i < argc-3 ; i++)
            {
                //variable that contains given file names
                file[i] = argv[i+3];

                //if file name exists, increment number of files in arguments
                if(file[i] != 0)
                {
                    file_number++;
                }
            }
        } 


        //length of the file name
        short len = 0;

        //offset of the current header
        int off = lseek(fd, 0, SEEK_SET);

        //for each passed file 
        for(int j = 0 ; j < file_number ; j++)
        {
            //while more headers remaining
            while(1)
            { 
                //read in header
                read(fd, &header, sizeof(header));

                //for each file in a header
                for(int i = 0 ; i < header.block_count ; i++)
                {   
                    //lseek to header.file_name[i]
                    lseek(fd, header.file_name[i], SEEK_SET);

                    //read 2 bytes for length of file name store in variable called len
                    read(fd, &len, sizeof(short));
        
                    //name of current file to compare
                    char currName[len];
                    //read next len bytes and store in currName
                    read(fd, currName, len);
                    currName[len] = 0;

                    //if file to be deleted is found 
                    if(strcmp(file[j], currName) == 0)
                    {
                        //mark as deleted
                        header.deleted[i] = 1;

                        //write header back
                        lseek(fd, off, SEEK_SET);
                        write(fd, &header, sizeof(header));
                    }
                }

                //if no more headers
                if((header.next == 0))
                {   
                    close(fd);   
                    return 0;
                } 
                
                off = lseek(fd, header.next, SEEK_SET);
            }
        }
    }

    close(fd);
    return 0;
}

//helper method 
void init_header(hdr *header)
{
    header->magic = 0x63746172;
    header->eop = 0;
    header->block_count = 0;
    for(int i = 0 ; i < 4 ; i++)
    {
        header->file_size[i] = 0;
        header->deleted[i] = 0;
        header->file_name[i] = 0;
    }
    header->next = 0;
}

//check if archive is an archive and if files already exist in archive
int check(char* name, char* file[], int file_number)
{
    //attempt to open an existing archive
    int test = open(name, O_RDONLY, 0644);

    //if unable to open
    if(test == -1)
    {
        //must create archive
        return 0;
    }

    //if able to open
    hdr header;
    read(test, &header, sizeof(header));

    if(header.magic != 0x63746172)
    {
        return -1;
    }

    //check for existing files 
            
    //length of the file name
    short len = 0;

    //offset of the current header
    lseek(test, 0, SEEK_SET);

    //for each passed file 
    for(int j = 0 ; j < file_number ; j++)
    {
        //while more headers remaining
        while(1)
        { 
            //read in header
            read(test, &header, sizeof(header));

            //for each file in a header
            for(int i = 0 ; i < header.block_count ; i++)
            {   
                //lseek to header.file_name[i]
                lseek(test, header.file_name[i], SEEK_SET);

                //read 2 bytes for length of file name store in variable called len
                read(test, &len, sizeof(short));
    
                //name of current file to compare
                char currName[len];
                //read next len bytes and store in currName
                read(test, currName, len);
                currName[len] = 0;

                //if file already exists
                if(strcmp(file[j], currName) == 0)
                {
                    return -1;
                }
            }

            //if no more headers
            if((header.next == 0))
            {   
                close(test);    
                return 0;
            } 
            
            lseek(test, header.next, SEEK_SET);
        }
    }

    close(test);

    return 0;
}