*This project has been created as part of the 42 curriculum by nfenollo, iargence.*

# Description

This project is about writing our own HTTP server. HTTP is one of the most widely used protocols on the internet. The goal of this project is to understand the basics about HTTP servers.

# Instructions

Once the project is clone, you can use the **make** command to comile the project.

Before lunching the server you need to create a configuration file. The way our configuration file work was inspired by the nginx conf file system.

## Configuration file

### Server Block

You have two types of "blocks" you can use in the configuration file. the first one is the "server" block.

```
server {
	...
}
```

In this block we're going to define the main server rules and attributes.

 * ### listen
 the listen rule defines the port of the server.
 ` listen 8080;`

* ### server_name
the server name is used to define the domain names or IP addresses that this server block will respond to. You can provide multiple names separated by spaces.
`server_name localhost example.com;`

* ### root

root is the path to the folder where you want your files to be stored. `root path/to/folder;`

* ### index

the index rule defines the default file(s) that will be served when a client requests a directory. If multiple files are provided, the server will try them in order.
`index index.html index.htm default.html;`

* ### error_page

allows you to define custom error pages for specific HTTP error codes. You provide the error code followed by the path to the HTML file.
`error_page 404 /custom_404.html;`

* ### client_max_body_size

restricts the maximum size of the client's request body (you can use **k/K** for kilobytes and **m/M** for megabytes).
`client_max_body_size 100000000;`

___

### Location Block

The second type of block is the "location" block. It must be defined **inside** a `server` block. It allows you to set specific rules for specific routes or file extensions.

```
location /directory/ {
    ...
}
```

In this block, you can override server rules or add new ones specific to the requested URI.

* ### allow_methods

restricts which HTTP methods are accepted for this specific route.
`allow_methods GET POST DELETE;`

* ### root

just like the server block, it overrides the root directory for this specific location.
`root /path/to/specific/folder;`

* ### index

overrides the default index file(s) for this location.
`index youpi.bad_extension;`

* ### autoindex

turns on or off the directory listing. If set to `on` and no `index` file is found, the server will display an HTML page listing the contents of the directory.
`autoindex on;`

* ### allow_upload

enables or disables the ability for a client to upload files to this location.
`allow_upload on;`

* ### upload_store

defines the directory where uploaded files will be saved. This is mandatory if `allow_upload` is set to `on`.
`upload_store ./uploads/;`

* ### client_max_body_size

overrides the server's `client_max_body_size` for this specific route.
`client_max_body_size 50000;`

* ### cgi_extension

defines the file extension that will trigger the execution of a CGI script instead of serving the file normally.
`cgi_extension .bla;`

* ### cgi_path

provides the absolute or relative path to the CGI executable that will process the files matching the `cgi_extension`.
`cgi_path ./cgi-bin/cgi_tester;`

* ### return

allows you to set up an HTTP redirection. You must provide the HTTP status code (like 301 or 302) followed by the redirection URL.

___

## Execution

Now that the configuration file is ready you can start the server by execution the binary file : `./webserv [conf file path]`
When the server is running you can contact it by using your browser, tools like postman or bash commands like curl.

# Ressources

## links 
 - https://www.youtube.com/watch?v=YwHErWJIh6Y&t=1388s
 - https://www.youtube.com/watch?v=pTIZ9YjE3Pw
 - https://www.youtube.com/watch?v=bEsRapsPAWI
 - https://www.youtube.com/watch?v=N49UyTlUXp4

 - https://medium.com/@sakhawy/creating-an-http-server-from-scratch-ed41ef83314b
 - https://dev.to/dmytro_huz/building-your-own-web-server-part-1-theory-and-foundations-3kgo
 - https://www.geeksforgeeks.org/c/socket-programming-cc/

 - https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Messages

## AI usage

In this project AI was used to :
 - list explain every rule that we needed in the configuration file.
 - explain what a HTTP server would do in certain cituations.
 - unblock when we were stuck un bugs.