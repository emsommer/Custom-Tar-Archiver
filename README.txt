As far as I am aware. Utar works perfectly. 

Ctar on the other hand, works perfectly for a single call of the program. If you try to call the program multiple times to append files onto the end of the existing archive, it will not work, and will ruin the existing archive. However, the delete function and add function works as specified and the outputted archive will utar. You just cant call ctar with the -a flag multiple times on the same archive. You can call -d multiple times on the same archive though and it should work.


Test cases:

I used the files in the TESTS directory to test my program. I also used Gorkems supplied test ctar archive.

I called ctar -a using no file arguments, more than 4 file arguments, and 4 file arguments. I also called it with a file name that already exists in the archive. I also called it will no arguments.

I called ctar -d with files that dont exist in the archive, and files that do. I also called it with no arguments.

I called utar with all of the files in the TESTS directory (once they were archived) and I also tested Gorkems given test ctar archive. I also called utar with a non-ctar archive, and no arguments at all. 