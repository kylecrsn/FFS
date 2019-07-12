#include "global.h"
#include "mkfs.h"
#include "tests.h"
#include "alloc.h"
#include "dir_alloc.h"

int main(int argc, char *argv[])
{
	uint32_t i;

	//Setup rand()
	srand(time(NULL));

	//Mount and build the file system
	mount();
	mkfs();

	//Setup test/output directories
	make_test_data();

	//Perform any debugging/testing
	clear_file("./outputs/inode_data_1.txt");
	for(i = 0; i < 20; i++)
	{
		Inode node = read_inode(allocate_inode(i));
		print_inode_data("./outputs/inode_data_1.txt", "a", node);
	}
	
	//Clean up dynamic variables
	clean();
	
	return 0;
}
