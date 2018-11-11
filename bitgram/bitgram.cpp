#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

// breaks one 32bit int into 4 8bit chars 'abcd' using modular arithmetic and shifting bits
breakInt(unsigned long int n, char &a, char &b, char &c, char &d){
d = n % 256;
c = n % 65536 >> 8;
b = n % 16777216 >> 16;
a = n % 4294967296 >> 24;
}

// helper function to generate 52-bit bitmap header (filesize is unsigned cause we don't want any accidental negative numbers)
void createHeader(std::vector<char> &header, unsigned long int fileSize){

unsigned int bytesInRow = 12; // we'll have 12 bytes per row, for starters
unsigned int bytesInColumn = fileSize/12; // the rest will be distributed across the columns

// could (should) be done in a better way
while(bytesInColumn % 2 == 0){
    bytesInColumn/=2;
    bytesInRow*=2;
    if(bytesInRow > bytesInColumn) break;
}

header.push_back(0x42); // always 'B'
header.push_back(0x4d); // always 'M'
char a{},b{},c{},d{};
breakInt(fileSize+54, a, b, c, d); // filsize is written as dcba (from least significant BYTE to most significant BYTE) - NOT BIT !!!
header.push_back(d);
header.push_back(c);
header.push_back(b);
header.push_back(a);

for(int i = 0; i < 4; ++i) header.push_back(0x00);
header.push_back(0x36); // header offset to data - in our case, it's always 54 (0x36) bytes
for(int i = 0; i < 3; ++i) header.push_back(0x00);
header.push_back(0x28); // size of infoHeader - in our case, it's always 40 (0x28) bytes
for(int i = 0; i < 3; ++i) header.push_back(0x00);

// width of image (aka number of columns)
breakInt(bytesInRow/3, a, b, c, d);
header.push_back(d); // also written from least significant BYTE to most significant BYTE
header.push_back(c);
header.push_back(b);
header.push_back(a);

// height of image (aka number of rows)
breakInt(bytesInColumn/3, a, b, c, d);
header.push_back(d); // same here
header.push_back(c);
header.push_back(b);
header.push_back(a);

// number of planes - always 1 (also written in reverse order)
header.push_back(0x01);
header.push_back(0x00);

// bytes per pixel - always 24 (0x18) because it's a 24-bit bitmap (written in reverse order)
header.push_back(0x18);
header.push_back(0x00);

// remaining 24 bytes of header can be set to 0 (these are bytes used for information about compression - but we don't do that here)
for(int i = 0; i<24; ++i) header.push_back(0x00);
return;
}

// main function that makes the bitgram
void makeBitgram(char *src, char *dest){

std::vector<char> byteCode; // we'll store the actual program (pixels) here
std::vector<char> header; // the header of the bitmap
std::ifstream s(src, std::ifstream::in); // open file for reading

char c{};
while(s.get(c)) byteCode.push_back(c); // get all the characters of the program, and put them in the pixel area of the bitmap
s.close(); // we're done with the input file

while(byteCode.size() % 12 != 0) byteCode.push_back(' '); // make the total number of bytes divisible by 12 (this way we don't have to do any row padding)

createHeader(header, byteCode.size()); // create the header

std::ofstream d(dest, std::ios::binary); // open output file

// write the header
for(int i = 0; i < header.size(); ++i)
    d.write(reinterpret_cast<char*>(&header[i]), sizeof(char));

// now write the pixels (try to edit this part so it writes them down encoded for extra swag points)
for(int i = 0; i<byteCode.size();++i)
    d.write(reinterpret_cast<char*>(&byteCode[i]), sizeof(char));

d.close(); // close output file

return; // we're done here
}


int main(int argc,char *argv[]){

if(argc == 2 && *argv[1] == 'h'){ // if the help section is called
    std::cout<<"\n===========================================================================\n";
    std::cout<<"Welcome to bitgram v1.0\n";
    std::cout<<"This is a very simple program that turns your files into bitmaps.\n";
    std::cout<<"Call with: bitgram [source] [destination]\n";
    std::cout<<"Made by Edvin Teskeredzic 2018 - github.com/eteskeredzic\n";
    std::cout<<"\n===========================================================================\n";
    std::cout<<"HELP";

}
else if(argc != 3){ // if there is an error, report it to the error stream
    std::cerr<<"\n===========================================================================\n";
    std::cerr<<"Invalid syntax!\nUsage: bitgram s[source] [destination]\nUse bitgram -h for more info\n";
    std::cerr<<"===========================================================================\n";
}
else{ // if all is good
    try{
    makeBitgram(argv[1], argv[2]); // main function
    }
    catch(std::domain_error e){ // if anything unwanted or spooky happens, let the user know
    std::cerr<<"\n========================================================\n";
    std::cerr<<"ERROR: "<<e.what()<<"\n";
    std::cerr<<"========================================================\n";
    }
    catch(...){ // if something REALLY spooky happens
        std::cout<<"ERROR: I have no idea what just happened.";
    }
}

return 0;
}
