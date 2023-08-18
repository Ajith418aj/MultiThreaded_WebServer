#include "http_server.hh"

#include <vector>

#include <sys/stat.h>

#include <fstream>
#include <sstream>

node_t* head = NULL;
node_t* tail = NULL;

vector<string> split(const string &s, char delim) {
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request) {

  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');
  
  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */
  this->method = first_line[0];
  this->url = first_line[1];
  if (this->method != "GET") {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req) {

  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;
  response->HTTP_version = "HTTP/1.0";
  
  struct stat sb;
  
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body;
    if (S_ISDIR(sb.st_mode)) {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      struct stat ab;
      string filename;
      if(url.back() != '/'){
      	filename = url + string("/index.html");
      } else {
      	filename = url + string("index.html");
      }

      if (stat(filename.c_str(), &ab) == 0) // requested file exists
      { 
      	url = filename;
      } 
 
    }

    /*
    TODO : open the file and read its contents
    */
    	string word;
    	string data;
	fstream file;
	file.open(url.c_str());
	while (!file.eof())
        {       
           getline(file, word);
           data += word;
           
        }
        response->body = data;
        response->body += "\n";
	
    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  else {
    response->status_code = "404";
	string error = R"(<!DOCTYPE html>
	<html>
	<head>
  	 <title>404 Not Found</title>
	</head>
	<body>
  	 <h1>Not Found</h1>
  	 <p>The requested URL was not found on this server.</p>
	</body>
	</html>)";
	response->body = error;
    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  delete request;
  return response;
}

string HTTP_Response::get_string() {
  /*
  TODO : implement this function
  */
  string bodyy = this->body;
  int size = bodyy.length();
  string response = this->HTTP_version +" "+ this->status_code+ " "+this->status_text + "\n" 
                     + "Content-Type: "+ this->content_type+"\n"+"Content-Length:"+ to_string(size) +"\n\n"+this->body;
                    
 return response;
}

void enqueue(int *client_socket) {
	node_t *newnode = (node_t*)malloc(sizeof(node_t));
	newnode->client_socket = client_socket;
	newnode->next = NULL;
	if(tail == NULL) {
		head = newnode;
	} else {
		tail->next = newnode;
	}
	tail = newnode;
	//free(newnode);
}

int* dequeue() {

	if(head == NULL) {
		return NULL;
	} else {
		int* result = head->client_socket;
		node_t *temp = head;
		head = head->next;
		if(head == NULL) {
			tail = NULL;
		}
		delete temp;
		return result;
	}
}
