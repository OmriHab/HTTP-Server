# HTTP Server
Local HTTP server runing on given port
# Getting started
This server has been tested on Ubuntu Linux and should work on other Linux repos and Mac OS.
## Installing
First clone the package by entering this in your terminal:
```
git clone https://github.com/OmriHab/HTTP-Server.git
```
Then cd into the folder `cd httpServer`, and make it by using `make`  
Run the HTTP server by using `./httpServer <port> <base_path>`  
base_path supports relative paths.
# Server functionality
The server serves files from the base path given in the arguments and doesn't give files in a lower directory than that.  
In the case of an http error, the server sends an appropiate error html page with a message containing the error code number and error message.  
### CHC
The server also supports CHC files.  
A CHC file is an html file which might have CHC code inside it and ends with a .chc suffix.
### CHC rules
A CHC section is a section between `<?chc` and `?>`.  
The CHC code is run server-side and the output of the program is witten in place of the CHC section.  
CHC code is written in c++, with all includes on top, then the code.  
CHC code has `iostream` included built-in and `using namespace std` for easier use, as CHC is meant for small scripts and not big programs.  
On fail to run file, or on an exception that cuts the program before finishing, nothing is printed in the CHC section.
# TODO
Add config file for server options e.g port, base path, supported file types and more.  
Add GET variables to CHC pages.
