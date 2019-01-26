#include <stdio.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
using namespace std;

#define DESTINATION_DEV "/media/USBdisk"

// Return 0 in case there exists no such a file or folder
// Return 1 in case it is a folder
// Return 2 in case it is a file
// Return 3 in other cases (Symbolic link, Socket, FIFO special, Character special or Block special)
int getStatusFileOrDir(const string& path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return 0;
	if ((info.st_mode & S_IFDIR) != 0)
		return 1;
	if ((info.st_mode & S_IFREG) != 0)
		return 2;
	return 3;
}

//Copy file to destination
//RETURN 0: File copied.
//RETURN -1: Source file not found.
//RETURN -2: Destination file cannot be created.
//Filename: you want to copy the file name
//Newfile: to be copied to the file name
//
int cp(const char*filename, const char*newfile)
{
	ifstream in;
	ofstream out;

	//Open the source file
	in.open(filename);
	if (in.fail()) 
	{
		//Failed to open file
		in.close();
		out.close();
		return -1;
	}

	//Open the destination file
	out.open(newfile);
	if (out.fail()) 
	{
		//Failed to open file
		in.close();
		out.close();
		return -2;
	}

	//Copy file
	out << in.rdbuf();
	out.close();
	in.close();
	return 0;
}

int main(int argc, char *argv[])
{
	// copy an Evidence file to USB

	std::string pathUSB("/dev/sda1");
	std::string targetDir = DESTINATION_DEV;

	if (getStatusFileOrDir(pathUSB) == 0)
	{
		std::cout << "USB not inserted.\n";
		return -1;
	}

	//FILE *f = popen("mount | grep /dev/sda1", "r");
	//if (f == NULL)
	//{
	//	std::cout << "USB not inserted.\n";
	//	return -1;
	//}

	//if ((fgetc(f) == EOF) && mount("/dev/sda1", DESTINATION_DEV, "vfat", MS_NOATIME, NULL))
	//{
	//	pclose(f);
	//	std::cout << "USB cannot be mounted.\n";
	//	return -2;
	//}

	if (mount("/dev/sda1", DESTINATION_DEV, "vfat", MS_NOATIME, NULL))
	{
		cout << "USB /dev/sda1 cannot be mounted on " << DESTINATION_DEV << ".\n";
		return -2;
	}

	std::cout << targetDir.append("/") << std::endl;

	//int start = targetDir.find(" on ", 8) + 4;
	//int len = targetDir.find(" type ", start) - start;
	//targetDir = targetDir.substr(start, len).append("/");
	//std::cout << start << " " << len << " " << targetDir << std::endl;
	
	if (argc <= 1)
	{
		std::cout << "Usage " << argv[0] << " file1 [file2 ... [filen]]" << std::endl;
		return -3;
	}

	int rst = 0;
	for (int i = 1; i < argc; i++)
	{
		std::string source(argv[i]);
		int start = source.find_last_of('/');
		if (start > 0)
			source = source.substr();
		rst = cp(argv[i], targetDir.append(source).c_str());
		switch ( rst )
		{
			case -1:
				std::cout << "Cannot find evidence file " << argv[i] << std::endl;
				goto out_return;
				break;
			case -2:
				std::cout << "Cannot copy evidence file " << argv[i] << " to " << targetDir << std::endl;
				goto out_return;
				break;
		}
	}

out_return:
	// do the unmount/eject
	umount2(DESTINATION_DEV, MNT_FORCE);
	
	return (rst);
}
