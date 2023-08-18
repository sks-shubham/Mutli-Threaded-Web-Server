#include "http_server.hh"

#include <vector>
#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <dirent.h>

vector<string> split(const string &s, char delim)
{
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    if (!item.empty())
      elems.push_back(item);
  }
  return elems;
}

int found = false;

HTTP_Request::HTTP_Request(string request)
{
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  //  TODO : extract the request method and URL from first_line here
  this->method = first_line[0];
  this->url = first_line[1];
  // TODO : Complete

  if (this->method != "GET")
  {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req)
{
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "1.0";

  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    string body;

    if (S_ISDIR(sb.st_mode))
    {
      // In this case, requested path is a directory.
      // TODO : find the index.html file in that directory (modify the url
      // accordingly)
      DIR *dir = NULL;
      struct dirent *url_of_dir = NULL;
      // cout << url << endl;
      if ((dir = opendir(url.c_str())) != NULL)
      {
        // cout << "<<" << dir << endl;
        while ((url_of_dir = readdir(dir)) != NULL)
        {
          found = false;
          string split_dir = url_of_dir->d_name;
          // cout << "$$" << split_dir << endl;
          // if(strcmp(split_dir,"..")==0)
          //   break;
          if (split_dir == "index.html")
          {
            url = url + "/index.html";
            found = true;
            // cout << url;
            break;
          }
        }
        closedir(dir);
        if (found == false)
        {
          // cout << "inside false" << endl;
          response->status_code = "404";
          response->status_text = "Page not found";
          response->content_type = "text/html";
          url = "html_files/error404.html";
          std::ifstream urlfs(url);
          response->body.assign((std::istreambuf_iterator<char>(urlfs)), (std::istreambuf_iterator<char>()));
          urlfs.seekg(0, std::ios::end);
          size_t sizeofurl = urlfs.tellg();
          response->content_length = to_string(sizeofurl); // return 0;
        }
      }
      else
      {
        /* could not open directory */
        perror("");
        // return 0;
      }
    }

    /*
    TODO : open the file and read its contents
    */

    std::ifstream urlfs(url);
    response->body.assign((std::istreambuf_iterator<char>(urlfs)), (std::istreambuf_iterator<char>()));
    urlfs.seekg(0, std::ios::end);
    size_t sizeofurl = urlfs.tellg();

    /*
    TODO : set the remaining fields of response appropriately
    */
    response->content_type = "text/html";
    response->content_length = to_string(sizeofurl);
  }

  else
  {
    response->status_code = "404";
    /*
    TODO : set the remaining fields of response appropriately
    */
    response->status_text = "Page not found";
    response->content_type = "text/html";
    response->status_code = "404";
    url = "html_files/error404.html";
    std::ifstream urlfs(url);
    response->body.assign((std::istreambuf_iterator<char>(urlfs)), (std::istreambuf_iterator<char>()));
    // response->body = "<html>\n<head>\n<title>404 Not Found</title>\n</head>\n<body>\n<h1> Error 404 : Page Not Found</ h1>\n</body>\n</html>\n";
    urlfs.seekg(0, std::ios::end);
    size_t sizeofurl = urlfs.tellg();
    response->content_length = to_string(sizeofurl);
    // response->content_length=to_string(response->body.length());
  }

  delete request;

  return response;
}

string HTTP_Response::get_string()
{
  /*
  TODO : implement this function
  */
  return "HTTP/" + this->HTTP_version + " " + this->status_code + " " + this->status_text + "\n" + "Content-Type: " + this->content_type + "\nContent-Length: " + this->content_length + "\n\n" + this->body;
}
