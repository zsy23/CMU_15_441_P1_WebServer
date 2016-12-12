# CMU_15_441 Computer Network
Project 1 Web Server

My Solutions to CMU_15_441 Computer Network Project 1 Web Server and developed under macOS 10.12.1.

Usage:
    make
    ./http_server <http_port> <https_port> <log_file> <lock_file> <www_folder> <cgi_script> <private_key_file> <certificate_file>

Description:
    The URI starting with '/cgi/' will provide a simple blog service called flaskr(http://flask.pocoo.org/docs/0.11/tutorial/) through CGI.
    Otherwise the server will provide a static web page provided by the course website.
